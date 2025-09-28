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

require_once('Saml2/autoload.php');

use Exception;
use OneLogin\Saml2\Error;
use OneLogin\Saml2\IdPMetadataParser;
use OneLogin\Saml2\Settings;
use OneLogin\Saml2\Utils;

/**
 * Defines a class container our SAML2 handlers.
 */
class Auth {
    const SP_METADATA_URL = "/saml2_auth/sso/metadata/";
    const SP_ACS_URL = "/saml2_auth/sso/acs/";
    const SSO_URL = "/saml2_auth/sso/";
    const SSO_ERROR_URL = "/saml2_auth/sso/error/";
    const SSO_REDIRECT_URL = "/saml2_auth/sso/redirect/";
    
    public \OneLogin\Saml2\Auth $auth;
    public Config $config;

    /**
     * Constructor for the Saml2 Auth class. This class contains the handlers needed to initiate the SAML2
     * auth processes using the current configuration.
     * @param bool $no_auth
     */
    public function __construct(bool $no_auth=false) {
        session_start();
        $this->config = new Config();

        # Only try to start authentication if the object wasn't requested without authentication
        if (!$no_auth) {
            # Try to start SAML2 authentication, handle errors accordingly
            try {
                $this->auth = new \OneLogin\Saml2\Auth($this::get_saml_settings());
            } catch (Exception $error) {
                $this->log(level: LOG_ERR, message: $error->getMessage());
                header("Location: ".self::SSO_ERROR_URL);
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
        # Mark the SAML2 login as in process and start login
        $_SESSION["saml2_started"] = true;

        # Try to start SAML2 authentication, handle errors accordingly
        try {
            $this->auth->login($redirect);
        } catch (Exception $error) {
            $this->log(level: LOG_ERR, message: $error->getMessage());
            header("Location: ".self::SSO_ERROR_URL);
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
        unset($_SESSION['saml2_started']);

        # Attempt to process the SAML response
        try {
            $this->auth->processResponse($_SESSION['AuthNRequestID']);
        }
        catch (Exception $error) {
            $this->log(level: LOG_ERR, message: $error->getMessage());
            header("Location: ".self::SSO_ERROR_URL);
            exit();
        }

        # If the sign on attempt is valid, map attributes to our session array.
        if ($this->auth->isAuthenticated()) {
            # Set session data
            $_SESSION["saml2_auth"] = true;
            $_SESSION['saml2_user_data'] = $this->auth->getAttributes();
            $_SESSION['saml2_name_id'] = $this->auth->getNameId();
            unset($_SESSION['AuthNRequestID']);

            # Support RelayState settings
            if (isset($_POST['RelayState']) && Utils::getSelfURL() != $_POST['RelayState']) {
                $this->auth->redirectTo($_POST['RelayState']);
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
            throw new Error (
                msg: 'Invalid SP metadata: '.implode(', ', $errors),
                code: Error::METADATA_SP_INVALID
            );
        } catch (Exception $e) {
            $this->log(level: LOG_ERR, message: $e->getMessage());
            header("Location: ".self::SSO_ERROR_URL);
            exit();

        }
    }

    /**
     * Writes a log entry to the applicable log file
     * @param string $logfile The log file to write to. Use `auth` to write to the auth log (will print to console) or
     * `saml2` to write to the saml2 package log file.
     */
    public function log(int $level, string $message, string $logfile="saml2"): void
    {
        # Only log debug messages when verbose logging is enabled
        if ($level === LOG_DEBUG and !$this->config->verbose_logging) {
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
        if ($_SESSION['Logged_In']) {
            header("Location: ".self::SSO_REDIRECT_URL);
            exit();
        }

        # Redirect to SSO login if the login process has not been started
        if (!$_SESSION['saml2_started']) {
            header("Location: ".self::SSO_URL);
            exit();
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
            header("Location: ".self::SSO_ERROR_URL);
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
                'entityId' => $this->config->sp_base_url.self::SP_METADATA_URL,
                'assertionConsumerService' => [
                    'url' => $this->config->sp_base_url.self::SP_ACS_URL,
                ],
                'NameIDFormat' => 'urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified',
            ],
            'idp' => [
                'entityId' => $this->config->idp_entity_id,
                'singleSignOnService' => [
                    'url' => $this->config->idp_sign_on_url,
                ],
                'x509cert' => $this->config->idp_x509_cert
            ],
        ];

        # When an IdP metadata URL is configured, allow the IdP metadata to override the current IdP settings
        if ($this->config->idp_metadata_url) {
            try {
                $idp_metadata = IdPMetadataParser::parseRemoteXML($this->config->idp_metadata_url);
                $php_saml_config = array_replace_recursive($php_saml_config, $idp_metadata);
                $this->log(
                    level: LOG_DEBUG,
                    message: "Fetched IdP metadata from {$this->config->idp_metadata_url} ".
                        "yielded: ".json_encode($idp_metadata)
                );
            }
            catch (Exception $error) {
                $this->log(level: LOG_ERR, message: $error->getMessage());
                header("Location: ".self::SSO_ERROR_URL);
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

        # Log the final configuration at debug level
        $this->log(level: LOG_DEBUG, message: "Using SAML2 settings: ".json_encode($php_saml_config));

        return $php_saml_config;
    }
}
