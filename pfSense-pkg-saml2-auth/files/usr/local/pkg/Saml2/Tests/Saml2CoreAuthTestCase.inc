<?php

namespace Saml2\Tests;

require_once 'Saml2/autoload.php';

use Saml2\Core\Auth;
use Saml2\Core\TestCase;
use Saml2\Core\TestCaseRetry;
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
 * A test case to validate the Saml2 Core\Auth class.
 */
class Saml2CoreAuthTestCase extends TestCase {
    /**
     * Checks that log() method writes a new entry to the log file
     */
    #[TestCaseRetry(retries: 3, delay: 1)]
    public function test_log(): void {
        # Generate a unique message to log
        $unique_message = 'Test log entry ' . uniqid();
        $auth = new Auth(no_auth: true);
        $auth->log(level: LOG_INFO, message: $unique_message);

        # Ensure the log entry was written to the log file
        $log_contents = file_get_contents('/var/log/saml2.log');
        $this->assert_str_contains($log_contents, $unique_message);

        # Ensure verbose log entries are not present if verbose logging is disabled
        $auth->config->debug_mode = false;
        $unique_message = 'Verbose log entry ' . uniqid();
        $auth->log(level: LOG_DEBUG, message: $unique_message);
        $log_contents = file_get_contents('/var/log/saml2.log');
        $this->assert_str_does_not_contain($log_contents, $unique_message);

        # Ensure verbose log entries are present if verbose logging is enabled
        $auth->config->debug_mode = true;
        $unique_message = 'Verbose log entry ' . uniqid();
        $auth->log(level: LOG_DEBUG, message: $unique_message);
        $log_contents = file_get_contents('/var/log/saml2.log');
        $this->assert_str_contains($log_contents, $unique_message);
    }
}
