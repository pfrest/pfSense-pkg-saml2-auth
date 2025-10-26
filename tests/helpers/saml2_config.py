"""Helper class for managing SAML2 configuration in pfSense."""

import base64
import json

from tests.helpers.params import Params
from tests.helpers.pfsense_client import PfSenseClient


class Saml2Config:
    """Helper class for managing SAML2 configuration in pfSense."""

    # pylint: disable=too-many-instance-attributes
    enable: bool
    strip_username: bool
    debug_mode: bool
    idp_metadata_url: str
    idp_entity_id: str
    idp_sign_on_url: str
    idp_groups_attribute: str
    idp_x509_cert: str
    sp_base_url: str
    custom_conf: str

    def __init__(self, **kwargs) -> None:
        self.client = PfSenseClient()
        self.params = Params()
        self.enable = kwargs.get("enable", True)
        self.strip_username = kwargs.get("strip_username", True)
        self.debug_mode = kwargs.get("debug_mode", True)
        self.idp_metadata_url = kwargs.get(
            "idp_metadata_url", self.params.idp_metadata_url
        )
        self.idp_entity_id = kwargs.get("idp_entity_id", self.params.idp_entity_id)
        self.idp_sign_on_url = kwargs.get(
            "idp_sign_on_url", self.params.idp_sign_on_url
        )
        self.idp_groups_attribute = kwargs.get(
            "idp_groups_attribute", self.params.idp_groups_attribute
        )
        self.idp_x509_cert = kwargs.get("idp_x509_cert", self.params.idp_x509_cert)
        self.idp_verify_cert = kwargs.get("idp_verify_cert", False)
        self.sp_base_url = kwargs.get("sp_base_url", self.params.pfsense_url)
        self.custom_conf = kwargs.get("custom_conf", "")

    def save(self):
        """
        Saves the SAML2 configuration on pfSense by overwriting the backup file and restoring it.
        """
        config = self.to_pfsense_config()
        config_json = json.dumps(config)
        backup_file = "/var/cache/pfSense-pkg-saml2-auth/backup.json"
        cmd = f"echo '{config_json}' > {backup_file} && pfsense-saml2 restore"
        self.client.run_command(cmd)

    @staticmethod
    def to_pfsense_bool(value: bool) -> str:
        """Converts a boolean value to pfSense's expected string format."""
        return "yes" if value else ""

    def to_pfsense_config(self) -> dict:
        """Converts the SAML2 configuration to a dictionary format suitable for pfSense."""
        return {
            "enable": self.to_pfsense_bool(self.enable),
            "strip_username": self.to_pfsense_bool(self.strip_username),
            "debug_mode": self.to_pfsense_bool(self.debug_mode),
            "idp_metadata_url": self.idp_metadata_url,
            "idp_entity_id": self.idp_entity_id,
            "idp_sign_on_url": f"{self.params.idp_url}{self.idp_sign_on_url}",
            "idp_groups_attribute": self.idp_groups_attribute,
            "idp_x509_cert": base64.b64encode(self.idp_x509_cert.encode()).decode(),
            "sp_base_url": self.sp_base_url,
            "custom_conf": base64.b64encode(self.custom_conf.encode()).decode(),
        }
