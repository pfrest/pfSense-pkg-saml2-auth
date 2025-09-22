<?php

# Include the autoload file for packages installed via Composer
require_once("Saml2/Vendor/autoload.php");

# Include pfSense libraries
require_once("config.inc");

# Include SAML2 package libraries
require_once("Saml2/Config.inc");
require_once("Saml2/Auth.inc");
require_once("Saml2/Update.inc");
require_once("Saml2/Errors/ConfigError.inc");
require_once("Saml2/Errors/SystemError.inc");
require_once("Saml2/Errors/UpdateError.inc");
require_once("Saml2/Errors/ValidationError.inc");
