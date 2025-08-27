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

use Saml2\Errors\ConfigError;
use Saml2\Errors\ValidationError;

/**
 * Defines the configuration class for the pfSense-pkg-saml2-auth package.
 */
class Config
{
    const BACKUP_FILE_PATH = "/usr/local/pkg/saml2_auth/backup.json";
    const RESTORE_SUCCESS = 0;
    const RESTORE_FAILURE = 1;
    const RESTORE_NO_BACKUP = 2;

    public int $id;
    public bool $enable;
    public bool $strip_username;
    public bool $debug_mode;
    public string $idp_entity_id;
    public string $idp_sign_on_url;
    public string $idp_groups_attribute;
    public string $idp_x509_cert;
    public string $sp_base_url;
    public string $custom_conf;

    /**
     * Constructs the Config object. This ensures the values currently in the configuration file are loaded
     * into this object's properties.
     * @param array $data Optional configuration data to initialize the object with. If empty, the current
     * pfSense-pkg-saml2-auth package configuration data will be loaded.
     */
    public function __construct(array $data = []) {
        # Load this object using the current pfSense-pkg-saml2-auth package configuration data
        $this->from_internal($data);
    }

    /**
     * Obtains the current pfSense-pkg-saml2-auth package configuration data.
     * @return array Returns an array containing the package ID and its configuration data.
     */
    public function get_raw_config(): array {
        // Get the data from pfSense, which could be a reference or an iterator object.
        $package_data = config_get_path("installedpackages/package", []);

        // **THE FIX**: Force the data into a new, clean array.
        // This creates a true copy with its own pointer, making our function
        // immune to any prior iterations, solving the issue permanently.
        $packages = is_array($package_data) ? $package_data : iterator_to_array($package_data);

        // Now, loop over our safe, local copy.
        foreach ($packages as $id => $pkg) {
            if (is_array($pkg) && isset($pkg["internal_name"]) && $pkg["internal_name"] === "saml2-auth") {
                return ["id" => $id, "conf" => $pkg["conf"]];
            }
        }

        # If we reach this point, the package is not installed or the configuration is missing
        throw new ConfigError("pfSense-pkg-saml2-auth package configuration not found.");
    }

    /**
     * Converts this object's properties into an array of pfSense configuration data.
     */
    public function to_internal(): array
    {
        return [
            "enable" => $this->to_internal_boolval($this->enable),
            "strip_username" => $this->to_internal_boolval($this->strip_username),
            "debug_mode" => $this->to_internal_boolval($this->debug_mode),
            "idp_entity_id" => $this->idp_entity_id,
            "idp_sign_on_url" => $this->idp_sign_on_url,
            "idp_groups_attribute" => $this->idp_groups_attribute,
            "idp_x509_cert" => base64_encode($this->idp_x509_cert),
            "sp_base_url" => $this->sp_base_url,
            "custom_conf" => base64_encode($this->custom_conf)
        ];
    }

    /**
     * Loads this object's properties from an array of pfSense configuration data.
     * @param array $config_data The configuration data to load into this object.
     */
    public function from_internal(array $config_data): void {
        # Read the current config
        $stored_config_data = $this->get_raw_config();
        $config_data = $config_data ?: $stored_config_data["conf"];


        # Assign values to the class properties based on the configuration data
        $this->id = $stored_config_data["id"];
        $this->enable = $this->from_internal_boolval($config_data["enable"]);
        $this->strip_username = $this->from_internal_boolval($config_data["strip_username"]);
        $this->debug_mode = $this->from_internal_boolval($config_data["debug_mode"]);
        $this->idp_entity_id = $config_data["idp_entity_id"] ?? "";
        $this->idp_sign_on_url = $config_data["idp_sign_on_url"] ?? "";
        $this->idp_groups_attribute = $config_data["idp_groups_attribute"] ?? "";
        $this->idp_x509_cert = $config_data["idp_x509_cert"] ?? "";
        $this->sp_base_url = $config_data["sp_base_url"] ?: $this->__get_default_sp_base_url();
        $this->custom_conf = base64_decode($config_data["custom_conf"] ?? "");
    }

    /**
     * Converts the configs internal boolean representation to a PHP boolean. This is necessary because pfSense
     * does not parse the XML configuration into data types other than strings and arrays.
     * @param string $value The incoming config value to convert.
     * @return bool Returns true if the value is "yes", false otherwise.
     */
    public static function from_internal_boolval($value): bool {
        return $value === "yes";
    }

    /**
     * Converts a PHP boolean to the pfSense internal representation of a boolean.
     * @param bool $value The PHP boolean value to convert.
     * @return string Returns "yes" if true, "" if false.
     */
    public static function to_internal_boolval(bool $value): string
    {
        return $value ? "yes" : "";
    }

