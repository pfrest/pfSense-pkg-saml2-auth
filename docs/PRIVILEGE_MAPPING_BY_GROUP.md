# Group-based Privilege Mapping

Mapping privileges by IdP group membership allows your IdP to specify which groups the SSO user belongs to. When
the user logs in, the user will automatically inherit the permissions assigned to matching user groups configured on 
pfSense. This is the preferred method of privilege mapping as it requires no existing local pfSense user accounts and
can greatly simplify user management and onboarding/offboarding. Your IdP must support returning group memberships
in the SAML assertion for this method to work.

!!! Important
    In the event that the IdP does not provide any group memberships for a the user **and** the user has a local pfSense
    user, pfSense will automatically fall back to inheriting privileges from the user's local account if one exists. 
    This can lead to discrepancies in user permissions if the local account has different privileges than those 
    intended by your IdP.

## Pre-requisites

Before configuring privilege mapping by group, ensure that you have already done the following:

1. Created and configured the desired groups in your IdP
2. Added users to those groups in your IdP
3. Configured your IdP to include group memberships in the SAML assertion

## Configuration

1. Navigate to **System > User Manager > Group**. Create a new remote-scoped group with a name that matches a corresponding
   group name in your IdP exactly. Save this group
2. Edit the group you just created and assign the desired privileges to this group. Save the group again.
3. Navigate to **System > SAML2 > Settings** and set the **IdP Groups Attribute** field to the name of the attribute
   in the SAML assertion that contains the user's group memberships. This is often `groups`, `memberOf`, or similar, but
   can vary across different IdPs. Save the settings.