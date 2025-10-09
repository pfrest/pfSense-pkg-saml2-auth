"""Pytest configuration and fixtures for SAML2 authentication tests with pfSense"""
import pytest
from playwright.sync_api import Browser, sync_playwright
from tests.helpers import PfSenseClient, Params, Saml2Config


@pytest.fixture(scope="session")
def params() -> Params:
    """
    Pytest fixture to provide test parameters from environment variables
    """
    return Params()


@pytest.fixture
def saml2_config_default() -> Saml2Config:
    """
    Pytest fixture to provide the default valid SAML2 configuration.

    Returns:
        Saml2Config: Configuration object with expected default settings
    """
    conf = Saml2Config()
    conf.save()
    yield conf


@pytest.fixture
def saml2_config_manual() -> Saml2Config:
    """
    Pytest fixture to configure SAML2 using valid manual settings.

    Returns:
        Saml2Config: Configuration object with expected manual settings
    """
    # Obtain our default Saml2Config and modify to use only manual settings
    conf = Saml2Config()
    conf.idp_metadata_url = ""
    conf.save()
    yield conf

    # After test, restore to default valid config
    conf = Saml2Config()
    conf.save()


@pytest.fixture
def saml2_config_auto() -> Saml2Config:
    """
    Pytest fixture to configure SAML2 using valid automatic settings.

    Returns:
        Saml2Config: Configuration object with expected automatic settings
    """
    # Obtain our default Saml2Config and modify to use only automatic settings
    conf = Saml2Config()
    conf.idp_entity_id = ""
    conf.idp_sign_on_url = ""
    conf.idp_x509_cert = ""
    conf.save()
    yield conf

    # After test, restore to default valid config
    conf = Saml2Config()
    conf.save()


@pytest.fixture
def saml2_config_no_groups() -> Saml2Config:
    """
    Pytest fixture to provide a SAML2 configuration that does not map groups.

    Returns:
        Saml2Config: Configuration object with no group mappings
    """
    conf = Saml2Config()
    conf.idp_groups_attribute = ""
    conf.save()
    yield conf

    # After test, restore to default valid config
    conf = Saml2Config()
    conf.save()


@pytest.fixture
def webkit_browser() -> Browser:
    """
    Pytest fixture to provide a WebKit browser instance for tests
    """
    with sync_playwright() as playwright:
        browser = playwright.webkit.launch(headless=True)
        context = browser.new_context(ignore_https_errors=True)
        yield context
        context.close()
        browser.close()


@pytest.fixture
def chromium_browser() -> Browser:
    """
    Pytest fixture to provide a Chromium browser instance for tests
    """
    with sync_playwright() as playwright:
        browser = playwright.chromium.launch(headless=True)
        context = browser.new_context(ignore_https_errors=True)
        yield context
        context.close()
        browser.close()


@pytest.fixture
def firefox_browser() -> Browser:
    """
    Pytest fixture to provide a Firefox browser instance for tests
    """
    with sync_playwright() as playwright:
        browser = playwright.firefox.launch(headless=True)
        context = browser.new_context(ignore_https_errors=True)
        yield context
        context.close()
        browser.close()


@pytest.fixture(scope="session")
def pfsense_client() -> PfSenseClient:
    """
    Pytest fixture to provide a PfSenseClient instance for tests
    """
    client = PfSenseClient()
    client.install_restapi_pkg()
    return client


@pytest.fixture
def pfsense_user(pfsense_client: PfSenseClient, params: Params) -> None:
    """
    Pytest fixture to ensure a test user exists in pfSense
    """
    user = pfsense_client.add_user(
        username=params.idp_expected_nameid,
        password="testpassword",
        privileges=["page-all"],
    )
    yield
    pfsense_client.delete_user(user["data"]["id"])


@pytest.fixture
def pfsense_user_group(pfsense_client: PfSenseClient, params: Params) -> None:
    """
    Pytest fixture to ensure a test user group exists in pfSense
    """
    group = pfsense_client.add_user_group(
        name=params.idp_expected_group, privileges=["page-all"]
    )
    yield
    pfsense_client.delete_user_group(group["data"]["id"])
