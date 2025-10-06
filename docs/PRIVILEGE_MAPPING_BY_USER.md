# By User

Mapping privileges by local user account allows existing pfSense users to be assigned their existing permissions 
when logging in via SSO. This is useful if you want to manage user accounts and permissions entirely on pfSense,
but want to support SSO to authenticate those users. This method requires that a local user account already 
exists before the user can log in via SSO.

!!! Note
    If an SSO user has a local account on pfSense and group mapping is used, pfSense will always use the 
    privileges inherited from the groups and fallback to the local user account only if no groups are provided by the
    IdP.
