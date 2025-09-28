<?php

namespace Saml2\Tests;

require_once("Saml2/autoload.php");

use Saml2\Core\Config;
use Saml2\Core\TestCase;
use Saml2\Errors\ConfigError;
use Saml2\Errors\ValidationError;


/**
 * A test case to validate the Saml2 Core\Config class.
 */
class Saml2CoreConfigTestCase extends TestCase
{
    /**
     * Test that Config class objects are properly constructed
     */
    public function test_construct_config(): void {
        # Construct a new Config object with specific internal config values
        $conf = new Config(
            [
                "enable" => "yes",
                "strip_username" => "",
                "verbose_logging" => "yes",
                "idp_metadata_url" => "https://idp.example.com/metadata",
                "idp_entity_id" => "https://idp.example.com/entity",
                "idp_sign_on_url" => "https://idp.example.com/sso",
                "idp_x509_cert" => base64_encode("-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----"),
                "idp_groups_attribute" => "groups",
                "sp_base_url" => "https://sp.example.com",
                "custom_conf" => base64_encode('{"test_key": "test_value"}'),
            ]
        );

        # Ensure each config value is properly set and retrieved
        $this->assert_equals($conf->enable, true);
        $this->assert_equals($conf->strip_username, false);
        $this->assert_equals($conf->verbose_logging, true);
        $this->assert_equals($conf->idp_metadata_url, "https://idp.example.com/metadata");
        $this->assert_equals($conf->idp_entity_id, "https://idp.example.com/entity");
        $this->assert_equals($conf->idp_sign_on_url, "https://idp.example.com/sso");
        $this->assert_equals($conf->idp_groups_attribute, "groups");
        $this->assert_equals($conf->sp_base_url, "https://sp.example.com");
        $this->assert_equals($conf->custom_conf, '{"test_key": "test_value"}');
        $this->assert_equals(
            $conf->idp_x509_cert,
            "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----"
        );
    }

    /**
     * Checks that the get_raw_config() method returns the original config value
     */
    public function test_get_raw_config(): void {
        $conf = new Config();
        $this->assert_is_true(is_array($conf->get_raw_config()));
    }

    /**
     * Checks that the get_raw_config() method throws an error if the package config is not found
     */
    public function test_get_raw_config_throws_not_found_error(): void
    {
        $conf = new Config();

        # Temporary unset the package config
        config_del_path("installedpackages/package/$conf->id");

        $this->assert_throws(
            exceptions: [ConfigError::class],
            callable: function () use ($conf) {
                $conf->get_raw_config();
            }
        );
    }

    /**
     * Checks that the to_internal() method correctly converts the Config object into the expected pfSense config array
     */
    public function test_to_internal(): void {
        # Create a new config object with our desired values
        $conf = new Config();
        $conf->enable = true;
        $conf->strip_username = true;
        $conf->verbose_logging = false;
        $conf->idp_metadata_url = "https://idp.example.com/metadata";
        $conf->idp_entity_id = "https://idp.example.com/entity";
        $conf->idp_sign_on_url = "https://idp.example.com/sso";
        $conf->idp_groups_attribute = "groups";
        $conf->idp_x509_cert = "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----";
        $conf->sp_base_url = "https://sp.example.com";
        $conf->custom_conf = '{"test_key": "test_value"}';

        # Convert to internal config format and ensure the values are as expected
        $internal = $conf->to_internal();
        $this->assert_equals($internal["enable"], "yes");
        $this->assert_equals($internal["strip_username"], "yes");
        $this->assert_equals($internal["verbose_logging"], "");
        $this->assert_equals($internal["idp_metadata_url"], "https://idp.example.com/metadata");
        $this->assert_equals($internal["idp_entity_id"], "https://idp.example.com/entity");
        $this->assert_equals($internal["idp_sign_on_url"], "https://idp.example.com/sso");
        $this->assert_equals($internal["idp_groups_attribute"], "groups");
        $this->assert_equals(
            $internal["idp_x509_cert"],
            base64_encode("-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----")
        );
        $this->assert_equals($internal["sp_base_url"], "https://sp.example.com");
        $this->assert_equals($internal["custom_conf"], base64_encode('{"test_key": "test_value"}'));
    }