    /**
     * Obtains a default sp_base_url using the pfSense system configuration
     * @return string Returns the default SP base URL for the SAML2 authentication package.
     */
    private function __get_default_sp_base_url(): string {
        $protocol = config_get_path("system/webgui/protocol");
        $hostname = config_get_path("system/hostname");
        $domain = config_get_path("system/domain");
        $port = config_get_path("system/webgui/port", "");
        $fqdn = "$hostname.$domain";
        return $port ? "$protocol://$fqdn:$port" : "$protocol://$fqdn";
    }

    /**
     * Validates the idp_entity_idp property.
     * @throws ValidationError If the idp_entity_id is over 1024 characters long or contains invalid characters.
     */
    public function validate_idp_entity_id(): void {
        if (strlen($this->idp_entity_id) > 1024) {
            throw new ValidationError("IDP Entity ID must be less than 1024 characters long.");
        }
        if (!preg_match('/^[a-zA-Z0-9\-._~:\/?#\[\]@!$&\'()*+,;=]+$/', $this->idp_entity_id)) {
            throw new ValidationError("IDP Entity ID contains invalid characters.");
        }
    }

    /**
     * Validates the idp_sign_on_url property.
     * @throws ValidationError If the idp_sign_on_url is not a valid URL
     */
    public function validate_idp_sign_on_url(): void
    {
        if (!filter_var($this->idp_sign_on_url, FILTER_VALIDATE_URL)) {
            throw new ValidationError("IDP Sign-On URL is not a valid URL.");
        }
    }

    /**
     * Validates the idp_x509_cert property.
     * @throws ValidationError If the idp_x509_cert is not a valid X.509 certificate.
     */
    public function validate_idp_x509_cert(): void {
        if (
            !str_contains($this->idp_x509_cert, "-----BEGIN CERTIFICATE-----") or
            !str_contains($this->idp_x509_cert, "-----END CERTIFICATE-----")
        ) {
            throw new ValidationError("IDP X.509 Certificate is not a valid X.509 certificate.");
        }
    }

    /**
     * Validates the sp_base_url property.
     * @throws ValidationError If the sp_base_url is not a valid URL.
     */
    public function validate_sp_base_url(): void {
        if (!filter_var($this->sp_base_url, FILTER_VALIDATE_URL)) {
            throw new ValidationError("SP Base URL is not a valid URL.");
        }
    }

    /**
     * Validates the custom_conf property.
     * @throws ValidationError If the custom_conf is not a valid JSON string.
     */
    public function validate_custom_conf(): void
    {
        # Check if the custom_conf is a valid JSON string
        json_decode($this->custom_conf);
        if (json_last_error() !== JSON_ERROR_NONE) {
            throw new ValidationError("Custom configuration is not a valid JSON string.");
        }
    }

    /**
     * Validates the configuration data. This method checks all properties for validity and throws an exception if any
     * validation fails.
     * @throws ValidationError If any of the configuration properties are invalid.
     */
    public function validate(): void {
        $this->validate_idp_entity_id();
        $this->validate_idp_sign_on_url();
        $this->validate_idp_x509_cert();
        $this->validate_sp_base_url();
        $this->validate_custom_conf();
    }

    /**
     * Saves changes made to this object's properties to the pfSense configuration.
     */
    public function save(): void
    {
        # Validate the configuration data before saving
        $this->validate();

        # Save the configuration data to the pfSense master configuration and backup the config
        config_set_path("installedpackages/package/$this->id/conf", $this->to_internal());
        write_config("Modified SAML2 configuration");
        $this->backup();
    }

    /**
     * Backups up the current pfSense-pkg-saml2-auth configuration to a persistent JSON file.
     * @return bool Returns true if the backup was successful, false otherwise.
     */
    public function backup(): bool {
        # Save a JSON file containing the data
        $backup_result = file_put_contents(self::BACKUP_FILE_PATH, json_encode($this->get_raw_config()));
        return $backup_result !== false;
    }

    /**
     * Restores the last backup of the pfSense-pkg-saml2-auth configuration from the persistent JSON file.
     * @return int Returns an integer indicating the result of the restore operation: 0 for success, 1 for failure,
     * and 2 if no backup file was found.
     */
    public function restore(): int {
        # Save the backup configuration to the pfSense master configuration if found
        if (file_exists(self::BACKUP_FILE_PATH)) {
            $backup_data = file_get_contents(self::BACKUP_FILE_PATH);
            $restore_result = config_set_path("installedpackages/package/$this->id/conf", $backup_data);
            write_config("Restored SAML2 configuration");
            return $restore_result ? self::RESTORE_SUCCESS : self::RESTORE_FAILURE;
        }
        else {
            return self::RESTORE_NO_BACKUP;
        }
    }
}