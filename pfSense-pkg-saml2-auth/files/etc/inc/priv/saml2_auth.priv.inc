<?php
//   Copyright 2025 Jared Hendrickson
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

global $priv_list;

# Define privilege for access the SAML2 settings page
$priv_list['page-system-saml2-settings'] = [];
$priv_list['page-system-saml2-settings']['name'] = "WebCfg - System: SAML2 Settings";
$priv_list['page-system-saml2-settings']['descr'] = "Allow access to SAML2 package's settings page.";
$priv_list['page-system-saml2-settings']['match'] = ["system_saml2_settings.php"];

# Define privilege for accessing the SAML2 update
$priv_list['page-system-saml2-update'] = [];
$priv_list['page-system-saml2-update']['name'] = "WebCfg - System: SAML2 Update";
$priv_list['page-system-saml2-update']['descr'] = "Allow access to SAML2 package's update page.";
$priv_list['page-system-saml2-update']['match'] = ["system_saml2_update.php"];
