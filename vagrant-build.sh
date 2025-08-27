#!/bin/sh

# Set build variables
FREEBSD_VERSION=${FREEBSD_VERSION:-"freebsd/FreeBSD-14.1-STABLE"}
BUILD_VERSION=${BUILD_VERSION:-"0.0_0-dev"}

# Start the vagrant box
FREEBSD_VERSION=${FREEBSD_VERSION} vagrant up
vagrant ssh -c "rm -rf /home/vagrant/build"

# Obtain the SSH config for the vagrant box
SSH_CONFIG_FILE=$(mktemp)
vagrant ssh-config > "$SSH_CONFIG_FILE"

# Copy the source code to the vagrant box using SCP (vagrant upload skips hidden files)
rsync -avz --progress -e "ssh -F $SSH_CONFIG_FILE" ../pfSense-pkg-saml2-auth vagrant@default:/home/vagrant/build/ --exclude node_modules --exclude .git --exclude .phpdoc --exclude ./vendor --exclude .vagrant

# Run the build script on the vagrant box
cat << END | vagrant ssh
composer install --working-dir /home/vagrant/build/pfSense-pkg-saml2-auth
cp -r /home/vagrant/build/pfSense-pkg-saml2-auth/vendor/* /home/vagrant/build/pfSense-pkg-saml2-auth/pfSense-pkg-saml2-auth/files/usr/local/pkg/Saml2/Vendor
python3.11 /home/vagrant/build/pfSense-pkg-saml2-auth/tools/make_package.py -t $BUILD_VERSION
END

# Copy the built package back to the host using SCP
scp -F $SSH_CONFIG_FILE vagrant@default:/home/vagrant/build/pfSense-pkg-saml2-auth/pfSense-pkg-saml2-auth/work/pkg/pfSense-pkg-saml2-auth-$BUILD_VERSION.pkg .
rm $SSH_CONFIG_FILE