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

namespace Saml2\Core;

require_once 'Saml2/autoload.php';

use Exception;
use OneLogin\Saml2\Error;
use OneLogin\Saml2\IdPMetadataParser;
use OneLogin\Saml2\Settings;
use OneLogin\Saml2\Utils;
use Saml2\Errors\SystemError;

/**
 * Defines the main SAML2 authentication class. This class is responsible for handling all SAML2 related processes,
 * including initiating SSO, processing ACS responses, and generating SP metadata.
 */
class Auth {
    const ASSERTION_CACHE_FILE = '/var/cache/pfSense-pkg-saml2-auth/assertions.json';
    const ASSERTION_CACHE_TTL = 600;
    const SP_METADATA_URL = '/saml2_auth/sso/metadata/';
    const SP_ACS_URL = '/saml2_auth/sso/acs/';
    const SSO_URL = '/saml2_auth/sso/';
    const SSO_ERROR_URL = '/saml2_auth/sso/error/';
    const SSO_REDIRECT_URL = '/saml2_auth/sso/redirect/';

    public \OneLogin\Saml2\Auth $auth;
    public Config $config;

    /**
     * Constructor for the Saml2 Auth class. This class contains the handlers needed to initiate the SAML2
     * auth processes using the current configuration.
     * @param bool $no_auth
     */
    public function __construct(bool $no_auth = false) {
        session_start();
        $this->config = new Config();

        # Only try to start authentication if the object wasn't requested without authentication
        if (!$no_auth) {
            # Try to start SAML2 authentication, handle errors accordingly
            try {
                $this->__check_enabled();
                $this->auth = new \OneLogin\Saml2\Auth($this::get_saml_settings());
            } catch (\Throwable $error) {
                $this->log(level: LOG_ERR, message: $error->getMessage());
                header('Location: ' . self::SSO_ERROR_URL);
                exit();
            }
        }
    }

    /**
     * Initiates the SSO login.
     * @param string $redirect The URL to redirect to.
     * @return void
     */
    public function sso(string $redirect): void {
        # Invalidate any previous incomplete login before generating a replacement request
        unset($_SESSION['saml2_started']);
        unset($_SESSION['AuthNRequestID']);
        unset($_SESSION['saml2_relay_state']);

        # Try to start SAML2 authentication, handle errors accordingly
        try {
            # php-saml exits when stay is false, so build the redirect first and persist the request state ourselves
            $url = $this->auth->login($redirect, stay: true);
            $request_id = $this->auth->getLastRequestID();
            if (!$request_id or !$url) {
                throw new SystemError('Unable to initiate SAML2 authentication.');
            }

            $_SESSION['saml2_started'] = true;
            $_SESSION['AuthNRequestID'] = $request_id;
            $_SESSION['saml2_relay_state'] = $redirect;
            header('Location: ' . $url);
            exit();
        } catch (\Throwable $error) {
            $this->log(level: LOG_ERR, message: $error->getMessage());
            header('Location: ' . self::SSO_ERROR_URL);
            exit();
        }
    }

