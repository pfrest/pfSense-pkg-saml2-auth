# Development Guide

Contributions to this project are welcome! To ensure a smooth, consistent, and efficient development process, please 
follow the guidelines and instructions detailed in this guide. Contributions include but are not limited to:

- Bug reports
- Feature requests
- Code contributions
- Documentation improvements

## Opening Issues

Before you open an issue, please check the [existing issues](https://github.com/pfrest/pfSense-pkg-saml2-auth/issues) 
to see if your concern has already been addressed. If not, you may open an issue 
[here](https://github.com/pfrest/pfSense-pkg-saml2-auth/issues/new/choose). Issues should only
be opened for bugs or feature requests. For questions or general troubleshooting please open a 
[discussion](https://github.com/pfrest/pfSense-pkg-saml2-auth/discussions/new/choose) instead.

If you are trying to report a security vulnerability, please do not open a public issue. Instead, follow the 
instructions [here](SECURITY.md).

!!! Important
    Please **do not open issues** requesting support or troubleshooting for specific Identity Providers. This package 
    is intentionally designed to be IdP-agnostic, acting as a wrapper around the powerful **OneLogin PHP-SAML toolkit**.
    To ensure maximum flexibility, the package includes a Custom Configuration option that allows administrators to 
    define any setting available in the underlying `php-saml` library. If you are struggling with an integration, first
    leverage this custom configuration. If you discover a genuine compatibility problem that cannot be resolved through
    configuration, the issue most likely lies with the upstream toolkit. In that case, please report the issue directly 
    to the [OneLogin PHP-SAML repository](https://github.com/SAML-Toolkits/php-saml) rather than to this project so it 
    can be addressed upstream. Once the upstream issue is resolved, it will make its way back into this package through 
    regular updates. These types of issues are welcome as [discussions](https://github.com/pfrest/pfSense-pkg-saml2-auth/discussions/new/choose).
    however.

## Submitting Pull Requests

If you would like to contribute code to the project, please follow these steps:

1. Fork the repository on GitHub.
2. Clone your fork to your local machine.
3. Make your changes in a new branch.
4. Ensure your code adheres to the project's coding standards and passes all tests.
5. Commit your changes with clear and descriptive commit messages.
6. Push your changes to your fork on GitHub.
7. Open a pull request against the `master` branch of the original repository.

When submitting a pull request, please include a detailed description of your changes and the problem they solve. If
your pull request addresses an existing issue, please reference the issue number in your description.

## Style Guidelines

The project uses automated tools to enforce code style and quality. You can easily run the same tools to check your
code before submitting a pull request.

### PHP

In general, the project adheres to the [PSR-12](https://www.php-fig.org/psr/psr-12/) coding standard. We use 
Prettier's PHP plugin to automatically format PHP code. To check or reformat your code, run:

```bash
npm install
npx prettier --check .
npx prettier --write .
```

!!! Note
    These commands assume you have npm already installed and that you are running them from the root of the repository
    checked out on your local machine.

### Python
    
Python is only used for development tools and testing. The project adheres to the [PEP 8](https://peps.python.org/pep-0008/)
coding standard. We use `black` to automatically format Python code. To check or reformat your code, run:

```bash
python3 -m pip install -r requirements-dev.txt
python3 -m black --check .
python3 -m black .
```

!!! Notes
    - While not always necessary, it is recommended to use a virtual environment for Python development. You can create
    one using `python3 -m venv venv` and activate it with `source venv/bin/activate` before running these commands.
    - These commands assume you have Python 3 and pip already installed and that you are running them from the root of 
    the repository checked out on your local machine.

## Building the Package

Building this package requires access to a suitable FreeBSD build environment. The package must be built using the FreeBSD
version that corresponds to the pfSense version you are building for. Luckily, there are several tools available to help
you build the package:

### Step 1: Install Virtualbox and Vagrant

On your development machine, install [Virtualbox](https://www.virtualbox.org) and [Vagrant](https://www.vagrantup.com).
These tools will allow you to create a FreeBSD virtual machine for building the package with minimal effort.

### Step 2: Clone the Repository

Clone the repository to your development machine:

```bash
git clone git@github.com:pfrest/pfSense-pkg-saml2-auth.git
```

### Step 3: Run the vagrant-build.sh script

From the project root, run the `vagrant-build.sh` script to create a FreeBSD virtual machine and build the package:

```bash
sh vagrant-build.sh
```

After the script completes, the package will be built and the resulting `.pkg` file will be located in your current
working directory.

!!! Important
    - You will need to copy this `.pkg` file to your pfSense instance and install it using `pkg`. The package will not operate correctly if installed on a non-pfSense FreeBSD system.
    - The script will not automatically destroy the virtual machine after the build process completes. You will need to manually destroy the virtual machine using `vagrant destroy` when you are finished.

### Supported Build Environment Variables

Below are the supported environment variables to customize the build process:

- `FREEBSD_VERSION`: The version of FreeBSD to use for the build. This must be an existing Vagrant box name including the `freebsd/` prefix. The default value is `freebsd/FreeBSD-14.0-CURRENT`. For a list of available boxes, see the [Vagrant Cloud](https://app.vagrantup.com/freebsd) page.
- `BUILD_VERSION`: The version tag to give the build, this must be in a FreeBSD package version format (e.g. `0.0_0`). The default value is `0.0_0-dev`.

!!! Note
    Some of the official FreeBSD Vagrant boxes may not work correctly due to missing dependencies or other issues. If 
    you encounter problems, try using a different box version.

## Testing

Unfortunately, pfSense does not lend itself well to automated testing due to its architecture and the way packages
are integrated into the system. This means it's very difficult to get standard testing tools onto pfSense itself,
and missing pfSense-specific context makes it hard to run tests outside of pfSense accurately.

As a result, the testing ideology for this package is a hybrid approach that aims to test what really matters, not
blindly test for coverage. This includes basic unit tests to verify core functionality, as well as end-to-end tests
that fully check the SSO flows in a real pfSense environment using a mock Identity Provider.

### Unit Tests

Because getting standard testing tools onto pfSense is difficult, the package includes a very basic testing framework.
This framework works by creating subclasses of the `\Saml2\Core\TestCase` class and placing the class files in the package's
[Tests](https://github.com/pfrest/pfSense-pkg-saml2-auth/tree/master/pfSense-pkg-saml2-auth/files/usr/local/pkg/Saml2/Tests) 
directory. These tests check for basic assertions like equality, truthiness, and exception throwing. While not as 
powerful as more common testing frameworks, this framework is sufficient for basic unit tests and can be run directly 
on a pfSense system without any extra dependencies.

#### Writing Unit Tests

Unit tests are located in the [Tests](https://github.com/pfrest/pfSense-pkg-saml2-auth/tree/master/pfSense-pkg-saml2-auth/files/usr/local/pkg/Saml2/Tests) 
directory/namespace of the package. Below is a very simple example of a unit test:

```php
<?php

namespace Saml2\Tests;

require_once 'Saml2/autoload.php';

use Saml2\Core\TestCase;

/**
 * An example test case to demonstrate the testing framework
 */
class ExampleTestCase extends TestCase {
    /**
     * A simple test that always passes.
     */
    public function test_throw_config_error(): void {
        $this->assert_equals(1, 1);
    }
}
```

!!! Note
    You can also copy your tests onto an existing pfSense installation if you don't want to rebuild the package for
    testing. Just make sure to place them in the correct directory: `/usr/local/pkg/Saml2/Tests`.

!!! Important
    The `tests/` directory in the root of the repository does not contain unit tests. It contains end-to-end tests
    and integration tests that are intended to be run outside of pfSense.

#### Running Unit Tests

To run the unit tests, you must have the package installed on a pfSense system. A pfSense VM with a snapshots is
recommended for testing. Once you have pfSense running with the package installed, you can run the tests by executing
the following command from the command line:

```commandline
pfsense-saml2 runtests
```

If you want to only run tests whose class name contains a specific keyword, you can provide that keyword as an 
argument. For example:

```commandline
pfsense-saml2 runtests SomeExampleTestCase
```

This will only run tests in classes whose name contains `SomeExampleTestCase`.

### End-to-End Tests

End-to-end tests are located in the `tests/` directory in the root of the repository. These tests are written in
Python using the `pytest` framework and leverage the `playwright` library to automate browser interactions. The end-to-end
tests are designed to run against a real pfSense installation with the package installed, as well as a mock Identity.
To make setting up the test environment easier, the repository includes  a `docker-compose.yml` file that can
be used to spin up both the test suite and the mock IdP using Docker containers. Your pfSense installation must be
setup outside of Docker, however.

#### Writing End-to-End Tests

End-to-end tests are written in Python using the [pytest](https://docs.pytest.org/en/stable/) framework and the 
[playwright](https://playwright.dev/) library. Example of these tests can be found in the `tests/e2e` directory in the 
root of the repository. It is usually not necessary to write new end-to-end tests unless you are adding a new feature 
changes the SSO flow in a significant way. In that case, please add tests to cover the new functionality.

#### Running End-to-End Tests

As previously mentioned, the end-to-end tests use Docker containers to simplify the setup. You must have Docker and
Docker Compose installed on your machine to run the tests. You will also need a pfSense installation with your build
of the package already installed. The pfSense installation must be accessible from the machine running the tests.

You will need to set the following environment variables on the host running Docker to configure the tests:

- `SP_ENTITY_ID`: The Entity ID of the Service Provider (your pfSense installation). This is usually in the format `https://<pfsense-ip-or-domain>/saml2_auth/sso/metadata/`.
- `SP_ASSERTION_CONSUMER_SERVICE_URL`: The Assertion Consumer Service (ACS) URL of the Service Provider. This is usually in the format `https://<pfsense-ip-or-domain>/saml2_auth/sso/acs/`.
- `PFSENSE_PKG_SAML2_AUTH_PFSENSE_HOST`: The hostname or IP address of your pfSense installation.
- `PFSENSE_PKG_SAML2_AUTH_IDP_HOST`: The hostname or IP address of the host running Docker. This must be reachable from pfSense.

Once you have Docker and Docker Compose installed, you can
run the tests by executing the following command from the root of the repository:

```commandline
docker compose -f tests/docker-compose.yml up -d --build
docker compose -f tests/docker-compose.yml exec tests pytest --pyargs tests.e2e
docker compose -f tests/docker-compose.yml down --rmi all
```

!!! Notes
    - If you don't want to make these environment variables permanent, you can prefix the `docker compose` commands with 
    the variable assignments.
    - It is normal for these tests to take some time, especially when first pulling/building the Docker images.

!!! Danger
    Do not run the end-to-end tests against a production pfSense installation! Doing so may result in data loss or
    configuration corruption. Always use a test or development installation of pfSense for running the end-to-end tests.

## Documentation

There are two different sets of documentation for this project, each serves a different purpose and is managed by 
different tools:

### Package Documentation (MkDocs)

The primary documentation site for the project is built using [MkDocs](https://www.mkdocs.org) and is located at
https://pfSense-pkg-saml2-auth.pfrest.org. This documentation is intended for administrators of the package and covers
installation, configuration, and usage of the package. The MkDocs documentation is written in Markdown and is located
in the [`docs/`](https://github.com/pfrest/pfSense-pkg-saml2-auth/tree/master/docs) directory of the repository. The 
documentation is automatically built and deployed to GitHub Pages with each corresponding release of the package.

For contributors, you can build, serve and test the documentation locally using the following commands:

```bash
python3 -m pip install -r requirements-dev.txt
python3 -m mkdocs serve
```

!!! Note
    - It is recommended to use a virtual environment for Python development. You can create one using `python3 -m venv venv` 
    and activate it with `source venv/bin/activate` before running these commands.
    - These commands assume you have Python 3 and pip already installed and that you are running them from the root of 
    the repository checked out on your local machine.

### PHP Reference Documentation (phpDocumentor)

The [PHP reference documentation](https://pfrest.org/php-docs/) for this project is generated using 
[PHPDocumentor](https://www.phpdoc.org). The PHP reference provides detailed documentation for all PHP classes, functions,
script, etc. included in the package and is intended for those wishing to aide in development of the package. PHPDoc 
looks at the PHPDoc comments in the project's PHP files and generates a static HTML site with the documentation. 
The PHPDoc configuration file is located at `phpdoc.dist.xml`. There are two ways to generate the PHP documentation 
and start a local development server to view the documentation:

!!! Important
    This should be done on your local development machine, **not** on a pfSense instance.

#### Via Phar
```bash
# Download the phpdoc phar
wget https://phpdoc.org/phpDocumentor.phar
chmod +x phpDocumentor.phar

# Build the docs
./phpDocumentor.phar -c phpdoc.dist.xml

# Start a local development server
cd .phpdoc/build
php -S localhost:8000
```

#### Via Docker
```bash
# Build the docs using Docker
docker run --rm -v $(pwd):/data phpdoc/phpdoc run -c phpdoc.dist.xml

# Start a local development server
cd .phpdoc/build
php -S localhost:8000
```

!!! Note
    This method requires you to have Docker installed and running on your system. If you do not have Docker installed, you can
    use the Phar method to generate the documentation.


