# Security Policy

Security is a top priority for this package and we are committed to addressing any vulnerabilities that may arise. 
This document outlines the process for reporting security vulnerabilities and how we handle them.

## Reporting a Vulnerability

Should you discover a vulnerability in the pfSense SAML2 authentication code, please report the issue in one of the following ways:

1. Submit a [private vulnerability report](https://github.com/pfrest/pfSense-pkg-saml2-auth/security/advisories/new).
2. Send an email to a package [maintainer](index.md#maintainers).
3. Open a pull request with a fix for the vulnerability.
4. As a last resort, you may open a public issue, but please be aware that this is not the preferred method for 
5. reporting security vulnerabilities.

When reporting a vulnerability, please include the following information:

- A detailed description of the vulnerability.
- Steps to reproduce the issue.
- Any relevant logs or screenshots.
- The version of the package you are using or a list of versions affected.
- Your contact information for follow-up questions.

Please note this is an independent and open-source project and no bug bounty or reward can be granted.

## Patching Upstream Vulnerabilities

At it's core, this package has very limited external dependencies. Most dependencies used in the project are not
bundled into the package itself, but are utilized for development, testing, and CI/CD purposes. Regardless, all
dependencies are monitored for vulnerabilities using GitHub's Dependabot service. When a vulnerability is identified
in a dependency, a pull request is automatically opened to update the affected package to a secure version. These
pull requests are reviewed and merged as quickly as possible to ensure the package remains secure. It is recommended
to update to the latest version of the package regularly to benefit from these security updates.