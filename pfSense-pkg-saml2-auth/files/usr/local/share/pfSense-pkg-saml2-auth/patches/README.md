# pfSense-pkg-saml2-auth Patches

This directory contains the patch files necessary to integrate SAML2 authentication into the base pfSense authentication
system. These patches generally only need to change if there's been a significant change in the pfSense codebase that
affects authentication, or if there's a need to modify the SAML2 authentication behavior.

## Default Patches

The patches included in the `default/` directory are the standard patches that should be applied to a typical pfSense
instance. The default patches are always used for the latest stable release of pfSense.

## Patch Overrides

This package's default patches are designed for the latest version of pfSense, but major updates can introduce 
"breaking changes" that make them incompatible with older releases. To ensure backward compatibility, the patches/ 
directory may contain override folders for specific older versions (e.g., 2.7.2-RELEASE) that require different fixes. 
When you install or update, the script automatically checks your pfSense version; if you are on an older release with a
dedicated patch folder, it will apply those specific fixes. Otherwise, it defaults to using the main patches intended 
for the current version.

!!! Important
    The name of the override folder must match the contents of `/etc/version` on pfSense exactly to be applied correctly.