<?php

namespace Saml2\Tests;

require_once("Saml2/autoload.php");

use Saml2\Core\TestCase;
use Saml2\Errors\ConfigError;


/**
 * A test case to validate the Saml2 Errors\ConfigError class.
 */
class Saml2ErrorsConfigErrorTestCase extends TestCase
{
    /**
     * Test that the ConfigError class can be thrown
     */
    public function test_throw_config_error(): void
    {
        $this->assert_throws(
            exceptions: [ConfigError::class],
            callable: function() {
                throw new ConfigError("Test config error");
            }
        );
    }
}