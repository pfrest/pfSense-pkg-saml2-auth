<?php

namespace Saml2\Core;

use AssertionError;
use Error;
use Exception;
use ReflectionException;
use ReflectionMethod;

require_once 'Saml2/autoload.php';

/**
 * Defines a TestCase object that can be used to define child TestCase classes that test internal functionality of
 * the API. This is needed because pfSense does not have PHPUnit readily available.
 */
class TestCase {
    /**
     * @var string Placeholder for the method that is currently being tested.
     */
    public string $method = 'unknown';

    /**
     * @var string Placeholder for the docstring for the method that is currently being tested.
     */
    public string $method_docstring = '';

    /**
     * Runs this TestCase when the object is created. Any method that starts with `test` will automatically be run.
     * @throws ReflectionException When ReflectionMethod could not find a test method.
     * @throws Error|Exception The exception thrown by failed Tests, typically an AssertionError.
     */
    public function run(): void {
        # Need direct config access to revert any changes made by Tests
        global $config;

        # use a property to keep track of the method that is currently running
        $this->method = 'unknown';

        # Setup the test case before running tests
        $this->setup();

        # Loop through all methods in this object
        foreach (get_class_methods($this) as $method) {
            # If this method starts with `test`, run the function.
            if (str_starts_with($method, 'test')) {
                # Backup the current config so we can undo any changes made during the test
                $original_config = unserialize(serialize($config));

                # Set the current method undergoing testing
                $this->method = $method;
                $ref_method = new ReflectionMethod($this, $method);

                # Gather the description for this test method from its doc comment
                $this->method_docstring = $ref_method->getDocComment();
                $this->method_docstring = str_replace([PHP_EOL, '/**', '*/', '*', '  '], '', $this->method_docstring);

                # Get the retry settings for this method
                $retry_settings = $this->get_method_retry_settings($ref_method);

                # Always attempt once, then add retries if configured
                for ($attempt = 0; $attempt <= $retry_settings->retries; $attempt++) {
                    try {
                        # Try to run the test
                        $this->$method();
                        break;
                    } catch (Error | Exception $e) {
                        # Tear down any resources created by this test before retrying or exiting
                        $config = $original_config;
                        write_config("Restored config after API test '$method'");
                        $this->teardown();

                        # If we have retries left, wait the configured delay and try again
                        if ($attempt < $retry_settings->retries) {
                            sleep($retry_settings->delay);
                            continue;
                        }

                        # If we made it here, we have no retries left. Throw the exception to the caller.
                        throw $e;
                    }
                }

                # Restore the config as it was when the test began.
                $config = $original_config;
                write_config("Restored config after API test '$method'");
            }
        }

        # Teardown the test case now that all tests are completed
        $this->teardown();
    }

    /**
     * Sets up the test case before tests are run. This can be overridden by your TestCase to setup shared resources
     * required for your tests.
     */
    public function setup(): void {}

    /**
     * Tears down the test case after tests are run. This can be overridden by your TestCase to destroy resources
     * that may have been created during tests.
     */
    public function teardown(): void {}

    /**
     * @param ReflectionMethod $method The method to check for a TestCaseRetry attribute.
     * @return TestCaseRetry The TestCaseRetry attribute found on the method, or a default TestCaseRetry object
     */
    protected function get_method_retry_settings(ReflectionMethod $method): TestCaseRetry {
        # Use default retry settings if no attribute is found
        $retries = new TestCaseRetry();

        # If this method has a TestCaseRetry attribute, use it
        $attributes = $method->getAttributes(TestCaseRetry::class);
        if (count($attributes) > 0) {
            $retries = $attributes[0]->newInstance();
        }

        return $retries;
    }

    /**
     * Checks if two values are equal.
     * @param mixed $value1 The first value to evaluate.
     * @param mixed $value2 The second value to evaluate.
     * @param string $message Set the error message embedded in any AssertionError thrown by this method.
     * @throws AssertionError When $value1 and $value2 are not equal.
     */
    protected function assert_equals(mixed $value1, mixed $value2, string $message = ''): void {
        # Throw a TestError if the two values do not match
        if ($value1 !== $value2) {
            # Ensure the test failure message includes the values that did not equal. Serialize objects into strings.
            $value1_str = is_object($value1) ? serialize($value1) : $value1;
            $value2_str = is_object($value2) ? serialize($value2) : $value2;
            $message = $message ?: "Expected value `$value1_str` to equal `$value2_str`.";
            throw new AssertionError($message);
        }
    }

