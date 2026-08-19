<?php

namespace Saml2\Tests;

require_once 'Saml2/autoload.php';

use ReflectionMethod;
use Saml2\Core\Auth;
use Saml2\Core\Config;
use Saml2\Core\TestCase;
use Saml2\Core\TestCaseRetry;
use Saml2\Errors\SystemError;
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
     * Invokes a private Auth method for test coverage.
     * @param Auth $auth
     * @param string $method_name
     * @return mixed
     */
    private function invoke_private_method(Auth $auth, string $method_name): mixed {
        $method = new ReflectionMethod(Auth::class, $method_name);
        return $method->invoke($auth);
    }

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

    /**
     * Checks that __check_enabled() allows authentication to proceed when the package is enabled.
     */
    public function test_check_enabled_with_enabled_package(): void {
        $auth = new Auth(no_auth: true);
        $auth->config->enable = true;

        $this->assert_does_not_throw(function () use ($auth) {
            $this->invoke_private_method($auth, '__check_enabled');
        });
    }

    /**
     * Checks that __check_enabled() throws a SystemError when the package is disabled.
     */
    public function test_check_enabled_with_disabled_package(): void {
        $auth = new Auth(no_auth: true);
        $auth->config->enable = false;

        $this->assert_throws(
            exceptions: [SystemError::class],
            callable: function () use ($auth) {
                $this->invoke_private_method($auth, '__check_enabled');
            },
        );
    }

    /**
     * Checks that Auth(no_auth: true) still constructs when the package is disabled so tests and utility callers can
     * inspect config without attempting authentication.
     */
    public function test_construct_with_no_auth_when_disabled(): void {
        $conf = new Config([
            'enable' => '',
            'strip_username' => '',
            'debug_mode' => 'yes',
            'idp_metadata_url' => 'https://idp.example.com/metadata',
            'idp_entity_id' => 'https://idp.example.com/entity',
            'idp_sign_on_url' => 'https://idp.example.com/sso',
            'idp_x509_cert' => base64_encode("-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----"),
            'idp_groups_attribute' => 'groups',
            'sp_base_url' => 'https://sp.example.com',
            'custom_conf' => base64_encode('{"test_key": "test_value"}'),
        ]);
        $conf->save();

        $auth = null;
        $this->assert_does_not_throw(function () use (&$auth) {
            $auth = new Auth(no_auth: true);
        });

        $this->assert_is_false($auth->config->enable);
        $this->assert_is_false(isset($auth->auth));
    }
}
