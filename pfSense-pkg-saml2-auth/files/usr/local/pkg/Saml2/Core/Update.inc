<?php
//    Copyright 2025 Jared Hendrickson
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

namespace Saml2\Core\Update;

require_once 'Saml2/autoload.php';

use Saml2\Errors\SystemError;
use Saml2\Errors\UpdateError;

const RELEASES_FETCH_TIMEOUT = 10; // Timeout for fetching releases in seconds
const RELEASES_CACHE_FILE = '/var/cache/pfSense-pkg-saml2-auth/releases.json';
const RELEASES_URL = 'https://api.github.com/repos/pfrest/pfSense-pkg-saml2-auth/releases';
const PKG_STATIC_EXEC = '/usr/local/sbin/pkg-static';

/**
 * Obtains the current version of pfSense running on this system.
 * @return string The current pfSense version, formatted as x.x or x.x.x.
 */
function get_pfsense_version(): string {
    return str_replace(PHP_EOL, '', file_get_contents('/etc/version'));
}

/**
 * Obtains the base version of pfSense running on this system.
 * @return string The base pfSense version, formatted as x.x.
 * @throws SystemError If the version cannot be determined or is not in the expected format.
 */
function get_pfsense_base_version(): string {
    $parts = explode('.', get_pfsense_version());

    # Throw an error if we don't have at least a major and minor version
    if (count($parts) < 2) {
        throw new SystemError('Could not determine the base pfSense version.');
    }

    return $parts[0] . '.' . $parts[1];
}

/**
 * Obtains the current version of the pfSense-pkg-saml2-auth package installed on this system.
 * @return string The current package version, formatted as x.x or x.x.x.
 */
function get_pkg_version(): string {
    # Pull the raw pkg info for the SAML2 auth package into an array for each line
    $pkg_info = explode(PHP_EOL, shell_exec(PKG_STATIC_EXEC . ' info pfSense-pkg-saml2-auth'));

    # Loop through each line and check the version
    foreach ($pkg_info as $pkg_line) {
        if (str_starts_with($pkg_line, 'Version')) {
            # Locate the version and format it to a standard semantic version format (x.x.x)
            $version = str_replace(' ', '', $pkg_line);
            $version = explode(':', $version)[1];
            $version = strlen($version) === 3 ? $version . '.0' : $version;
            return str_replace('_', '.', $version);
        }
    }

    throw new SystemError('Could not determine the installed version of pfSense-pkg-saml2-auth.');
}

/**
 * Obtains the latest package release version available for download on this system.
 * @return string Returns the latest package version available for download.
 */
function get_latest_pkg_version(): string {
    return array_key_first(get_supported_pkg_releases()) ?? '';
}

/**
 * Obtains the release date of the latest package version available for download on this system.
 * @return string Returns the release date of the latest package version or empty string if not found.
 */
function get_latest_pkg_release_date(): string {
    $releases = get_supported_pkg_releases();
    $latest_release = reset($releases);
    return $latest_release['release_date'] ?? '';
}

/**
 * Checks if an update is available for the pfSense-pkg-saml2-auth package.
 * @return bool Returns true if an update is available, false otherwise.
 */
function is_update_available(): bool {
    # Check if the current version is less than the latest version
    return version_compare(get_pkg_version(), get_latest_pkg_version(), operator: '<');
}

/**
 * Fetches current package releases from GitHub. This method always fetches the latest releases from the GitHub API,
 * it does not attempt to read from the cache.
 * @return array Returns array of all available package releases, or empty array if the response was empty.
 * @throws SystemError If the fetch fails due to a cURL error.
 */
function fetch_pkg_releases(): array {
    # Initialize a cURL session
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, RELEASES_URL);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, RELEASES_FETCH_TIMEOUT);
    curl_setopt($ch, CURLOPT_USERAGENT, 'pfSense-pkg-saml2-auth/' . get_pkg_version());

    # Execute the request and decode the response
    $api_resp = curl_exec($ch);

    # Check for errors
    if (curl_errno($ch)) {
        throw new SystemError('Failed to fetch package releases: ' . curl_error($ch));
    }

    # Close the session
    curl_close($ch);

    return $api_resp ? json_decode($api_resp, associative: true) : [];
}

/**
 * Obtains package releases from the cache file.
 * @return array Returns array of all available package releases, or empty array if the cache is empty.
 */
function get_pkg_releases(): array {
    # Read the cache
    $cache = file_get_contents(RELEASES_CACHE_FILE);

    # Return the cached releases if the cache contains data that is valid JSON
    if ($cache) {
        $releases = json_decode($cache, associative: true);
        if (is_array($releases)) {
            return $releases;
        }
    }

    return [];
}

/**
 * Obtains the package releases that are supported by the running pfSense version.
 * @return array Returns an array of package releases supported by the current pfSense version. The array is indexed by
 * the release tag name and contains a 'name' and 'pkg_install_url' for each supported release.
 */
function get_supported_pkg_releases(): array {
    # Variables
    $pfsense_version = get_pfsense_base_version();
    $releases = get_pkg_releases();
    $supported_releases = [];

    # Loop through each version and populate our version data
    foreach ($releases as $release) {
        # Loop through the assets of each release and check if our version of pfSense is supported
        foreach ($release['assets'] as $asset) {
            if ($asset['name'] === "pfSense-$pfsense_version-pkg-saml2-auth.pkg") {
                $tag_name = $release['tag_name'];
                $supported_releases[$tag_name] = [
                    'name' => $tag_name,
                    'pkg_install_url' => $asset['browser_download_url'],
                    'release_date' => $release['published_at'],
                ];
            }
        }
    }
    return $supported_releases;
}

/**
 * @param $version string|null The version tag name to update/revert to or null to update to the latest version.
 * @throws UpdateError If the requested version does not exist or is not supported.
 * @throws UpdateError If the package removal or installation fails during the update process.
 */
function update_pkg(string|null $version = null): void {
    # Obtain all supported releases
    $supported_versions = get_supported_pkg_releases();

    # Ensure we have at least one supported version available
    if (!$supported_versions) {
        throw new UpdateError('No supported releases found!');
    }

    # Ensure the specified version exists as a supported version
    if ($version && !array_key_exists($version, $supported_versions)) {
        throw new UpdateError("Version $version either does not exist or is not supported on this system.");
    }

    # Determine the install URL for the update candidate
    $install_url = $version
        ? $supported_versions[$version]['pkg_install_url']
        : $supported_versions[array_key_first($supported_versions)]['pkg_install_url'];

    # Remove the existing installation
    exec(PKG_STATIC_EXEC . ' delete -y pfSense-pkg-saml2-auth', $output, $result_code);
    if ($result_code !== 0) {
        throw new UpdateError('Failed to remove existing package: ' . implode("\n", $output));
    }

    # Install the new package
    exec(PKG_STATIC_EXEC . " add $install_url", $output, $result_code);
    if ($result_code !== 0) {
        throw new UpdateError('Failed to install package: ' . implode("\n", $output));
    }
}
