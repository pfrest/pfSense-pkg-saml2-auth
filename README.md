# pfSense SAML2 Authentication

[![Quality](https://github.com/pfrest/pfSense-pkg-saml2-auth/actions/workflows/quality.yml/badge.svg)](https://github.com/pfrest/pfSense-pkg-saml2-auth/actions/workflows/quality.yml)
[![Build](https://github.com/pfrest/pfSense-pkg-saml2-auth/actions/workflows/build.yml/badge.svg)](https://github.com/pfrest/pfSense-pkg-saml2-auth/actions/workflows/build.yml)
[![Release](https://github.com/pfrest/pfSense-pkg-saml2-auth/actions/workflows/release.yml/badge.svg)](https://github.com/pfrest/pfSense-pkg-saml2-auth/actions/workflows/release.yml)

`pfSense-pkg-saml2-auth` implements SAML2 authentication for the pfSense webConfigurator, enabling seamless integration 
with modern identity providers (IdPs). This package integrates with pfSense's existing authentication system, allowing 
administrators to leverage single sign-on (SSO) and robust multi-factor authentication (MFA) from their preferred IdP.
This solution is ideal for meeting stringent security compliance requirements, such as PCI DSS, that mandate MFA for 
administrative logins. Key benefits include centralized user management, simplified administrator onboarding, and a 
significantly enhanced security posture for your firewall administration.

## Key Features

- Easily enables SSO authentication on pfSense without losing any existing authentication functionality.
- Supports both group-based and user-based privilege mapping.
- Auto-configuration available via IdP metadata URL.

## Getting Started

- [Installation](https://docs.pfrest.org/pfSense-pkg-saml2-auth/INSTALLATION/)
- [Configuration](https://docs.pfrest.org/pfSense-pkg-saml2-auth/CONFIGURATION/)
- [Privilege Mapping](https://docs.pfrest.org/pfSense-pkg-saml2-auth/PRIVILEGE_MAPPING/)
- [IdP Setup Guides](https://docs.pfrest.org/pfSense-pkg-saml2-auth/IDP_INSTRUCTIONS/)

## Quickstart

It's recommended to use the links in the [Getting Started](#getting-started) section for full installation and 
configuration instructions. For quick reference, the following commands can be used to install or uninstall the package 
via the pfSense command line.

```
pkg add https://github.com/pfrest/pfSense-pkg-saml2-auth/releases/latest/download/pfSense-2.8-pkg-saml2-auth.pkg
```

To uninstall:

```
pkg delete pfSense-pkg-saml2-auth
```

> [!NOTE]
> - When pfSense is updated, this package will be automatically uninstalled. After updating pfSense, the package will need to be reinstalled to match the updated version.
> - You may need to adjust the package URL above to match your pfSense version (e.g., `pfSense-2.7-pkg-saml2-auth.pkg` for pfSense 2.7.x). Check the [Releases](https://github.com/pfrest/pfSense-pkg-saml2-auth/releases) page for the correct package for your version.

## Limitations

- This package is only intended to add SAML2 authentication to the webConfigurator. SAML2 authentication is not made
  available for other pfSense services such as SSH, captive portal, OpenVPN, etc.

## Disclaimer

> [!CAUTION]
> This project is in no way affiliated with the pfSense project, or it's parent organization Netgate. Any use of the
> pfSense name is intended to relate the project to it's developed platform and in no way capitalizes on the
> pfSense trademark. By using this software, you acknowledge that no entity can provide support or guarantee
> functionality.
