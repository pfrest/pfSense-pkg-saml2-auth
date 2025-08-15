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

include_once("util.inc");
include_once("guiconfig.inc");
require_once("saml2_auth/SAML2Auth.inc");


# Initialize the pfSense UI page (note: $pgtitle must be defined before including head.inc)
$pgtitle = array(gettext('System'), gettext('SAML2'), gettext('Update'));
include('head.inc');
$update_tab = (SAML2Auth::is_update_available()) ? "Update (New Release Available)" : "Update";
$tab_array = [[gettext("Settings"), false, "/saml2_auth/"], [gettext($update_tab), true, "/saml2_auth/update/"]];
display_top_tabs($tab_array, true);    # Ensure the tabs are written to the top of page

# Variables
$form = new Form(false);
$pf_ver = SAML2Auth::get_pfsense_version(true);
$curr_ver = SAML2Auth::get_pkg_version();
$latest_ver = SAML2Auth::get_latest_pkg_version();
$latest_ver_date = date("Y-m-d", strtotime(SAML2Auth::get_latest_pkg_release_date()));
$all_vers = SAML2Auth::get_all_pkg_versions();
$curr_ver_msg = (SAML2Auth::is_update_available()) ? " - Update available" : " - Up-to-date";


# On POST, start the update process
if ($_POST["confirm"] and !empty($_POST["version"])) {
    # Start the update process in the background and print notice
    shell_exec("nohup pfsense-saml2 update ".escapeshellarg($_POST["version"])." > /dev/null &");
    print_apply_result_box(0, "\nSAML2 package update process has started and is running in the background. Check back in a few minutes.");
}

# Populate our update status form
$update_status_section = new Form_Section('Update Status');
$update_status_section->addInput(new Form_StaticText(
    'Support Status',
    (SAML2Auth::is_pkg_supported()) ? "<span style='color: green'>Verified</span>" : "<span style='color: red'>Unverified</span>"
))->setHelp(
    "Displays whether or not the package version currently installed fully supports pfSense ".$pf_ver."."
);
$update_status_section->addInput(new Form_StaticText('Current Version', $curr_ver.$curr_ver_msg));
$update_status_section->addInput(new Form_StaticText(
    'Latest Version',
    $latest_ver." - <a href='https://github.com/pfrest/pfSense-pkg-saml2-auth/releases/tag/v".$latest_ver."'>View Release</a>"." - Released on ".$latest_ver_date
));

# Populate our update settings form
$update_settings_section = new Form_Section('Update Settings');
$update_settings_section->addInput(new Form_Select(
    'version',
    'Select Version',
    $latest_ver,
    $all_vers
))->setHelp(
    "Select the version you'd like to update or rollback to. Only releases capable of installing on pfSense ".$pf_ver." 
    are shown. Use caution when reverting to a previous version of the package as this can remove some features and/or 
    introduce vulnerabilities that have since been patched in a later release."
);


# Display our populated form
$form->addGlobal(new Form_Button('confirm', 'Confirm', null, 'fa-check'))->addClass('btn btn-sm btn-success');
$form->add($update_status_section);
$form->add($update_settings_section);
print $form;

include('foot.inc');
