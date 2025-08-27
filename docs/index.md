# pfSense SAML2 Package

The pfSense SAML2 package is an unofficial, open-source SAML2 implementation for pfSense CE. It provides a simple and
secure way to integrate Single Sign-On (SSO) capabilities into your pfSense instances. This package aims to provide
enterprise-level authentication for pfSense CE admins.

## Key Features

- Supports most SAML2 Identity Providers (IdPs)
- Integrates seamlessly with pfSense CE's authentication system
- Allows privilege inheritance from either pfSense user groups or pfSense local users
- Works with either Identity Provider or Service Provider initiated SSO

!!! Important
    SSO is only available for the pfSense webConfigurator. It does not support SSO for other authentication areas
    such as SSH, VPN, or other services.

## Source Code & Contributions

The source code for this project is available in its entirety on [GitHub](https://github.com/pfrest/pfSense-pkg-saml2-auth)
and is licensed under an [Apache 2.0 license](https://github.com/pfrest/pfSense-pkg-saml2-auth/blob/master/LICENSE).
Contributions are welcome and encouraged.

### Maintainers

- <a href="https://github.com/jaredhendrickson13"><img src="https://github.com/jaredhendrickson13.png" alt="Jared Hendrickson" title="Jared Hendrickson" width="30" height="30"/> Jared Hendrickson</img></a> - github@jaredhendrickson.com

!!! Important
    Unless your inquiry is regarding a [security vulnerability](SECURITY.md) or other sensitive matter, please do not
    contact the maintainers directly. Instead, please [open an issue](https://github.com/pfrest/pfSense-pkg-saml2-auth/issues/new/choose) to report a bug or request a feature. For
    general questions or help requests, please [open a discussion](https://github.com/pfrest/pfSense-pkg-saml2-auth/discussions/new/choose).

## Disclaimers

!!! Caution
    - This package is not affiliated or supported by Netgate or the pfSense team; it is developed and maintained
    by the community.
    - This package is not supported on pfSense Plus. Installing on pfSense Plus may result in unexpected behavior.