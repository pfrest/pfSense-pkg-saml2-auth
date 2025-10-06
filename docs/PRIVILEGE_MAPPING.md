# Privilege Mapping

pfSense privileges can be mapped to SSO users by either IdP group membership or by an existing local pfSense user 
account. Unfortunately, pfSense does not allow remote authentication servers to assign privileges by both group 
membership and local user account reliably. Therefore, it is recommended to use one method or the other. 

- [By Group](PRIVILEGE_MAPPING_BY_GROUP.md)
- [By User](PRIVILEGE_MAPPING_BY_USER.md)