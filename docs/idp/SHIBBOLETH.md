# Shibboleth

This guide will walk you through configuring pfSense to use (Shibboleth IdP)[https://www.shibboleth.net] for webConfigurator authentication via SAML 2.0. 
This allows your users to log into pfSense using Shibboleth, centralizing access and features such as MFA where used.

## Prerequisites

- A pfSense instance with the [SAML2 package installed](../INSTALLATION.md)
- Access to the Shibboleth IdP and it's configuration files and folders in the IdP base installation directory usually `/opt/shibboleth-idp` or `c:\opt\shibboleth-idp`
    - `conf/relying-party.xml`
    - `conf/attribute-filter.xml`
    - `conf/saml-nameid.xml`
    - `conf/metadata-providers.xml`
    - `metadata/`
- Access to be able to restart the IdPs web container or use Reloadable services
- If you are unfamiliar with XML then please use an XML linter or even a web browser to check you have valid XML.

## Step 1: XML metadata file

Create an XML metadata file e.g. `metadata/pfsense-metadata.xml`

Replace `pfsense.example.org` with the IP or FQDN of your IdP.

```
<?xml version="1.0" encoding="UTF-8"?>
<EntityDescriptor
	xmlns="urn:oasis:names:tc:SAML:2.0:metadata"
	xmlns:ds="http://www.w3.org/2000/09/xmldsig#"
	xmlns:shibmd="urn:mace:shibboleth:metadata:1.0"
	xmlns:xml="http://www.w3.org/XML/1998/namespace"
	xmlns:mdui="urn:oasis:names:tc:SAML:metadata:ui"
	entityID="https://pfsense.example.org/saml2_auth/sso/metadata/">
    <SPSSODescriptor
	    protocolSupportEnumeration="urn:oasis:names:tc:SAML:2.0:protocol">
	    <AssertionConsumerService
                Binding="urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST"
		Location="https://pfsense.example.org/saml2_auth/sso/acs/ "
		index="1"/>
    </SPSSODescriptor>
</EntityDescriptor>
```
## Step 2: Update Metadata Providers

Configure the IdP to read the metadata, add the following to `metadata-providers.xml`, note the `id` must be unique

```
<MetadataProvider id="pfSenseMetadata"  xsi:type="FilesystemMetadataProvider" metadataFile="%{idp.home}/metadata/pfsense-metadata.xml"/>
```
## Step 3: Update Relying Party

Configure `relying-party.xml`, this is to override the defualt configurations for Shibboleth.

These would be `encryptAssertions` and use of the `transient` NameID, it appears that the pfSense SAML2 package doesn't support encrypted assertions out of the box, and relies on the identifier/attribute for the user being in the NameID.  Other NameID formats could be used?

```
<!-- pfSense -->
<bean parent="RelyingPartyByName" c:relyingPartyIds="https://pfsense.example.org/saml2_auth/sso/metadata/">
    <property name="profileConfigurations">
        <list>
            <bean parent="SAML2.SSO" p:nameIDFormatPrecedence="urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified" p:signAssertions="true" p:encryptAssertions="false" />
        </list>
    </property>
</bean>
```

## Step 4 - Update SAML NameID

Configure `saml-nameid.xml`, this goes hand-in-hand with the above, it maps a source attribute to the NameID format you've chosen to use, and conditions this for only this relying party, this should not affect other services using the IdP.

```
<bean parent="shibboleth.SAML2AttributeSourcedGenerator"
    p:omitQualifiers="true"
    p:format="urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified"
    p:attributeSourceIds="#{ {'uid'} }">
    <property name="activationCondition">
       <bean parent="shibboleth.Conditions.RelyingPartyId" c:candidates="#{{'https://pfsense.example.org/saml2_auth/sso/metadata/'}}" />
    </property>
</bean>
```
## Step 5 - Update Attribute Filter

You'll need an appropriate AttributeFilterPolicy in `attribute-filter.xml` to release your chosen attribute, that's also mentioned above.  The Requester is the EntityID of the SP.

```
  <AttributeFilterPolicy id="uid">
        <PolicyRequirementRule xsi:type="OR">
                <Rule xsi:type="Requester" value="https://pfsense.example.org/saml2_auth/sso/metadata/" />
        </PolicyRequirementRule>
        <AttributeRule attributeID="uid">
            <PermitValueRule xsi:type="ANY" />
        </AttributeRule>
  </AttributeFilterPolicy>
```
## Step 6 - Update Attribute Resolver

This step is optional, it depends if `uid` or any other attribute you might use is configured in the resolver.  This is the part that will really depend on your configuration and DataConnectors in the Shibboleth IdP.  You could use "Filter Email Usernames" for any attribute that uses a `username@scope` format e.g. `eduPersonPrincipalName`, `subject-id` or `mail` (!).  You could also likely use attributes like `sAMAccountName`, if you have that instead of my `uid` example.

This an example AttributeDefinition

```
    <AttributeDefinition id="uid" xsi:type="Simple">
        <InputDataConnector ref="myLDAP" attributeNames="uid" />
    </AttributeDefinition>
```


## Step 7 - Reload/Restart

Use (Reloadable Services)[https://shibboleth.atlassian.net/wiki/spaces/IDP5/pages/3199507931/ReloadableServices] to restart the IdPs services, given the number of files changed, then there are a few;

```
./reload-service.sh -id shibboleth.RelyingPartyResolverService
./reload-service.sh -id shibboleth.MetadataResolverService
./reload-service.sh -id shibboleth.AttributeResolverService
./reload-service.sh -id shibboleth.NameIdentifierGenerationService
./reload-service.sh -id shibboleth.AttributeFilterService
```

Alternatively, restart the web container e.g. Jetty/tomcat.

You should monitor `idp-process.log` following e.g. `tail -f idp-process.log`

## Step 8: Test the configuration

On the pfSense login page, there should now be a **Sign In with SSO** link below the standard login form. Click this 
link to be redirected to Okta for authentication. Assuming everything is configured correctly, you should be redirected
back to pfSense and be logged in. If you encounter issues, you can enable the **Debug** option on the **System > SAML2** 
page to enable more detailed logging. The SAML2 logs can be found under **Status > System Logs > Packages > SAML2**.

## Future areas to document and test..

1. Groups are not covered in the above, but that would just be another attribute released with an Attribute Filter and Resolver configuration.  I've assumed that access has been created based on my matching an existing user to `uid`

2. SAML encryption testing, that would required 'Custom SAML2 configuration' in the plugin

3. Testing with a SAML attribute not just NameID.

<sub>Last updated: 2026-07-11</sub>

