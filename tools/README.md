Tools
=====
This directory includes scripts and files to aide the development of this project. Below are some basic overviews of 
the scripts included.

## MAKE_PACKAGE.PY
This script is designed to automatically generate our FreeBSD package Makefile and pkg-plist. This also attempts to
compile the package automatically if you run it on a FreeBSD system. Files are rendered using Jinja2 and templates are 
found in the `templates` subdirectory

### Usage
`python3 tools/make_package.py`

### Dependencies
- `Jinja2` package must be installed before running (`python3 -m pip install jinja2`)

### Output
Command will output the FreeBSD make command output. Outputs the following files:

- `pfSense-pkg-saml2-auth/pfSense-pkg-saml2-auth/Makefile` : The rendered Makefile
- `pfSense-pkg-saml2-auth/pfSense-pkg-saml2-auth/pkg-plist`: The rendered pkg-plist
- `pfSense-pkg-saml2-auth/pfSense-pkg-saml2-auth/pfSense-pkg-saml2-auth-<VERSION>.pkg` : The FreeBSD package distribution file. On FreeBSD 11, 
this should be located in the `pfSense-pkg-saml2-auth/pfSense-pkg-saml2-auth` directory after completion. On FreeBSD 12 it should be 
located in the `pfSense-pkg-saml2-auth/pfSense-pkg-saml2-auth/work/pkg` directory.

### Notes
- This script heavily depends on it's relative filepaths. You may execute the script from any directory, but do not move
the script to another directory.