    /**
     * Checks if two values are not equal.
     * @param mixed $value1 The first value to evaluate.
     * @param mixed $value2 The second value to evaluate.
     * @param string $message Set the error message embedded in any AssertionError thrown by this method.
     * @throws AssertionError When $value1 and $value2 are equal.
     */
    protected function assert_not_equals(mixed $value1, mixed $value2, string $message = ''): void {
        # Throw a TestError if the two values match
        if ($value1 === $value2) {
            # Ensure the test failure message includes the values that did not equal. Serialize objects into strings.
            $value1_str = is_object($value1) ? serialize($value1) : $value1;
            $value2_str = is_object($value2) ? serialize($value2) : $value2;
            $message = $message ?: "Expected value `$value1_str` to not equal `$value2_str`.";
            throw new AssertionError($message);
        }
    }

    /**
     * Checks if an embedded callable raises a specific error
     * @param array $exceptions An array of Exceptions this method will expect the callable to raise.
     * @param mixed $callable An anonymous function that raises one of the $exceptions.
     * @param string $message Set the error message embedded in any AssertionError thrown by this method.
     * @throws AssertionError When the $callable did not raise one of the $exceptions.
     */
    protected function assert_throws(array $exceptions, mixed $callable, string $message = ''): void {
        # Variables
        $exceptions_str = implode(', ', $exceptions);
        $message = $message ?: "Expected callable to raise one of [$exceptions_str]";

        # Try to run the call able
        try {
            $callable();
        } catch (Exception | Error $exc) {
            # If an exception was thrown, make sure it was one of the expected exceptions.
            $class_name = $exc::class;
            if (!in_array($class_name, $exceptions)) {
                $message = $message . " Received '$class_name'.";
                throw new AssertionError($message);
            }
            return;
        }

        # If we ended up here, no exception was thrown. Throw an AssertionError
        throw new AssertionError($message);
    }

    /**
     * Checks if a given callable throws any exception.
     * @param mixed $callable A callable to run and check for thrown exception.
     * @throws AssertionError If the $callable throws any exception.
     */
    protected function assert_does_not_throw(mixed $callable): void {
        try {
            $callable();
        } catch (Error | Exception $exc) {
            $class_name = $exc::class;
            throw new AssertionError(
                "Expected callable to not throw an exception. Received $class_name: {$exc->getMessage()}",
            );
        }
    }

    /**
     * Checks if a given value is a `true` boolean.
     * @param mixed $value The value to evaluate.
     * @param string $message Set the error message embedded in any AssertionError thrown by this method.
     * @throws AssertionError When the value is not `true`.
     */
    protected function assert_is_true(mixed $value, string $message = "Expected value to equal 'true'."): void {
        # Check if this value is true
        if ($value !== true) {
            throw new AssertionError($message);
        }
    }

    /**
     * Checks if a given value is a `false` boolean.
     * @param mixed $value The value to evaluate.
     * @param string $message Set the error message embedded in any AssertionError thrown by this method.
     * @throws AssertionError When the value is not `false`.
     */
    protected function assert_is_false(mixed $value, string $message = "Expected value to equal 'false'.") {
        # Check if this value is false
        if ($value !== false) {
            throw new AssertionError($message);
        }
    }

    /**
     * Checks if a string contains another string.
     * @param string $haystack The string to check for the $needle in.
     * @param string $needle The string to check for within the $haystack.
     * @throws AssertionError When $haystack string did not contain the $needle string.
     */
    protected function assert_str_contains(string $haystack, string $needle): void {
        # Fail the test if $needle is not in $haystack
        if (!str_contains($haystack, $needle)) {
            throw new AssertionError("Expected string `$haystack` to contain string `$needle`");
        }
    }