    /**
     * The ACS handler for SAML2 responses. This method is responsible for processing the SAML2 response from
     * the IdP and setting session variables accordingly.
     */
    public function acs(): void {
        # Check the state of SAML2 authentication. Only proceeds if in expected state.
        $this->__check_saml2_state();
        $request_id = $_SESSION['AuthNRequestID'];
        $relay_state = $_SESSION['saml2_relay_state'] ?? null;
        unset($_SESSION['saml2_started']);
        unset($_SESSION['AuthNRequestID']);
        unset($_SESSION['saml2_relay_state']);

        # Attempt to process the SAML response. The request ID is consumed before validation to prevent retries.
        try {
            $this->auth->processResponse($request_id);
            if ($this->auth->isAuthenticated()) {
                $assertion_id = $this->auth->getLastAssertionId();
                $assertion_expiry = $this->auth->getLastAssertionNotOnOrAfter();
                if (!$assertion_id or !$this->__consume_assertion_id($assertion_id, $assertion_expiry)) {
                    throw new SystemError('The SAML2 assertion has already been used or has no ID.');
                }
            }
        } catch (\Throwable $error) {
            $this->log(level: LOG_ERR, message: $error->getMessage());
            header('Location: ' . self::SSO_ERROR_URL);
            exit();
        }

        # If the sign on attempt is valid, map attributes to our session array.
        if ($this->auth->isAuthenticated()) {
            # Set session data
            $_SESSION['saml2_auth'] = true;
            $_SESSION['saml2_user_data'] = $this->auth->getAttributes();
            $_SESSION['saml2_name_id'] = $this->auth->getNameId();

            # Log attributes to debug log
            $this->log(
                level: LOG_DEBUG,
                message: "Successful SAML2 sign on for '{$this->auth->getNameId()}', received attributes: " .
                    json_encode($_SESSION['saml2_user_data']),
            );

            # Support RelayState settings
            if ($relay_state and Utils::getSelfURL() != $relay_state) {
                $this->auth->redirectTo($relay_state);
            }
        }

        # Handle SAML errors
        $this->get_saml2_errors();
    }

    /**
     * Sets up SAML2 settings and metadata. This method is responsible for generating and validating the SP metadata.
     */
    public function metadata(): void {
        try {
            # Validate the SP metadata and print the XML metadata if valid
            $settings = new Settings($this->get_saml_settings(), spValidationOnly: true);
            $metadata = $settings->getSPMetadata();
            $errors = $settings->validateMetadata($metadata);
            if (empty($errors)) {
                header('Content-Type: text/xml');
                echo $metadata;
                return;
            }
            throw new Error(msg: 'Invalid SP metadata: ' . implode(', ', $errors), code: Error::METADATA_SP_INVALID);
        } catch (Exception $e) {
            $this->log(level: LOG_ERR, message: $e->getMessage());
            header('Location: ' . self::SSO_ERROR_URL);
            exit();
        }
    }

    /**
     * Writes a log entry to the applicable log file
     * @param string $logfile The log file to write to. Use `auth` to write to the auth log (will print to console) or
     * `saml2` to write to the saml2 package log file.
     */
    public function log(int $level, string $message, string $logfile = 'saml2'): void {
        # Only log debug messages when verbose logging is enabled
        if ($level === LOG_DEBUG and !$this->config->debug_mode) {
            return;
        }

        # Otherwise, write to the applicable log file
        openlog($logfile, flags: LOG_PID, facility: LOG_LOCAL0);
        syslog(priority: $level, message: $message);
        closelog();
    }

    /**
     * Checks the SAML2 state after SSO login. This method is responsible for either redirecting the user to the
     * dashboard if authentication was successfully, or redirecting them back to the SSO URL to start the login process.
     */
    private function __check_saml2_state(): void {
        # Redirect to home page if user is already logged in
        if (!empty($_SESSION['Logged_In'])) {
            header('Location: ' . self::SSO_REDIRECT_URL);
            exit();
        }

        # Redirect to SSO login if the login process has not been started or has no request to validate against
        if (empty($_SESSION['saml2_started']) or empty($_SESSION['AuthNRequestID'])) {
            header('Location: ' . self::SSO_URL);
            exit();
        }
    }

