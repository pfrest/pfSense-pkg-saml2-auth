# DEFAULT FILE PATCHES

pfSense-pkg-saml2-auth works by patching existing files on the pfSense system to add support for SAML2. In most cases,
these patches will apply cleanly to any version of pfSense. The `default` directory contains the patches that will be
applied to most pfSense installations.

In event that there is a directory within the `patches/` directory that matches the version of pfSense you are running,
the patches in that directory will be applied instead of the ones in the `default` directory. This allows for
version-specific changes to be made without affecting the majority of installations. This also allows patches for
older versions of pfSense to be maintained without affecting newer versions.
