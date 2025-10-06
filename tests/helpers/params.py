import os
import requests


class Params:
    """
    A class to hold parameters used to describe the pfSense and IdP test environment.

    Attributes:
        pfsense_host (str): The hostname or IP address of the pfSense instance to test against.
        pfsense_username (str): The username for authentication with the pfSense instance.
        pfsense_password (str): The password for authentication with the pfSense instance.
        pfsense_port (int): The port number for the pfSense instance.
        pfsense_scheme (str): The URL scheme (http or https) for the pfSense instance.
        pfsense_url (str): The complete base URL for the pfSense instance.
        pfsense_restapi_pkg_url (str): The URL to the pfSense REST API package download.
        idp_host (str): The hostname or IP address of the IdP instance to test against.
        idp_port (int): The port number for the IdP instance.
        idp_scheme (str): The URL scheme (http or https) for the IdP instance.
        idp_url (str): The complete base URL for the IdP instance.
        idp_metadata_url (str): The complete URL to the IdP metadata endpoint.
        idp_entity_id (str): The entity ID for the IdP.
        idp_sign_on_url (str): The sign-on URL for the IdP.
        idp_groups_attribute (str): The attribute name for user groups in the IdP.
        idp_x509_cert (str): The x509 certificate data for the IdP.
        idp_expected_nameid (str): The expected NameID value in SAML assertions from the IdP.
        idp_expected_group (list[str]): A list of expected user groups in SAML assertions from the IdP.

    """
    def __init__(self) -> None:
        self.pfsense_host = os.environ["PFSENSE_PKG_SAML2_AUTH_PFSENSE_HOST"]
        self.pfsense_username = os.environ.get("PFSENSE_PKG_SAML2_AUTH_PFSENSE_USERNAME", "admin")
        self.pfsense_password = os.environ.get("PFSENSE_PKG_SAML2_AUTH_PFSENSE_PASSWORD", "pfsense")
        self.pfsense_port = os.environ.get("PFSENSE_PKG_SAML2_AUTH_PFSENSE_PORT", 443)
        self.pfsense_scheme = os.environ.get("PFSENSE_PKG_SAML2_AUTH_PFSENSE_SCHEME", "https")
        self.pfsense_url = f"{self.pfsense_scheme}://{self.pfsense_host}:{self.pfsense_port}"
        self.pfsense_restapi_pkg_url = os.environ.get("PFSENSE_PKG_SAML2_AUTH_PFSENSE_RESTAPI_PKG_URL")
        self.idp_host = os.environ["PFSENSE_PKG_SAML2_AUTH_IDP_HOST"]
        self.idp_port = os.environ.get("PFSENSE_PKG_SAML2_AUTH_IDP_PORT", 8443)
        self.idp_scheme = os.environ.get("PFSENSE_PKG_SAML2_AUTH_IDP_SCHEME", "https")
        self.idp_url = f"{self.idp_scheme}://{self.idp_host}:{self.idp_port}"
        self.idp_metadata_url = f"{self.idp_url}{os.environ['PFSENSE_PKG_SAML2_AUTH_IDP_METADATA_URL']}"
        self.idp_entity_id = os.environ["PFSENSE_PKG_SAML2_AUTH_IDP_ENTITY_ID"]
        self.idp_sign_on_url = os.environ["PFSENSE_PKG_SAML2_AUTH_IDP_SIGN_ON_URL"]
        self.idp_groups_attribute = os.environ.get("PFSENSE_PKG_SAML2_AUTH_IDP_GROUPS_ATTRIBUTE")
        self.idp_x509_cert = self.fetch_cert_from_idp()
        self.idp_expected_nameid = os.environ.get("PFSENSE_PKG_SAML2_AUTH_IDP_EXPECTED_NAMEID")
        self.idp_expected_group = os.environ.get("PFSENSE_PKG_SAML2_AUTH_IDP_EXPECTED_GROUP", "")

    def fetch_cert_from_idp(self) -> str:
        """
        Obtains the SAML2 certificate from the IdP. This is necessary as the certificate is unique per deployment
        of mock-saml2-idp.

        Returns:
            str: The x509 certificate data from the IdP.
        """
        resp = requests.get(f"{self.idp_url}/api/settings.php", verify=False)
        resp.raise_for_status()
        return resp.json()["idp_cert"]