    /**
     * Atomically records a validated assertion ID and rejects IDs that were previously recorded.
     * @param string $assertion_id The unique ID of the validated SAML assertion.
     * @param int|null $assertion_expiry The assertion's NotOnOrAfter timestamp, when supplied by the IdP.
     * @return bool True when the assertion was unused and is now recorded, false when it was already consumed.
     */
    private function __consume_assertion_id(string $assertion_id, int|null $assertion_expiry): bool {
        $cache = fopen(self::ASSERTION_CACHE_FILE, 'c+');
        if ($cache === false or !flock($cache, LOCK_EX)) {
            if (is_resource($cache)) {
                fclose($cache);
            }
            throw new SystemError('Unable to lock the SAML2 assertion replay cache.');
        }

        try {
            rewind($cache);
            $cache_data = json_decode(stream_get_contents($cache), associative: true);
            $cache_data = is_array($cache_data) ? $cache_data : [];
            $now = time();
            $cache_data = array_filter($cache_data, fn($expiry) => is_int($expiry) and $expiry > $now);
            $cache_key = hash('sha256', $assertion_id);

            if (isset($cache_data[$cache_key])) {
                return false;
            }

            $cache_data[$cache_key] = max($now + self::ASSERTION_CACHE_TTL, $assertion_expiry ?? 0);
            rewind($cache);
            if (!ftruncate($cache, 0) or fwrite($cache, json_encode($cache_data)) === false or !fflush($cache)) {
                throw new SystemError('Unable to update the SAML2 assertion replay cache.');
            }
            return true;
        } finally {
            flock($cache, LOCK_UN);
            fclose($cache);
        }
    }

    /**
     * Checks if the SAML2 package is enabled. If not, throws an exception.
     */
    private function __check_enabled(): void {
        if (!$this->config->enable) {
            throw new SystemError('SAML2 authentication is not enabled.');
        }
    }

    /**
     * Checks for SAML2 login errors and logs them if found. This method is also responsible for exiting the session
     * when authentication fails.
     */
    public function get_saml2_errors(): void {
        # If errors are found, destroy the session, log them, and redirect to the error page
        if (!empty($this->auth->getErrors())) {
            session_destroy();
            foreach ($this->auth->getErrors() as $error) {
                $this->log(level: LOG_ERR, message: $error);
            }
            header('Location: ' . self::SSO_ERROR_URL);
        }
    }

    /**
     * Converts the pfSense-pkg-saml2-auth package's configuration to the onelogin/php-saml settings array.
     * @return array The settings array for the onelogin/php-saml library
     */
    public function get_saml_settings(): array {
        $php_saml_config = [
            'debug' => true, // Errors are securely logged to the saml2 log file
            'strict' => true,
            'sp' => [
                'entityId' => $this->config->sp_base_url . self::SP_METADATA_URL,
                'assertionConsumerService' => [
                    'url' => $this->config->sp_base_url . self::SP_ACS_URL,
                ],
                'NameIDFormat' => 'urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified',
            ],
            'idp' => [
                'entityId' => $this->config->idp_entity_id,
                'singleSignOnService' => [
                    'url' => $this->config->idp_sign_on_url,
                ],
                'x509cert' => $this->config->idp_x509_cert,
            ],
        ];

        # When an IdP metadata URL is configured, allow the IdP metadata to override the current IdP settings
        if ($this->config->idp_metadata_url) {
            try {
                $idp_metadata = IdPMetadataParser::parseRemoteXML($this->config->idp_metadata_url);
                $php_saml_config = array_replace_recursive($php_saml_config, $idp_metadata);
                $this->log(
                    level: LOG_DEBUG,
                    message: "Fetched IdP metadata from {$this->config->idp_metadata_url} " .
                        'yielded: ' .
                        json_encode($idp_metadata),
                );
            } catch (Exception $error) {
                $this->log(level: LOG_ERR, message: $error->getMessage());
                header('Location: ' . self::SSO_ERROR_URL);
            }
        }

        # When custom parameters are configured, update the config to include them
        if ($this->config->custom_conf) {
            # Decode the custom configuration values. This should be a Base64 encoded JSON string
            $custom_conf = json_decode($this->config->custom_conf, true);

            # Only merge custom configuration in if it decodes to an array
            if (is_array($custom_conf)) {
                $php_saml_config = array_replace_recursive($php_saml_config, $custom_conf);
            }
        }

        # Request correlation is mandatory and cannot be disabled by IdP metadata or custom configuration
        $php_saml_config['strict'] = true;
        $php_saml_config['security']['rejectUnsolicitedResponsesWithInResponseTo'] = true;

        # Log the final configuration at debug level
        $this->log(level: LOG_DEBUG, message: 'Using SAML2 settings: ' . json_encode($php_saml_config));

        return $php_saml_config;
    }
}