    /**
     * Checks if a string does not contains another string.
     * @param string $haystack The string to check for the $needle in.
     * @param string $needle The string to check for within the $haystack.
     * @throws AssertionError When $haystack string did contain the $needle string.
     */
    protected function assert_str_does_not_contain(string $haystack, string $needle): void {
        # Fail the test if $needle is found in $haystack
        if (str_contains($haystack, $needle)) {
            throw new AssertionError("Expected string `$haystack` to not contain string `$needle`");
        }
    }

    /**
     * Checks if a given $value1 is greater than $value2.
     * @param mixed $value1 The first value to evaluate. This value must be greater than value 2 to pass check.
     * @param mixed $value2 The second value to evaluate. This value must be less than value 1 to pass check.
     */
    protected function assert_is_greater_than(mixed $value1, mixed $value2): void {
        # Fail test if $value2 is greater than $value1.
        if ($value1 <= $value2) {
            throw new AssertionError("Expected $value1 to be greater than $value2");
        }
    }

    /**
     * Checks if a given $value1 is greater than or equal to $value2.
     * @param mixed $value1 The first value to evaluate. This value must be greater than or equal to value 2 to pass check.
     * @param mixed $value2 The second value to evaluate. This value must be less than or equal to value 1 to pass check.
     */
    protected function assert_is_greater_than_or_equal(mixed $value1, mixed $value2): void {
        # Fail test if $value2 is greater than or equal to $value1.
        if ($value1 < $value2) {
            throw new AssertionError("Expected $value1 to be greater than or equal to $value2");
        }
    }

    /**
     * Checks if a given $value1 is less than $value2.
     * @param mixed $value1 The first value to evaluate. This value must be less than value 2 to pass check.
     * @param mixed $value2 The second value to evaluate. This value must be greater than value 1 to pass check.
     */
    protected function assert_is_less_than(mixed $value1, mixed $value2): void {
        # Fail test if $value2 is less than $value1.
        if ($value1 >= $value2) {
            throw new AssertionError("Expected $value1 to be less than $value2");
        }
    }

    /**
     * Checks if a given $value1 is less than or equal to $value2.
     * @param mixed $value1 The first value to evaluate. This value must be less than or equal to value 2 to pass check.
     * @param mixed $value2 The second value to evaluate. This value must be greater than or equal to value 1 to pass check.
     */
    protected function assert_is_less_than_or_equal(mixed $value1, mixed $value2): void {
        # Fail test if $value2 is less than or equal to $value1.
        if ($value1 > $value2) {
            throw new AssertionError("Expected $value1 to be less than or equal to $value2");
        }
    }

    /**
     * Checks if a given value is empty.
     * @param mixed $value The value to evaluate.
     * @param string $message Set the error message embedded in any AssertionError thrown by this method.
     * @throws AssertionError When the value is not empty.
     */
    protected function assert_is_empty(mixed $value, string $message = 'Expected value to be empty.'): void {
        # Check if this value is false
        if (!empty($value)) {
            throw new AssertionError($message);
        }
    }

    /**
     * Checks if a given value is not empty.
     * @param mixed $value The value to evaluate.
     * @param string $message Set the error message embedded in any AssertionError thrown by this method.
     * @throws AssertionError When the value is not empty.
     */
    protected function assert_is_not_empty(mixed $value, string $message = 'Expected value to not be empty.'): void {
        # Check if this value is false
        if (empty($value)) {
            throw new AssertionError($message);
        }
    }

    /**
     * Checks if a given value is of a given type.
     * @param mixed $value The value to evaluate.
     * @param string $type The string representation of the type to expect. This must match a gettype() return value.
     * @param string $message Set the error message embedded in any AssertionError thrown by this method.
     * @throws AssertionError When the $value's type does not match the given $type.
     */
    protected function assert_type(mixed $value, string $type, string $message = ''): void {
        # Variables
        $message = $message ?: "Expected value to be of type '$type'";

        # Throw an assertion error if the type specified does not match the type of our value
        if (gettype($value) !== $type) {
            throw new AssertionError($message);
        }
    }
}