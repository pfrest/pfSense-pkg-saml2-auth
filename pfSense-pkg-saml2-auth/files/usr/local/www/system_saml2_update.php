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

use function Saml2\Core\Update\get_latest_pkg_release_date;
use function Saml2\Core\Update\get_latest_pkg_version;
use function Saml2\Core\Update\get_pfsense_version;
use function Saml2\Core\Update\get_pkg_version;
use function Saml2\Core\Update\get_supported_pkg_releases;
use function Saml2\Core\Update\is_update_available;

# Constants
const SAML2_UPDATE_LOG_FILE = '/tmp/saml2_update.log';

# Initialize the pfSense UI page (note: $pgtitle must be defined before including head.inc)
$pgtitle = [gettext('System'), gettext('SAML2'), gettext('Update')];
include 'head.inc';
$tab_array = [
    [gettext('Settings'), false, '/system_saml2_settings.php'],
    [gettext('Update'), true, '/system_saml2_update.php'],
];
display_top_tabs($tab_array, true); # Ensures the tabs are written to the top of page
# Variables
$form = new Form(false);
$pf_ver = get_pfsense_version();
$curr_ver = get_pkg_version();
$latest_ver = get_latest_pkg_version();
$latest_ver_date = get_latest_pkg_release_date();
$all_vers = array_keys(get_supported_pkg_releases());
$all_vers = array_combine($all_vers, $all_vers); # Make keys and values the same for Form_Select
$curr_ver_msg = is_update_available() ? ' - Update available' : ' - Up-to-date';

# On POST, start the update process
if ($_POST['confirm'] and !empty($_POST['version'])) {
    # Start the update process in the background and print notice
    shell_exec(
        command: 'nohup pfsense-saml2 revert ' .
            escapeshellarg($_POST['version']) .
            ' > ' .
            SAML2_UPDATE_LOG_FILE .
            ' &',
    );
    print_apply_result_box(
        0,
        'SAML2 package update process has started and is running in the background. Check back in a few minutes.',
    );
}

# Populate our update status form
$update_status_section = new Form_Section('Update Status');
$update_status_section->addInput(new Form_StaticText('Current Version', $curr_ver . $curr_ver_msg));
$update_status_section->addInput(
    new Form_StaticText(
        'Latest Version',
        "$latest_ver - <a href='https://github.com/pfrest/pfSense-pkg-saml2-auth/releases/tag/$latest_ver'>View Release</a> " .
            "- Released on $latest_ver_date",
    ),
);

# Populate our update settings form
$update_settings_section = new Form_Section('Update Settings');
$update_settings_section
    ->addInput(new Form_Select('version', 'Select Version', $latest_ver, $all_vers))
    ->setHelp(
        "Select the version you'd like to update or rollback to. Only releases capable of installing on pfSense $pf_ver " .
            'are shown. Use caution when reverting to a previous version of the package as this can remove some features ' .
            'and/or introduce vulnerabilities that have since been patched in a later release.',
    );

# Display our populated form
$form->addGlobal(new Form_Button('confirm', 'Confirm', null, 'fa-check'))->addClass('btn btn-sm btn-success');
$form->add($update_status_section);
$form->add($update_settings_section);
print $form;

include 'foot.inc';
