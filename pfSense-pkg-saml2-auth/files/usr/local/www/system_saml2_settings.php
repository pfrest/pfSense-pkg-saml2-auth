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

require_once 'guiconfig.inc';
require_once 'Saml2/autoload.php';

use Saml2\Core\Config;

# Initialize the pfSense UI page (note: $pgtitle must be defined before including head.inc)
$pgtitle = [gettext('System'), gettext('SAML2'), gettext('Settings')];
include 'head.inc';
$tab_array = [
    [gettext('Settings'), true, '/system_saml2_settings.php'],
    [gettext('Update'), false, '/system_saml2_update.php'],
];
display_top_tabs($tab_array, true); # Ensures the tabs are written to the top of page
# Variables
$form = new Form(false);
$conf = new Config();
$input_errors = [];

# On save, attempt to validate and save the posted configuration
if ($_POST['save']) {
    # Load the requested data into a Config object and validate it
    $_POST['idp_x509_cert'] = base64_encode($_POST['idp_x509_cert']);
    $_POST['custom_conf'] = base64_encode($_POST['custom_conf']);
    $conf->from_internal($_POST);
    try {
        $conf->save();
        print_apply_result_box(0);
    } catch (Error $e) {
        $input_errors[] = $e->getMessage();
        print_input_errors($input_errors);
    }
}

# POPULATE THE GENERAL SECTION OF THE UI
$general_section = new Form_Section('General');
$general_section
    ->addInput(new Form_Checkbox('enable', 'Enable', '', Config::to_internal_boolval($conf->enable)))
    ->setHelp('Enable SAML2 authentication for the pfSense webConfigurator.');

$general_section
    ->addInput(
        new Form_Checkbox(
            'strip_username',
            'Filter Email Usernames',
            '',
            Config::to_internal_boolval($conf->strip_username),
        ),
    )
    ->setHelp(
        "Enable removal of any characters after the @ character on email usernames. This is required if you intend to use SAML
    authentication that maps to an existing local user and your IdP returns email addresses as the NameID by default.",
    );

$general_section
    ->addInput(new Form_Checkbox('debug_mode', 'Debug Mode', '', Config::to_internal_boolval($conf->debug_mode)))
    ->setHelp(
        'Enable debug mode. This is intended for use when debugging issues with SAML2 logins. This will enable ' .
            'debug level logging as well as allow access to the <a href="/saml2_auth/sso/session/">debug session endpoint</a>.' .
            ' This option should not be enabled in production environments!',
    );

# POPULATE THE IDP SECTION OF THE UI
$idp_section = new Form_Section('Identity Provider Settings (IdP)');
$idp_section
    ->addInput(
        new Form_Input('idp_metadata_url', 'Identity Provider Metadata URL', 'text', $conf->idp_metadata_url, [
            'placeholder' => 'URL',
        ]),
    )
    ->setHelp(
        'Set the metadata URL of the upstream identity provider (if available). If set, the IdP configuration will be ' .
            'automatically fetched from your IdP and the fields below may be ignored. This will be provided by your IdP. ' .
            'Please note that any settings provided in the custom configuration field below may override these settings.',
    );

$idp_section
    ->addInput(
        new Form_Input('idp_entity_id', 'Identity Provider Entity ID', 'text', $conf->idp_entity_id, [
            'placeholder' => 'URL or alternate ID',
        ]),
    )
    ->setHelp('Set the entity ID of the upstream identity provider. This will be provided by your IdP.');

$idp_section
    ->addInput(
        new Form_Input('idp_sign_on_url', 'Identity Provider Sign-on URL', 'text', $conf->idp_sign_on_url, [
            'placeholder' => 'URL',
        ]),
    )
    ->setHelp('Set the sign-on URL of the upstream identity provider. This will be provided by your IdP.');

$idp_section
    ->addInput(
        new Form_Input(
            'idp_groups_attribute',
            'Identity Provider Groups Attribute',
            'text',
            $conf->idp_groups_attribute,
            ['placeholder' => 'Group attribute name'],
        ),
    )
    ->setHelp(
        'Set the groups attribute returned in the SAML assertion. This will be provided by your IdP if supported.',
    );

$idp_section
    ->addInput(new Form_Textarea('idp_x509_cert', 'Identity Provider x509 Certificate', $conf->idp_x509_cert))
    ->setHelp('Paste the x509 SAML2 certificate from the upstream identity provider.');

# POPULATE THE SP SECTION OF THE UI
$sp_section = new Form_Section('Service Provider Settings (SP)');
$sp_section
    ->addInput(
        new Form_Input('sp_base_url', 'Service Provider Base URL', 'text', $conf->sp_base_url, [
            'placeholder' => 'URL',
        ]),
    )
    ->setHelp(
        "Set the base URL of the service provider (pfSense). This must be the URL that is used to access pfSense's
    webConfigurator.",
    );

$sp_section
    ->addInput(new Form_StaticText('Service Provider Entity ID', "$conf->sp_base_url/saml2_auth/sso/metadata/"))
    ->setHelp("Displays the service provider's entity ID. This is the entity ID you will need to provide to your IdP.");

$sp_section
    ->addInput(new Form_StaticText('Service Provider Sign-on URL', "$conf->sp_base_url/saml2_auth/sso/acs/"))
    ->setHelp(
        "Displays the service provider's sign-on URL. This is the URL you will need to provide to your IdP. They may refer
    to this URL as the assertion consumer service (ACS).",
    );

# POPULATE THE ADVANCED SECTION OF THE UI
$advanced_section = new Form_Section('Advanced Settings');
$advanced_section
    ->addInput(new Form_Textarea('custom_conf', 'Custom SAML2 configuration', $conf->custom_conf))
    ->setHelp(
        'Adds custom configuration for SAML2 logins. This allows you to add custom php-saml settings in JSON format for the
    <a href="https://github.com/onelogin/php-saml" target="_blank">OneLogin PHP-SAML</a> library to use. This option is
    unsupported. Use at your own risk.',
    );

# POPULATE OUR COMPLETE FORM
$form->add($general_section);
$form->add($idp_section);
$form->add($sp_section);
$form->add($advanced_section);
$form->addGlobal(new Form_Button('save', 'Save', null, 'fa-solid fa-save'))->addClass('btn-primary');

# PRINT OUR FORM AND PFSENSE FOOTER
print $form;
include 'foot.inc';
