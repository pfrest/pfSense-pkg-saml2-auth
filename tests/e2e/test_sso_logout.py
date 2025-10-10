"""Tests for ensuring logouts end SSO sessions."""

import pytest
from playwright.sync_api import Browser

from tests.helpers.params import Params


@pytest.mark.usefixtures("pfsense_user_group")
@pytest.mark.usefixtures("saml2_config_default")
def test_sso_logout_chromium(params: Params, chromium_browser: Browser) -> None:
    """
    Test SSO logout functionality using Chromium browser
    """
    page = chromium_browser.new_page()

    # Navigate to the home page and initiate an SSO login
    page.goto(f"{params.pfsense_url}/")
    page.get_by_role("link", name="Sign In with SSO").click()
    page.get_by_role("link", name="Dashboard").click()

    # Ensure we have successfully logged in by checking for session details
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" not in session

    # Click the logout link and ensure we are logged out
    page.goto(f"{params.pfsense_url}/")
    page.locator('a[href="/index.php?logout"]').locator("visible=true").click()
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" in session
    assert session_resp.status == 401


@pytest.mark.usefixtures("pfsense_user_group")
@pytest.mark.usefixtures("saml2_config_default")
def test_sso_logout_firefox(params: Params, firefox_browser: Browser) -> None:
    """
    Test SSO login functionality using Firefox browser
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

    # Click the logout link and ensure we are logged out
    page.goto(f"{params.pfsense_url}/")
    page.locator('a[href="/index.php?logout"]').locator("visible=true").click()
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" in session
    assert session_resp.status == 401
