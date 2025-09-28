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

use Saml2\Core\Config;
use Saml2\Errors\UpdateError;
use function Saml2\Core\Update\fetch_pkg_releases;
use function Saml2\Core\Update\get_pkg_version;
use function Saml2\Core\Update\update_pkg;
use const Saml2\Core\Update\RELEASES_CACHE_FILE;

const REFRESH_CACHE_CMD = '/usr/local/pkg/Saml2/manage.php refreshcache';

/**
 * Performs a backup of the SAML2 configuration
 */
function backup(): void {
    # Start the backup process
    echo "Backing up SAML2 configuration... ";
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
    echo "Restoring SAML2 configuration... ";
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
    echo "Refreshing package cache files... ";
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
 * Sets up cron schedules for the package
 */
function setupschedule(): void {
    # Setup the cron job to run hourly
    echo "Setting up cron schedules... ";
    $cron_job = ["minute" => "@hourly", "who" => "root", "command" => REFRESH_CACHE_CMD];
    $cron_jobs = config_get_path("cron/item", []);
    $cron_jobs[] = $cron_job;
    config_set_path("cron/item", $cron_jobs);
    write_config("Created cron job for pfSense-pkg-saml2-auth package");
    configure_cron();

    echo "done.".PHP_EOL;
    exit(0);
}

/**
 * Removes cron schedules for the package
 */
function removeschedule(): void
{
    echo "Removing cron schedules... ";

    # Remove any existing cron jobs for this command
    foreach (config_get_path("cron/item", []) as $index => $cron) {
        if ($cron['command'] === REFRESH_CACHE_CMD) {
            config_del_path("cron/item/$index");
            write_config("Removed cron job for pfSense-pkg-saml2-auth package");
            break;
        }
    }

    # Apply the changes to cron
    configure_cron();
    echo "done.".PHP_EOL;
    exit(0);
}

/**
 * Updates the pfSense-pkg-saml2-auth package to the latest version
 */
function update(): void {
    echo "Updating package to latest version... ";

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
    # Ensure version is specified
    if (!$version) {
        echo "error: no version specified to revert to.".PHP_EOL;
        help();
        exit(1);
    }

    echo "Reverting package to $version... ";

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
 * Runs all (or select) TestCase classes in \RESTAPI\Tests. This is only intended to test development of this package
 * and should not be used on live installs.
 * @param $contains string|null Only run tests that contain this sub-string in the test name.
 * @note Tests will attempt to create, modify and delete configurations and files as well as restart services; which
 * can be disruptive to live systems.
 */
function run_tests(string|null $contains = ''): void {
    # Variables
    $test_cases = glob('/usr/local/pkg/Saml2/Tests/*.inc');
    $exit_code = 0;
    $test_count = count($test_cases);
    $skipped_count = 0;
    $succeed_count = 0;
    $failed_tests = [];

    # Print that we are starting tests now
    echo 'Running tests...';

    # Import each test class and run the test
    foreach ($test_cases as $test_case_file) {
        # Import classes files and create object
        require_once $test_case_file;
        $test_case = '\\Saml2\\Tests\\' . str_replace('.inc', '', basename($test_case_file));

        # Only run this test case if the test name contains the $contains string
        if (!str_contains($test_case, $contains)) {
            $skipped_count++;
            continue;
        }

        # Try to run the test case.
        $test_obj = new $test_case();
        try {
            $test_obj->run();
            $succeed_count++;
            echo '.'; // Print a dot for each test completed so the user is aware that tests are really running
            $fail_results = null;
        } catch (AssertionError $fail_results) {
            # If an AssertionError is received, there was a test failure. Print the traceback.
            echo 'F';
            $exit_code = 2;
        } catch (Exception | Error $fail_results) {
            echo 'E';
            $exit_code = 1;
        }

        if ($fail_results) {
            $exc_class = $fail_results::class;
            $fail_msg = $fail_results->getMessage();
            $result = '---------------------------------------------------------' . PHP_EOL;
            $result .= "Failure message: [$exc_class] $fail_msg" . PHP_EOL;
            $result .= "Test name: $test_obj->method" . PHP_EOL;
            $result .= "Test description: $test_obj->method_docstring" . PHP_EOL . PHP_EOL;
            $result .= $fail_results->getTraceAsString() . PHP_EOL;
            $failed_tests[] = $result;
        }
    }

    # Mark the test as a failure if there are any failed tests, otherwise mark tests as done.
    echo $failed_tests ? ' failed!' . PHP_EOL : ' done.' . PHP_EOL;

    # Loop through each failed test and print its results
    foreach ($failed_tests as $failed_test) {
        echo $failed_test;
    }

    # Adjust the total test count if Tests were skipped.
    if ($skipped_count > 0) {
        $test_count = $test_count - $skipped_count;
    }

    echo '---------------------------------------------------------' . PHP_EOL;
    echo "Ran all tests: $succeed_count/$test_count tests passed. $skipped_count tests skipped." . PHP_EOL;
    exit($exit_code);
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
    echo "  backup              : Makes a backup of the SAML2 configuration".PHP_EOL;
    echo "  restore             : Restores the SAML2 configuration from the JSON backup".PHP_EOL;
    echo "  refreshcache        : Refreshes the releases cache files used for updates".PHP_EOL;
    echo "  setupschedule       : Sets up cron schedules for the package".PHP_EOL;
    echo "  removeschedule      : Removes cron schedules for the package".PHP_EOL;
    echo "  update              : Update to the latest version of the package".PHP_EOL;
    echo "  revert <version>    : Revert to a specific version of the package".PHP_EOL;
    echo "  runtests [<filter>] : Runs all test cases, or only those that contain <filter> in the test name".PHP_EOL;
    echo "  version             : Displays the current version of pfSense-pkg-saml2-auth".PHP_EOL;
    echo "  help                : Displays the help page (this page)".PHP_EOL.PHP_EOL;
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
        case 'setupschedule':
            setupschedule();
            break;
        case 'removeschedule':
            removeschedule();
            break;
        case 'update':
            update();
            break;
        case 'revert':
            revert(version: $argv[2] ?? '');
            break;
        case 'runtests':
            run_tests(contains: $argv[2] ?? '');
            break;
        case 'version':
            version();
            break;
        case 'help':
            help();
            exit(0);
        default:
            echo "error: unknown command: '$argv[1]'".PHP_EOL;
            help();
            exit(1);
    }
}

main($argv);
