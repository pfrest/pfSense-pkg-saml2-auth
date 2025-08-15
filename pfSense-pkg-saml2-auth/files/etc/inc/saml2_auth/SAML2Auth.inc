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

require_once("robrichards/xmlseclibs/src/XMLSecurityKey.php");
require_once("robrichards/xmlseclibs/src/XMLSecurityDSig.php");
require_once("robrichards/xmlseclibs/src/XMLSecEnc.php");
require_once("robrichards/xmlseclibs/src/Utils/XPath.php");
require_once("onelogin/php-saml/_toolkit_loader.php");
require_once("config.inc");

class SAML2Auth {
    public $auth;
    public $file_path;

    # Constructs the saml2_auth object by loading SAML2 settings and creating the OneLogin_Saml2_Auth object
    public function __construct($no_auth=false) {
        session_start();
        $this->file_path = "/usr/local/share/pfSense-pkg-saml2-auth";

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

    # Initiates the SSO login. Requires a URL to redirect to
    public function sso($redirect) {
        # Mark the SAML2 login as in process and start login
        $_SESSION["saml2_started"] = true;
        $this->auth->login($redirect);
    }

    public function acs() {
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

    public function metadata() {
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

    private function __check_saml2_state() {
        # Redirect to home page if user is already logged in
        if ($_SESSION['Logged_In']) {
            header("Location: /saml2_auth/sso/redirect/");
            exit();
        }
        # Redirect to SSO login if the login process has not been started
        elseif (!$_SESSION['saml2_started']) {
            header("Location: /saml2_auth/sso/");
            exit();
        }
    }

    # Writes log data
    private function __log($msg, $console=false) {
        # Print our log message to the console if requested
        if ($console) {
            echo $msg;
        }
    }

    public function get_saml2_errors() {
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

    public static function get_package_config() {
        # Loop through each installed package until we find the saml2-auth package
        foreach (config_get_path("installedpackages/package", []) as $id=>$pkg) {
            if ($pkg["internal_name"] === "saml2-auth") {
                return [$id, $pkg["conf"]];
            }
        }
    }

    public static function get_saml_settings() {
        # Local variables
        $pkg_conf = SAML2Auth::get_package_config()[1];
        $php_saml_config = array (
            'debug' => boolval($pkg_conf["debug_mode"]),
            'sp' => array (
                'entityId' => $pkg_conf["sp_base_url"].'/saml2_auth/sso/metadata/',
                'assertionConsumerService' => array (
                    'url' => $pkg_conf["sp_base_url"].'/saml2_auth/sso/acs/',
                ),
                'NameIDFormat' => 'urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified',
            ),
            'idp' => array (
                'entityId' => $pkg_conf["idp_entity_id"],
                'singleSignOnService' => array (
                    'url' => $pkg_conf["idp_sign_on_url"],
                ),
                'x509cert' => base64_decode($pkg_conf["idp_x509_cert"])
            ),
        );

        # When custom parameters are configured, update the config to include them
        if (!empty($pkg_conf["custom_conf"])) {
            # Decode the custom configuration values. This should be a Base64 encoded JSON string
            $custom_conf = json_decode(base64_decode($pkg_conf["custom_conf"]), true);
            # Only merge custom configuration in if it decodes to an array
            if (is_array($custom_conf)) {
                $php_saml_config = array_merge_recursive($php_saml_config, $custom_conf);
            }
        }
        return $php_saml_config;
    }

    # Fetches the current version of pfSense
    public static function get_pfsense_version($full=false) {
        # Only display the full release name if requested, Otherwise only provide the major and minor.
        return ($full) ? str_replace(PHP_EOL, "", file_get_contents("/etc/version")) : substr(file_get_contents("/etc/version"), 0, 3);
    }

    public static function get_pkg_version() {
        # Pull the raw pkg info for the SAML2 auth package into an array for each line
        $pkg_info = explode(PHP_EOL, shell_exec("pkg-static info pfSense-pkg-saml2-auth"));

        # Loop through each line and check the version
        foreach ($pkg_info as $pkg_line) {
            if (substr($pkg_line, 0, 7 ) === "Version") {
                # Locate the version and format it to a standard semantic version format (x.x.x)
                $version = str_replace(" ", "", $pkg_line);
                $version = explode(":", $version)[1];
                $version = (strlen($version) === 3) ? $version.".0" : $version;
                $version = str_replace("_", ".", $version);
                return $version;
            }
        }
    }

    public static function get_latest_pkg_version() {
        # Fetch our latest version and format it semantically (x.x.x)
        $latest_version = array_key_first(self::get_all_pkg_versions());
        $latest_version = str_replace("v", "", $latest_version);
        return $latest_version;
    }

    public static function get_latest_pkg_release_date() {
        # Loop through each release and locate the latest available releases creation date
        foreach (self::get_pkg_releases()["releases"] as $release) {
            # Check if this releases is latest available for our platform
            if ($release["tag_name"] === "v".self::get_latest_pkg_version()) {
                return $release["created_at"];
            }
        }
    }

    public static function is_update_available() {
        # Check if the current version is less than the latest version
        return version_compare(self::get_pkg_version(), self::get_latest_pkg_version(), operator: "<");
    }

    public static function is_pkg_supported() {
        # Local variables
        $overrides_dir = "/etc/inc/saml2_auth/overrides/";
        $overrides = glob($overrides_dir."*" , GLOB_ONLYDIR);
        $pf_version = self::get_pfsense_version(true);

        # Check if this version of pfSense has a supported file override
        if (in_array($overrides_dir.$pf_version, $overrides)) {
            return true;
        } else {
            return false;
        }
    }

    public static function get_pkg_releases() {
        # Variables
        $releases_file = "/usr/local/share/pfSense-pkg-saml2-auth/releases.json";
        $releases = json_decode(file_get_contents($releases_file), true);
        $fetch_releases_cmd = "curl -s https://api.github.com/repos/pfrest/pfSense-pkg-saml2-auth/releases -m 10";

        # Check if our previous releases data is older than 120 seconds
        if ($releases["last_updated"] + 120 < time()) {
            # Pull the releases data from Github and rewrite the releases file
            $api_resp = shell_exec($fetch_releases_cmd);
            $api_resp = json_decode($api_resp, true);
            $releases = ["last_updated" => time(), "releases" => $api_resp];
            file_put_contents($releases_file, json_encode($releases));
        }
        return $releases;
    }

    public static function get_all_pkg_versions() {
        # Variables
        $pf_version = substr(self::get_pfsense_version(), 0, 3);
        $versions = [];
        $count = 0;
        $releases = self::get_pkg_releases()["releases"];

        # Loop through each version and populate our version data
        foreach ($releases as $release) {
            # Loop through the assets of each release and check if our version of pfSense is supported
            foreach ($release["assets"] as $asset) {
                if ($asset["name"] === "pfSense-".$pf_version."-pkg-saml2-auth.pkg") {
                    # The first item of our list is the latest release, mark it as such.
                    if ($count === 0) {
                        $versions[$release["tag_name"]] = $release["name"]." - Latest";
                        $count++;
                    } else {
                        $versions[$release["tag_name"]] = $release["name"];
                    }
                }
            }
        }
        return $versions;
    }

    # Pulls our current pfSense SAML2 configuration and saves to a non-volatile location.
    public function backup_config($console=false) {
        # Local variables
        $path = $this->file_path."/backup.json";
        $config_data = $this->get_package_config()[1];

        # Print status message
        $this->__log("Backing up configuration...", $console);

        # Save a JSON file containing the data
        file_put_contents($path, json_encode($config_data));
        $this->__log("done." . PHP_EOL, $console);
        return true;
    }

    # Restores the pfSense SAML2 configuration from a specified backup file
    public function restore_config($console=false) {
        # Local variables
        $path = $this->file_path."/backup.json";
        $config_id = $this->get_package_config()[0];
        $config_json = file_get_contents($path);
        $config_data = json_decode($config_json, true);

        # Print status message
        $this->__log("Restoring configuration...", $console);

        # Save the backup configuration to the pfSense master configuration if found
        if (file_exists($path)) {
            config_set_path("installedpackages/package/{$config_id}/conf", $config_data);
            write_config("Restoring SAML2 configuration");
            $this->__log("done." . PHP_EOL, $console);
            return true;
        }
        else {
            $this->__log("no backup found.".PHP_EOL, $console);
            return false;
        }
    }

    # Update to the latest version of the package
    public function update_pkg($version="latest", $console=false) {
        # Local variables
        $base_url = "https://github.com/pfrest/pfSense-pkg-saml2-auth/releases";

        # Format the package URL based on the requested version
        if ($version === "latest") {
            $url = $base_url."/latest/download/pfSense-".$this->get_pfsense_version()."-pkg-saml2-auth.pkg";
        } else {
            $url = $base_url."/download/".$version."/pfSense-".$this->get_pfsense_version()."-pkg-saml2-auth.pkg";
        }

        # Backup the package configuration before updating
        $this->backup_config();

        # Remove the existing package and add the new one, then og the results
        exec("pkg-static delete -y pfSense-pkg-saml2-auth", $del_cmd_out, $del_cmd_rc);
        exec("pkg-static add ".escapeshellarg($url), $add_cmd_out, $add_cmd_rc);
        $this->__log(implode("\n", $del_cmd_out), $console);
        $this->__log(implode("\n", $add_cmd_out), $console);

        # Check if the update was successful
        if ($del_cmd_rc === 0 and $add_cmd_rc === 0) {
            return true;
        } else {
            return false;
        }
    }
}
