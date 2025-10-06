"""Tests for ensuring SSO logins map to correct privileges based on matching local users."""
from playwright.sync_api import Browser
from tests.helpers import Params
from tests.conftest import params, chromium_browser, pfsense_user, saml2_config_no_groups
import pytest


@pytest.mark.usefixtures('pfsense_user')
@pytest.mark.usefixtures('saml2_config_no_groups')
def test_sso_local_user_privilege_mapping_chromium(params: Params, chromium_browser: Browser) -> None:
    """
    Test that SSO users can inherit privileges based on matching local users.
    """
    page = chromium_browser.new_page()

    # Navigate to the home page and initiate an SSO login
    page.goto(f"{params.pfsense_url}/")
    page.get_by_role("link", name="Sign In with SSO").click()
    page.wait_for_timeout(3000)
    with open("/tmp/debug.html", "w") as f:
        f.write(page.content())
    page.get_by_role("link", name="Dashboard").click()

    # Ensure we have successfully logged in by checking for session details
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" not in session
    assert "allowed_pages" in session
    assert "*" in session["allowed_pages"]  # pfsense_user fixture creates user with page-all privileges


@pytest.mark.usefixtures('saml2_config_default')
def test_sso_for_non_local_user_with_no_priv_chromium(params: Params, chromium_browser: Browser) -> None:
    """
    Ensure SSO logins for a user that is not mapped to a local user or group are denied access.
    """
    page = chromium_browser.new_page()

    # Navigate to the home page and initiate an SSO login
    page.goto(f"{params.pfsense_url}/")
    page.get_by_role("link", name="Sign In with SSO").click()
    page.wait_for_selector("text=No page assigned to this user!")
    assert "No page assigned to this user!" in page.content()

    # Since no privileges are mapped to this SSO user, ensure they did not establish a session
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" in session
    assert session_resp.status == 401


@pytest.mark.usefixtures('pfsense_user')
@pytest.mark.usefixtures('saml2_config_no_groups')
def test_sso_local_user_privilege_mapping_firefox(params: Params, firefox_browser: Browser) -> None:
    """
    Test that SSO users can inherit privileges based on matching local users.
    """
    page = firefox_browser.new_page()

    # Navigate to the home page and initiate an SSO login
    page.goto(f"{params.pfsense_url}/")
    page.get_by_role("link", name="Sign In with SSO").click()
    page.get_by_role("link", name="Dashboard").click()

    # Ensure we have successfully logged in by checking for session details
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" not in session
    assert "allowed_pages" in session
    assert "*" in session["allowed_pages"]  # pfsense_user fixture creates user with page-all privileges


@pytest.mark.usefixtures('saml2_config_default')
def test_sso_for_non_local_user_with_no_priv_firefox(params: Params, firefox_browser: Browser) -> None:
    """
    Ensure SSO logins for a user that is not mapped to a local user or group are denied access.
    """
    page = firefox_browser.new_page()

    # Navigate to the home page and initiate an SSO login
    page.goto(f"{params.pfsense_url}/")
    page.get_by_role("link", name="Sign In with SSO").click()
    page.wait_for_selector("text=No page assigned to this user!")
    assert "No page assigned to this user!" in page.content()

    # Since no privileges are mapped to this SSO user, ensure they did not establish a session
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" in session
    assert session_resp.status == 401
