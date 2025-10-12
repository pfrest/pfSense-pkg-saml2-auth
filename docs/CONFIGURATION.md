# Configuration

After installing the package, you will have access to the configuration page in the pfSense webConfigurator under
**System** > **SAML2** > **Settings**. Below are details on each configuration option available.

!!! Tip
    Check the [IdP Instructions](IDP_INSTRUCTIONS.md) page for specific instructions on configuring the package to work 
    with popular Identity Providers!

## Enable

**Internal name**: `enable`

**Description**: Checks this box to enable SAML2 authentication. If check and SAML2 is configured correctly, the login page will
show the 'Sign In with SSO' option below the standard login form. If unchecked, SAML2 authentication will be unavailable.

## Filter Email Usernames

**Internal name**: `strip_username`

**Description**: When enabled, this option will strip the domain part from email addresses used as usernames. For example, if your IdP
returns the NameID of the user as `john.doe@example.com`, after logging in pfSense will recognize the user as `john.doe`.
This is useful if your IdP only returns email addresses as the NameID. It's recommended to keep this option enabled as
pfSense does not officially support usernames with the `@` character.

!!! Note
    It may be necessary to disable this field if you have multiple SSO users with the same username, but different domains.

## Debug Mode

**Internal name**: `debug_mode`

**Description**: Enabling this option will turn on debug logging and tools to help troubleshoot issues with SAML2 authentication. When
enabled, additional, verbose logs will be written to the package log under **Status** > **System Logs** > **Packages** > **SAML2 **. 

Additionally, the /saml2_auth/sso/session/ endpoint will become available which will display the current SAML2 attributes
and session information for the logged-in user. This endpoint is useful for debugging and verifying that the attributes 
received from your IdP are correct and will provide the pages the user has access to.

!!! Warning
    Debug mode should only be enabled temporarily for troubleshooting purposes. Leaving debug mode enabled can expose
    sensitive information and may cause the package to log excessive information, potentially filling up disk space.

## Identity Provider Metadata URL

**Internal name**: `idp_metadata_url`

**Description**: If your IdP provides a URL to fetch the SAML2 metadata, you can enter it here. When this field is populated, the package
will automatically fetch the XML metadata from the URL to configure the connection to your IdP. This is the recommended
method to configure the IdP connection as it allows for automatic updates to the metadata.

If this field is left blank, you will need to manually enter IdP information in the following fields:

- [Identity Provider Entity ID](#identity-provider-entity-id)
- [Identity Provider Single Sign-On URL](#identity-provider-single-sign-on-url)
- [Identity Provider x509 Certificate](#identity-provider-x509-certificate)

!!! Important
    pfSense **must** has access to your IdP's metadata URL. If pfSense does not allow outbound connections to your IdP 
    metadata URL, the package will not be able to fetch the metadata and SSO will fail.

## Identity Provider Entity ID

**Internal name**: `idp_entity_id`

**Description**: The Entity ID is a unique identifier for your Identity Provider. This value is typically a URL or URN provided by your
IdP. If you are using the [Identity Provider Metadata URL](#identity-provider-metadata-url) field, this value can be left blank as it 
will be automatically fetched from your IdP.

## Identity Provider Single Sign-On URL

**Internal name**: `idp_sign_on_url`

**Description**: The Single Sign-On URL is the endpoint on your IdP where authentication requests are sent. This value must be a URL.
If you are using the [Identity Provider Metadata URL](#identity-provider-metadata-url) field, this value can be left blank as it will 
be automatically fetched from your IdP.

## Identity Provider x509 Certificate
**Internal name**: `idp_x509_cert`

**Description**: The x509 Certificate is used to verify the authenticity of the SAML2 responses sent by your IdP. This value must be
a PEM-encoded certificate. If you are using the [Identity Provider Metadata URL](#identity-provider-metadata-url) field, this value 
can be left blank as it will be automatically fetched from your IdP.

!!! Important
    The PEM data must include the `-----BEGIN CERTIFICATE-----` and `-----END CERTIFICATE-----` lines.

## Identity Provider Groups Attribute

**Internal name**: `idp_groups_attribute`

**Description**: The Groups Attribute is the name of the SAML2 attribute that contains the user's group membership information. This
value is case-sensitive and must match the attribute name exactly as provided by your IdP. If your IdP does not
provide group information, you can leave this field blank BUT you will only be able to use 
[user-based privilege mapping](PRIVILEGE_MAPPING_BY_USER.md).

!!! Important
    The groups attribute is required for [group-based privilege mapping](PRIVILEGE_MAPPING_BY_GROUP.md). If you plan
    to use group-based privilege mapping, this field must be populated with the correct attribute name.

## Service Provider Base URL

**Internal name**: `sp_base_url`

**Description**: The Service Provider Base URL is the base URL of your pfSense instance. This value is used to construct the
Service Provider Entity ID, and Service Provider Sign-on URL. This value must be a valid URL and should 
include the protocol (http or https) and the port if not using the default (80 for http, 443 for https).

!!! Notes
    - The package will automatically attempt to detect the correct base URL for your pfSense instance using the
    system's hostname, domain and webConfigurator settings. If the automatically detected URL is incorrect,
    you can manually enter the correct URL in this field.
    - If you are using a reverse proxy or accessing pfSense via a different URL than the system's hostname, you
    must manually enter the correct URL in this field.
    - Your IdP does **not** need network access to this URL. However, your users do need access to this URL to log in.

## Custom Configuration

**Internal name**: `custom_conf`

**Description**: While the other configuration fields should cover usage with most IdPs, there may be some advanced settings that 
are required for your specific IdP or use case. The Custom Configuration field allows you to enter additional
settings for the underlying [OneLogin SAML PHP toolkit](https://github.com/SAML-Toolkits/php-saml/blob/master/advanced_settings_example.php)
in a JSON format that will be merged in with final settings. This field is optional and can be left blank if not needed.

Another use case for this field is to manually override settings that are automatically fetched from your IdP's metadata URL.
In the event that your IdP's metadata is missing or incorrect information, you can use this field to manually set the correct values.

!!! Note
    In the event that a setting defined in this field conflicts with a setting defined in the other configuration fields,
    the value in this field will take precedence.

!!! Important
    The maintainers of this package do not provide support or troubleshooting for IdP specific settings issues. The package is written to 
    allow all settings in the underlying SAML toolkit to be configurable via this field. If there is a true compatibility issue with a
    specific IdP that cannot be resolved via this field, you should report the issue to upstream [OneLogin SAML PHP toolkit](https://github.com/SAML-Toolkits/php-saml)
    repository.