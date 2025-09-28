<?php

namespace Saml2\Tests;

require_once 'Saml2/autoload.php';

use Saml2\Core\TestCase;
use Saml2\Errors\SystemError;

/**
 * A test case to validate the Saml2 Errors\SystemError class.
 */
class Saml2ErrorsSystemErrorTestCase extends TestCase {
    /**
     * Test that the SystemError class can be thrown
     */
    public function test_throw_config_error(): void {
        $this->assert_throws(
            exceptions: [SystemError::class],
            callable: function () {
                throw new SystemError('Test system error');
            },
        );
    }
}
