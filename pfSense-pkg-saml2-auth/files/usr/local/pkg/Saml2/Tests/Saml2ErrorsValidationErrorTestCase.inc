<?php

namespace Saml2\Tests;

require_once 'Saml2/autoload.php';

use Saml2\Core\TestCase;
use Saml2\Errors\ValidationError;

/**
 * A test case to validate the Saml2 Errors\ValidationError class.
 */
class Saml2ErrorsValidationErrorTestCase extends TestCase {
    /**
     * Test that the ValidationError class can be thrown
     */
    public function test_throw_validation_error(): void {
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () {
                throw new ValidationError('Test validation error');
            },
        );
    }
}
