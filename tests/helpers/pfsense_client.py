"""Module containing a helper client class to interact with pfSense instances"""
from tests.helpers.params import Params

import json
import time

import pfsense_vshell
import requests


class PfSenseClient:
    """
    Defines a helper client class to configure and interact with the target pfSense instance

    Attributes:
        host (str): The hostname or IP address of the pfSense instance
        username (str): The username for authentication
        password (str): The password for authentication
        port (int): The port number for the pfSense instance
        scheme (str): The URL scheme (http or https)
        restapi_pkg_url (str): The URL to the pfSense REST API package
        url (str): The base URL for the pfSense instance
        client (pfsense_vshell.PFClient): An instance of PFClient for executing commands on pfSense
    """

    def __init__(self) -> None:
        """Initializes the PfSenseClient with parameters from environment variables"""
        # Load test parameters from environment variables
        params = Params()

        # Initialize attributes
        self.host = params.pfsense_host
        self.username = params.pfsense_username
        self.password = params.pfsense_password
        self.port = params.pfsense_port
        self.scheme = params.pfsense_scheme
        self.restapi_pkg_url = params.pfsense_restapi_pkg_url
        self.url = params.pfsense_url
        self.client = pfsense_vshell.PFClient(
            host=self.host,
            username=self.username,
            password=self.password,
            port=self.port,
            scheme=self.scheme,
            verify=False,
        )

    def set_saml2_config(self, config: dict) -> None:
        """
        Sets the SAML2 configuration in pfSense using the provided config dictionary

        Args:
            config (dict): A dictionary containing SAML2 configuration parameters
        """
        self.client.run_command(
            f"echo '{json.dumps(config)}' > /var/cache/pfSense-pkg-saml2-auth/backup.json && "
            "pfsense-saml2 restore"
        )

    def run_command(self, command: str) -> str:
        """
        Executes a command on the pfSense instance and returns the output

        Args:
            command (str): The command to execute

        Returns:
            str: The output of the command
        """
        return self.client.run_command(command)

    def install_restapi_pkg(self) -> None:
        """
        Installs the pfSense REST API package if not already installed
        """
        # Installing the REST API causes a web server reload which drops the vshell, catch accordingly.
        try:
            self.client.run_command(f"pkg-static add {self.restapi_pkg_url}")
        except pfsense_vshell.PFError as exc:
            time.sleep(5)
            pass

    def add_user(self, username: str, password: str, privileges: list[str]) -> dict:
        """
        Creates a user in pfSense with the specified username, password, and privileges

        Args:
            username (str): The username for the new user
            password (str): The password for the new user
            privileges (list[str]): A list of privileges to assign to the user

        Returns:
            dict: The JSON response from the pfSense REST API containing user details
        """
        user_post = requests.post(
            url=f"{self.scheme}://{self.host}:{self.port}/api/v2/user",
            auth=(self.username, self.password),
            headers={"Content-Type": "application/json", "Accept": "application/json"},
            verify=False,
            timeout=30,
            json={"name": username, "password": password, "priv": privileges},
        )

        if user_post.status_code != 200:
            raise SystemError(f"Failed to create user {username}: {user_post.text}")

        return user_post.json()

    def delete_user(self, user_id: int) -> dict:
        """
        Deletes a user in pfSense with the specified username

        Args:
            user_id (int): The ID of the user to delete

        Returns:
            dict: The JSON response from the pfSense REST API confirming deletion
        """
        user_delete = requests.delete(
            url=f"{self.scheme}://{self.host}:{self.port}/api/v2/user?id={user_id}",
            auth=(self.username, self.password),
            verify=False,
            timeout=30,
            headers={
                "Content-Type": "application/x-www-form-urlencoded",
                "Accept": "application/json",
            },
        )

        if user_delete.status_code != 200:
            raise SystemError(f"Failed to delete user {user_id}: {user_delete.text}")

        return user_delete.json()

    def add_user_group(self, name: str, privileges: list[str]) -> dict:
        """
        Creates a user group in pfSense with the specified group name and privileges

        Args:
            name (str): The name for the new user group
            privileges (list[str]): A list of privileges to assign to the group

        Returns:
            dict: The JSON response from the pfSense REST API containing group details
        """
        group_post = requests.post(
            url=f"{self.scheme}://{self.host}:{self.port}/api/v2/user/group",
            auth=(self.username, self.password),
            headers={"Content-Type": "application/json", "Accept": "application/json"},
            verify=False,
            timeout=30,
            json={"name": name, "priv": privileges},
        )

        if group_post.status_code != 200:
            raise SystemError(f"Failed to create group {name}: {group_post.text}")

        return group_post.json()

    def delete_user_group(self, group_id: int) -> dict:
        """
        Deletes a user group in pfSense with the specified group ID

        Args:
            group_id (int): The ID of the group to delete

        Returns:
            dict: The JSON response from the pfSense REST API confirming deletion
        """
        group_delete = requests.delete(
            url=f"{self.scheme}://{self.host}:{self.port}/api/v2/user/group?id={group_id}",
            auth=(self.username, self.password),
            verify=False,
            timeout=30,
            headers={
                "Content-Type": "application/x-www-form-urlencoded",
                "Accept": "application/json",
            },
        )

        if group_delete.status_code != 200:
            raise SystemError(f"Failed to delete group {group_id}: {group_delete.text}")

        return group_delete.json()
