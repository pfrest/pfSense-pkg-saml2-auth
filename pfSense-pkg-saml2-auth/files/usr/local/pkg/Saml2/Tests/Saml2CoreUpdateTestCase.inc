<?php

namespace Saml2\Tests;

require_once("Saml2/autoload.php");

use Saml2\Core\TestCase;
use function Saml2\Core\Update\fetch_pkg_releases;
use function Saml2\Core\Update\get_latest_pkg_release_date;
use function Saml2\Core\Update\get_latest_pkg_version;
use function Saml2\Core\Update\get_pfsense_base_version;
use function Saml2\Core\Update\get_pfsense_version;
use function Saml2\Core\Update\get_pkg_releases;
use function Saml2\Core\Update\get_pkg_version;
use function Saml2\Core\Update\get_supported_pkg_releases;
use const Saml2\Core\Update\PKG_STATIC_EXEC;
use const Saml2\Core\Update\RELEASES_CACHE_FILE;


/**
 * A test case to validate the Saml2 Core\Update class.
 */
class Saml2CoreUpdateTestCase extends TestCase
{
   /**
    * Ensures we can read the current pfSense version.
    */
   public function test_get_pfsense_version(): void {
        $version = get_pfsense_version();
        $this->assert_equals(
            $version,
            str_replace(PHP_EOL, "", file_get_contents("/etc/version"))
        );
   }

   /**
    * Ensures we can read the pfSense base version.
    */
    public function test_get_pfsense_base_version(): void
    {
        $version = get_pfsense_version();
        $parts = explode('.', $version);
        $expected_base_version = $parts[0] . '.' . $parts[1];
        $this->assert_equals(
            get_pfsense_base_version(),
            $expected_base_version
        );
    }

    /**
     * Ensures get_pkg_version() returns a valid semantic version string and is present in the pkg info.
     */
    public function test_get_pkg_version(): void
    {
        $version = get_pkg_version();
        $this->assert_is_true(
            preg_match('/^\d+\.\d+\.\d+([-_][a-zA-Z0-9-_]+)?$/', $version) === 1,
            "The package version '$version' is not in a valid format."
        );
    }

    /**
     * Ensures get_latest_pkg_version() returns a valid semantic version string.
     */
    public function test_get_latest_pkg_version(): void {
        $latest_version = get_latest_pkg_version();
        $this->assert_is_true(
            preg_match('/^v\d+\.\d+\.\d+([-_][a-zA-Z0-9-_]+)?$/', $latest_version) === 1,
            "The latest package version '$latest_version' is not in a valid format."
        );
    }

    /**
     * Ensures get_latest_pkg_release_date is a valid ISO 8601 date string if a latest version exists.
     */
    public function test_get_latest_pkg_release_date(): void {
        $release_date = get_latest_pkg_release_date();
        $this->assert_is_true(
            preg_match('/^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$/', $release_date) === 1,
            "The latest package release date '$release_date' is not in a valid ISO 8601 format."
        );
    }

    /**
     * Ensure fetch_pkg_releases() returns a JSON array
     */
    public function test_fetch_pkg_releases(): void
    {
        $releases = fetch_pkg_releases();
        $this->assert_is_true(
            !empty($releases),
            "The fetched package releases array is empty."
        );
    }

    /**
     * Ensures get_pkg_releases() reads all releases from the latest cache file
     */
    public function test_get_pkg_releases(): void {
        $releases = get_pkg_releases();
        $cache = file_get_contents(RELEASES_CACHE_FILE);
        $cache = json_decode($cache, associative: true);
        $this->assert_equals(
            $releases,
            $cache,
            "The releases fetched do not match the cached releases."
        );
    }

    /**
     * Ensures get_supported_pkg_releases() returns only releases supported by the current pfSense version
     */
    public function test_get_supported_pkg_releases(): void {
        $supported_releases = get_supported_pkg_releases();
        $all_releases = get_pkg_releases();
        $pfsense_version = get_pfsense_base_version();

        foreach ($supported_releases as $release) {
            # Ensure expectedd fields are present
            $this->assert_is_not_empty($release["name"]);
            $this->assert_is_not_empty($release["pkg_install_url"]);
            $this->assert_is_not_empty($release["release_date"]);

            # Ensure the pkg_install_url contains the current pfSense version
            $this->assert_is_true(
                str_contains($release["pkg_install_url"], "pfSense-$pfsense_version-pkg-saml2-auth.pkg"),
                message: "Supported release '{$release['name']}' doesn't support pfSense version '$pfsense_version'."
            );
        }
    }
}