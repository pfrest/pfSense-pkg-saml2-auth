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
?>
<head>
    <link rel="stylesheet" href="/css/saml2_auth/saml2_auth.css" type="text/css">
    <title>pfSense - SSO Authentication Failed</title>
</head>

<body>
    <div id="saml2_error_container">
        <h1 id="saml2_error_notice">SSO Authentication Failed</h1>
        <div class="saml2_circle_error_icon"></div>
        <p>The single sign-on attempt was unsuccessful due to a configuration error or invalid response. Administrators, please check the logs for more information under:<br><code>Status &gt; System Logs &gt; Packages &gt; SAML2</code></p>
        <a href="/" id="saml2_error_return_link">Return to Sign In</a>
    </div>
</body>