    /**
     * Checks that the from_internal() method correctly converts a pfSense config array into a Config object
     */
    public function test_from_internal(): void {
        # Construct a new Config object with specific internal config values
        $conf = new Config();

        # Load the $conf object from internal
        $conf->from_internal(
            [
                "enable" => "yes",
                "strip_username" => "",
                "verbose_logging" => "yes",
                "idp_metadata_url" => "https://idp.example.com/metadata",
                "idp_entity_id" => "https://idp.example.com/entity",
                "idp_sign_on_url" => "https://idp.example.com/sso",
                "idp_x509_cert" => base64_encode("-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----"),
                "idp_groups_attribute" => "groups",
                "sp_base_url" => "https://sp.example.com",
                "custom_conf" => base64_encode('{"test_key": "test_value"}'),
            ]
        );

        # Ensure each config value is properly set and retrieved
        $this->assert_equals($conf->enable, true);
        $this->assert_equals($conf->strip_username, false);
        $this->assert_equals($conf->verbose_logging, true);
        $this->assert_equals($conf->idp_metadata_url, "https://idp.example.com/metadata");
        $this->assert_equals($conf->idp_entity_id, "https://idp.example.com/entity");
        $this->assert_equals($conf->idp_sign_on_url, "https://idp.example.com/sso");
        $this->assert_equals($conf->idp_groups_attribute, "groups");
        $this->assert_equals($conf->sp_base_url, "https://sp.example.com");
        $this->assert_equals($conf->custom_conf, '{"test_key": "test_value"}');
        $this->assert_equals(
            $conf->idp_x509_cert,
            "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----"
        );
    }

    /**
     * Checks that the from_internal_boolval() method correctly converts the exected truthy and falsy values
     * config values
     */
    public function test_from_internal_boolval(): void {
        $this->assert_is_true(Config::from_internal_boolval("yes"));
        $this->assert_is_false(Config::from_internal_boolval(""));
        $this->assert_is_false(Config::from_internal_boolval("invalid"));
    }

    /**
     * Checks that the to_internal_boolval() method correctly converts boolean values to the expected internal
     * config values
     */
    public function test_to_internal_boolval(): void
    {
        $this->assert_equals(Config::to_internal_boolval(true), "yes");
        $this->assert_equals(Config::to_internal_boolval(false), "");
    }

    /**
     * Checks that the __get_default_sp_base_url() method correctly constructs the expected base URL using the
     * current pfSense webConfigurator values
     */
    public function test_get_default_sp_base_url(): void
    {
        config_set_path("system/webgui/protocol", "https");
        config_set_path("system/hostname", "saml2");
        config_set_path("system/domain", "example.com");
        $this->assert_equals(
            Config::get_default_sp_base_url(),
            "https://saml2.example.com"
        );

        config_set_path("system/webgui/protocol", "http");
        config_set_path("system/webgui/port", "8080");
        config_set_path("system/hostname", "pfsense");
        config_set_path("system/domain", "saml2.example.com");
        $this->assert_equals(
            Config::get_default_sp_base_url(),
            "http://pfsense.saml2.example.com:8080"
        );
    }

