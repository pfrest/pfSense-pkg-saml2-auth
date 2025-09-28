<?php

namespace Saml2\Tests;

require_once("Saml2/autoload.php");

use Saml2\Core\TestCase;
use Saml2\Errors\UpdateError;


/**
 * A test case to validate the Saml2 Errors\UpdateError class.
 */
class Saml2ErrorsUpdateErrorTestCase extends TestCase
{
    /**
     * Test that the UpdateError class can be thrown
     */
    public function test_throw_config_error(): void
    {
        $this->assert_throws(
            exceptions: [UpdateError::class],
            callable: function() {
                throw new UpdateError("Test update error");
            }
        );
    }
}