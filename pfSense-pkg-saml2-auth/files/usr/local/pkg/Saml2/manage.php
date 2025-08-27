#!/usr/local/bin/php -f
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

require_once("Saml2/autoload.php");

use Saml2\Config;
use Saml2\Errors\UpdateError;
use function Saml2\Update\fetch_pkg_releases;
use function Saml2\Update\get_pkg_version;
use function Saml2\Update\update_pkg;
use const Saml2\Update\RELEASES_CACHE_FILE;

/**
 * Performs a backup of the SAML2 configuration
 */
function backup(): void {
    # Start the backup process
    echo "Backing up SAML2 configuration...";
    $config = new Config();
    $backup_success = $config->backup();

    # Exit on non-zero exit code if the backup fails
    if (!$backup_success) {
        echo "failed.".PHP_EOL;
        exit(1);
    }

    # Otherwise, backup was successful. Print success message.
    echo "done.".PHP_EOL;
    exit(0);
}

/**
 * Restores the SAML2 configuration from the latest backup
 */
function restore(): void {
    # Start the restore process
    echo "Restoring SAML2 configuration...";
    $config = new Config();
    $restore_status = $config->restore();

    switch($restore_status) {
        case Config::RESTORE_SUCCESS:
            echo "done.".PHP_EOL;
            exit(0);
        case Config::RESTORE_NO_BACKUP:
            echo "nothing to restore.".PHP_EOL;
            exit(0);
        case Config::RESTORE_FAILURE:
            echo "failed.".PHP_EOL;
            exit(1);
        default:
            echo "unknown error.".PHP_EOL;
            exit(1);
    }
}

/**
 * Refreshes the package releases cache
 */
function refreshcache(): void {
    # Start the cache refresh process
    echo "Refreshing package releases cache...";
    try {
        $cache = fetch_pkg_releases();
    }
    catch (Error $e) {
        echo "failed: ".$e->getMessage().PHP_EOL;
        exit(1);
    }

    # If no releases were fetched, print an error message
    if (empty($cache)) {
        echo "failed to fetch releases to cache.".PHP_EOL;
        exit(1);
    }

    # Otherwise, write the cache to the file
    file_put_contents(RELEASES_CACHE_FILE, json_encode($cache));
    echo "done.".PHP_EOL;
    exit(0);
}

/**
 * Updates the pfSense-pkg-saml2-auth package to the latest version
 */
function update(): void {
    echo "Updating package to latest version...";

    # Try to update the package, print an error message if it fails and exit with a non-zero code
    try {
        update_pkg();
    }
    catch (UpdateError $e) {
        echo "failed: ".$e->getMessage().PHP_EOL;
        exit(1);
    }

    # Otherwise, print a success message
    echo "done.".PHP_EOL;
    exit(0);
}

/**
 * Reverts the pfSense-pkg-saml2-auth package to the specified version
 */
function revert(string $version): void {
    echo "Reverting package to $version...";

    # Try to revert the package, print an error message if it fails and exit with a non-zero code
    try {
        update_pkg($version);
    }
    catch (UpdateError $e) {
        echo "failed: ".$e->getMessage().PHP_EOL;
        exit(1);
    }

    # Otherwise, print a success message
    echo "done.".PHP_EOL;
    exit(0);
}

/**
 * Prints the installed version of the pfSense-pkg-saml2-auth package
 */
function version(): void {
    echo "pfSense-pkg-saml2-auth ". get_pkg_version().PHP_EOL;
    exit(0);
}

/**
 * Displays the help page for the CLI tool
 */
function help(): void {
    echo "pfsense-saml2 - CLI tool for pfSense-pkg-saml2-auth package management".PHP_EOL;
    echo "Copyright - ".date("Y")."© - Jared Hendrickson".PHP_EOL;
    echo "SYNTAX:".PHP_EOL;
    echo "  pfsense-saml2 <command> <args>".PHP_EOL;
    echo "COMMANDS:".PHP_EOL;
    echo "  backup        : Makes a backup of the SAML2 configuration".PHP_EOL;
    echo "  restore       : Restores the SAML2 configuration from a JSON backup".PHP_EOL;
    echo "  refreshcache  : Refreshes the releases cache for package updates".PHP_EOL;
    echo "  update        : Update to the latest version of the package".PHP_EOL;
    echo "  revert        : Revert to a specific version of the package".PHP_EOL;
    echo "  version       : Displays the current version of pfSense-pkg-saml2-auth".PHP_EOL;
    echo "  help          : Displays the help page (this page)".PHP_EOL.PHP_EOL;
}

/**
 * Main function that parses command line arguments and calls the appropriate function
 * @param array $argv Command line arguments
 */
function main(array $argv): void {
    switch ($argv[1]) {
        case 'backup':
            backup();
            break;
        case 'restore':
            restore();
            break;
        case 'refreshcache':
            refreshcache();
            break;
        case 'update':
            update();
            break;
        case 'revert':
            revert(version: $argv[2] ?? '');
            break;
        case 'version':
            version();
            break;
        case 'help':
            help();
            exit(0);
        default:
            echo "Unknown command: '$argv[1]'".PHP_EOL;
            help();
            exit(1);
    }
}

main($argv);
