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

namespace Saml2;

require_once('Saml2/autoload.php');

use Exception;

/**
 * Defines a class container our SAML2 handlers.
 */
class Auth {
    const SP_METADATA_URL = "/saml2_auth/sso/metadata/";
    const SP_ACS_URL = "/saml2_auth/sso/acs/";
    
    public OneLogin\Saml2\Auth $auth;
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
                $this->auth = new OneLogin\Saml2\Auth($this::get_saml_settings());
            } catch (OneLogin\Saml2\Error $error) {
                echo $error->getMessage().PHP_EOL;
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
        $this->auth->login($redirect);
    }

    /**
     * The ACS handler for SAML2 responses.
     */
    public function acs(): void {
        # Check the state of SAML2 authentication. Only proceeds if in expected state.
        $this->__check_saml2_state();

        # Remove the saml2_started handler and process our request response
        unset($_SESSION['saml2_started']);
        $this->auth->processResponse($_SESSION['AuthNRequestID']);

        # If the sign on attempt is valid, map attributes to our session array.
        if ($this->auth->isAuthenticated()) {
            # Set session data
            $_SESSION["saml2_auth"] = true;
            $_SESSION['saml2_user_data'] = $this->auth->getAttributes();
            $_SESSION['saml2_name_id'] = $this->auth->getNameId();
            unset($_SESSION['AuthNRequestID']);

            # Support RelayState settings
            if (isset($_POST['RelayState']) && OneLogin\Saml2\Utils::getSelfURL() != $_POST['RelayState']) {
                $this->auth->redirectTo($_POST['RelayState']);
            }
        }

        # Handle SAML errors
        $this->get_saml2_errors();
    }

    /**
     * Sets up SAML2 settings and metadata.
     */
    public function metadata(): void {
        try {
            # Validate the SP metadata and print the XML metadata if valid
            $settings = new OneLogin\Saml2\Settings($this->get_saml_settings(), true);
            $metadata = $settings->getSPMetadata();
            $errors = $settings->validateMetadata($metadata);
            if (empty($errors)) {
                header('Content-Type: text/xml');
                echo $metadata;
            } else {
                throw new OneLogin\Saml2\Error (
                    'Invalid SP metadata: '.implode(', ', $errors),
                    OneLogin\Saml2\Error::METADATA_SP_INVALID
                );
            }
        } catch (Exception $e) {
            echo $e->getMessage();
        }
    }

    /**
     * Checks the SAML2 state after SSO login. This method is responsible for either redirecting the user to the
     * dashboard if authentication was successfully, or redirecting them back to the SSO URL to start the login process.
     */
    private function __check_saml2_state(): void {
        # Redirect to home page if user is already logged in
        if ($_SESSION['Logged_In']) {
            header("Location: /saml2_auth/sso/redirect/");
            exit();
        }

        # Redirect to SSO login if the login process has not been started
        if (!$_SESSION['saml2_started']) {
            header("Location: /saml2_auth/sso/");
            exit();
        }
    }

    /**
     * Checks for SAML2 login errors and logs them if found. This method is also responsible for exiting the session
     * when authentication fails.
     */
    public function get_saml2_errors(): void {
        # Print SAML errors if they exist
        if (!empty($this->auth->getErrors())) {
            session_destroy();
            echo '<pre>',implode(', ', $this->auth->getErrors()),'</pre>';
            if ($this->auth->getSettings()->isDebugActive()) {
                echo '<pre>'.$this->auth->getLastErrorReason().'</pre>';
                exit();
            }
        }
    }

    /**
     * Converts the pfSense-pkg-saml2-auth package's configuration to the onelogin/php-saml settings array.
     * @return array The settings array for the onelogin/php-saml library
     */
    public function get_saml_settings(): array {
        # Local variables
        $php_saml_config = array (
            'debug' => $this->config->debug_mode,
            'sp' => array (
                'entityId' => $this->config->sp_base_url.self::SP_METADATA_URL,
                'assertionConsumerService' => array (
                    'url' => $this->config->sp_base_url.self::SP_ACS_URL,
                ),
                'NameIDFormat' => 'urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified',
            ),
            'idp' => array (
                'entityId' => $this->config->idp_entity_id,
                'singleSignOnService' => array (
                    'url' => $this->config->idp_sign_on_url,
                ),
                'x509cert' => $this->config->idp_x509_cert
            ),
        );

        # When custom parameters are configured, update the config to include them
        if (!empty($this->config->custom_conf)) {
            # Decode the custom configuration values. This should be a Base64 encoded JSON string
            $custom_conf = json_decode($this->config->custom_conf, true);
            # Only merge custom configuration in if it decodes to an array
            if (is_array($custom_conf)) {
                $php_saml_config = array_merge_recursive($php_saml_config, $custom_conf);
            }
        }
        return $php_saml_config;
    }
}
