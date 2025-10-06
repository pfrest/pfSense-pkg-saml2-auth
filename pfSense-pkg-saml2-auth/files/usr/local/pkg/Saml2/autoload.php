<?php

# Include the autoload file for packages installed via Composer
require_once 'Saml2/Vendor/autoload.php';

# Include pfSense libraries
require_once 'config.inc';
require_once 'auth.inc';

# Include SAML2 package libraries
require_once 'Saml2/Core/Config.inc';
require_once 'Saml2/Core/Auth.inc';
require_once 'Saml2/Core/Update.inc';
require_once 'Saml2/Core/TestCase.inc';
require_once 'Saml2/Core/TestCaseRetry.inc';
require_once 'Saml2/Errors/ConfigError.inc';
require_once 'Saml2/Errors/SystemError.inc';
require_once 'Saml2/Errors/UpdateError.inc';
require_once 'Saml2/Errors/ValidationError.inc';

# Include all tests
foreach (glob('/usr/local/pkg/Saml2/Tests/*.inc') as $filename) {
    require_once $filename;
}
