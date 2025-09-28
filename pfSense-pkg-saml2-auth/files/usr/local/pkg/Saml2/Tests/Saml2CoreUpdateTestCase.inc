<?php

namespace Saml2\Tests;

require_once("Saml2/autoload.php");

use Saml2\Core\TestCase;
use function Saml2\Core\Update\fetch_pkg_releases;
use function Saml2\Core\Update\get_latest_pkg_release_date;
use function Saml2\Core\Update\get_latest_pkg_version;
use function Saml2\Core\Update\get_pfsense_base_version;
use function Saml2\Core\Update\get_pfsense_version;
use function Saml2\Core\Update\get_pkg_releases;
use function Saml2\Core\Update\get_pkg_version;
use function Saml2\Core\Update\get_supported_pkg_releases;
use const Saml2\Core\Update\PKG_STATIC_EXEC;
use const Saml2\Core\Update\RELEASES_CACHE_FILE;


/**
 * A test case to validate the Saml2 Core\Update class.
 */
class Saml2CoreUpdateTestCase extends TestCase
{
   /**
    * Ensures we can read the current pfSense version.
    */
   public function test_get_pfsense_version(): void {
        $version = get_pfsense_version();
        $this->assert_equals(
            $version,
            str_replace(PHP_EOL, "", file_get_contents("/etc/version"))
        );
   }

   /**
    * Ensures we can read the pfSense base version.
    */
    public function test_get_pfsense_base_version(): void
    {
        $version = get_pfsense_version();
        $parts = explode('.', $version);
        $expected_base_version = $parts[0] . '.' . $parts[1];
        $this->assert_equals(
            get_pfsense_base_version(),
            $expected_base_version
        );
    }
}