    /**
     * Checks that the validate_idp_metadata_url() method correctly validates acceptable and unacceptable values
     */
    public function test_validate_idp_metadata_url(): void {
        $conf = new Config();

        # Valid URL should not throw an error
        $conf->idp_metadata_url = "https://idp.example.com/metadata";
        $conf->validate_idp_metadata_url();

        # Invalid URL should throw a ValidationError
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function() use ($conf) {
                $conf->idp_metadata_url = "invalid-url!";
                $conf->validate_idp_metadata_url();
            }
        );
    }

    /**
     * Checks that the validate_dip_entity_id() method correctly validates acceptable and unacceptable values
     */
    public function test_validate_idp_entity_id(): void
    {
        $conf = new Config();

        # Valid entity ID should not throw an error
        $conf->idp_entity_id = "https://idp.example.com/entity";
        $conf->validate_idp_entity_id();

        # Values over 1024 characters should throw a ValidationError
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () use ($conf) {
                $conf->idp_entity_id = str_repeat("a", 1025);
                $conf->validate_idp_entity_id();
            }
        );

        # Values that don't match '/^[a-zA-Z0-9\-._~:\/?#\[\]@!$&\'()*+,;=]+$/'
        $bad_values = [
            "invalid entity id with spaces",
            "invalid/entity\\id",
            "invalid<entity>id",
            "invalid|entity{id}",
            "invalid^entity`id",
            "invalid\"entity'id",
        ];
        foreach ($bad_values as $value) {
            $this->assert_throws(
                exceptions: [ValidationError::class],
                callable: function () use ($conf, $value) {
                    $conf->idp_entity_id = $value;
                    $conf->validate_idp_entity_id();
                }
            );
        }
    }

    /**
     * Checks that the validate_idp_sign_on_url() method correctly validates acceptable and unacceptable values
     */
    public function test_validate_idp_sign_on_url(): void
    {
        $conf = new Config();

        # Valid URL should not throw an error
        $conf->idp_sign_on_url = "https://idp.example.com/sso";
        $conf->validate_idp_sign_on_url();

        # Ensure value can be empty if metadata URL is set
        $conf->idp_metadata_url = "https://idp.example.com/metadata";
        $conf->idp_sign_on_url = "";
        $conf->validate_idp_sign_on_url();

        # Invalid URL should throw a ValidationError
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () use ($conf) {
                $conf->idp_metadata_url = ""; // Ensure metadata URL is empty to force validation
                $conf->idp_sign_on_url = "invalid-url!";
                $conf->validate_idp_sign_on_url();
            }
        );
    }

    /**
     * Checks that the validate_idp_x509_cert() method correctly validates acceptable and unacceptable values
     */
    public function test_validate_idp_x509_cert(): void
    {
        $conf = new Config();

        # Valid certificate should not throw an error
        $conf->idp_x509_cert = "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----";
        $conf->validate_idp_x509_cert();

        # Ensure value can be empty if metadata URL is set
        $conf->idp_metadata_url = "https://idp.example.com/metadata";
        $conf->idp_x509_cert = "";
        $conf->validate_idp_x509_cert();

        # Invalid certificate should throw a ValidationError
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () use ($conf) {
                $conf->idp_metadata_url = ""; // Ensure metadata URL is empty to force validation
                $conf->idp_x509_cert = "invalid-cert";
                $conf->validate_idp_x509_cert();
            }
        );
    }

    /**
     * Checks that the validate_sp_base_url() method correctly validates acceptable and unacceptable values
     */
    public function test_validate_sp_base_url(): void
    {
        $conf = new Config();

        # Valid URL should not throw an error
        $conf->sp_base_url = "https://sp.example.com";
        $conf->validate_sp_base_url();

        # Invalid URL should throw a ValidationError
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () use ($conf) {
                $conf->sp_base_url = "invalid-url!";
                $conf->validate_sp_base_url();
            }
        );
    }

    /**
     * Checks that the validate_custom_conf() method correctly validates acceptable and unacceptable values
     */
    public function test_validate_custom_conf(): void
    {
        $conf = new Config();

        # Valid JSON should not throw an error
        $conf->custom_conf = '{"test_key": "test_value"}';
        $conf->validate_custom_conf();

        # Empty value should not throw an error
        $conf->custom_conf = "";
        $conf->validate_custom_conf();

        # Invalid JSON should throw a ValidationError
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () use ($conf) {
                $conf->custom_conf = "{invalid-json:}";
                $conf->validate_custom_conf();
            }
        );
    }

    /**
     * Checks that the validate() method correctly validates a complete config object
     */
    public function test_validate(): void
    {
        $conf = new Config();

        # Valid config should not throw an error
        $conf->enable = true;
        $conf->idp_metadata_url = "https://idp.example.com/metadata";
        $conf->idp_entity_id = "https://idp.example.com/entity";
        $conf->idp_sign_on_url = "https://idp.example.com/sso";
        $conf->idp_x509_cert = "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----";
        $conf->idp_groups_attribute = "groups";
        $conf->sp_base_url = "https://sp.example.com";
        $conf->custom_conf = '{"test_key": "test_value"}';
        $conf->validate();

        # Invalid config should throw a ValidationError
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () use ($conf) {
                $conf->enable = true;
                $conf->idp_metadata_url = "invalid-url!";
                $conf->idp_entity_id = str_repeat("a", 1025);
                $conf->idp_sign_on_url = "invalid-url!";
                $conf->idp_x509_cert = "invalid-cert";
                $conf->sp_base_url = "invalid-url!";
                $conf->custom_conf = "{invalid-json:}";
                $conf->validate();
            }
        );
    }

    /**
     * Checks that the save() method correctly saves the config to the pfSense config
     */
    public function test_save(): void {
        $conf = new Config();

        # Valid config should not throw an error
        $conf->enable = true;
        $conf->strip_username = false;
        $conf->verbose_logging = false;
        $conf->idp_metadata_url = "https://idp.example.com/metadata";
        $conf->idp_entity_id = "https://idp.example.com/entity";
        $conf->idp_sign_on_url = "https://idp.example.com/sso";
        $conf->idp_x509_cert = "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----";
        $conf->idp_groups_attribute = "groups";
        $conf->sp_base_url = "https://sp.example.com";
        $conf->custom_conf = '{"test_key": "test_value"}';
        $conf->save();

        # Ensure the config was saved correctly
        $internal = $conf->get_raw_config()["conf"];
        $this->assert_equals($internal["enable"], "yes");
        $this->assert_equals($internal["strip_username"], "");
        $this->assert_equals($internal["verbose_logging"], "");
        $this->assert_equals($internal["idp_metadata_url"], "https://idp.example.com/metadata");
        $this->assert_equals($internal["idp_entity_id"], "https://idp.example.com/entity");
        $this->assert_equals($internal["idp_sign_on_url"], "https://idp.example.com/sso");
        $this->assert_equals($internal["idp_groups_attribute"], "groups");
        $this->assert_equals(
            $internal["idp_x509_cert"],
            base64_encode("-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----")
        );
        $this->assert_equals($internal["sp_base_url"], "https://sp.example.com");
        $this->assert_equals($internal["custom_conf"], base64_encode('{"test_key": "test_value"}'));
    }

    /**
     * Checks that the backup() method correctly creates a backup of the current config
     */
    public function test_backup(): void
    {
        $conf = new Config();
        $conf->enable = true;
        $conf->strip_username = false;
        $conf->verbose_logging = false;
        $conf->idp_metadata_url = "https://idp.example.com/metadata";
        $conf->idp_entity_id = "https://idp.example.com/entity";
        $conf->idp_sign_on_url = "https://idp.example.com/sso";
        $conf->idp_x509_cert = "-----BEGIN CERTIFICATE-----\n" . random_int(0, 100) . "\n-----END CERTIFICATE-----";
        $conf->idp_groups_attribute = "groups";
        $conf->sp_base_url = "https://sp.example.com";
        $conf->custom_conf = '{"test_key": "test_value"}';
        $conf->save(); // Calls backup() internally

        # Ensure the backup was created correctly
        $this->assert_is_true(file_exists(Config::BACKUP_FILE_PATH));

        # Read the backup file and ensure it matches the current config
        $backup_content = file_get_contents(Config::BACKUP_FILE_PATH);
        $this->assert_equals($backup_content, json_encode($conf->to_internal()));
    }

    /**
     * Checks that restore() method correctly restores a config from the backup file
     */
    public function test_restore(): void {
        # Ensure a backup file exists
        $conf = new Config();
        $conf->backup();

        # Load, modify and rewrite the backup file
        $backup_data = json_decode(file_get_contents(Config::BACKUP_FILE_PATH), associative: true);
        $date = date("YmdHis");
        $backup_data["idp_entity_id"] = $date; // Change to a unique value
        file_put_contents(Config::BACKUP_FILE_PATH, json_encode($backup_data));

        # Restore the config from the backup file
        $this->assert_equals($conf->restore(), Config::RESTORE_SUCCESS);
        $conf = new Config(); // Reload the config
        $this->assert_equals($conf->idp_entity_id, $date);

        # Remove the backup file and ensure restore() returns the expected code/int
        unlink(Config::BACKUP_FILE_PATH);
        $this->assert_equals($conf->restore(), Config::RESTORE_NO_BACKUP);
    }
}