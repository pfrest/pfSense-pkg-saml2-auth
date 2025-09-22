# Frequently Asked Questions (FAQs)

## How does this package work?

This package is designed for seamless integration with pfSense. When installed, it applies specific patches to the 
pfSense core to add SAML2 support directly into the native authentication process. This approach ensures that all
existing functionality remains completely unaffected, and is easily removable by simply uninstalling the package. The
package then uses OneLogin's excellent [SAML2 PHP toolkit](https://github.com/SAML-Toolkits/php-saml) to handle the 
SAML2 protocol interactions. This ensures a robust and secure implementation of SAML2 authentication within pfSense.

## Wny isn't pfSense Plus officially supported?

Official support for pfSense Plus isn't provided because it's a closed-source, commercial product, while this SAML2 
package is fundamentally built off the open-source pfSense Community Edition (CE). The package works by applying patch 
files derived from the CE codebase, and creating official patches for pfSense Plus could require partial exposure of
proprietary source code; which could be subject to significant legal issues. Because the pfSense Plus codebase may 
diverge from its open-source counterpart over time, functionality is not guaranteed. Therefore, any use of this package 
on a pfSense Plus installation is unofficial, unsupported, and undertaken entirely at your own risk.