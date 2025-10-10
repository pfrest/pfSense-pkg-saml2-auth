"""Tests for ensuring basic SSO login functionality works."""

import pytest
from playwright.sync_api import Browser

from tests.helpers import Params


@pytest.mark.usefixtures("pfsense_user_group")
@pytest.mark.usefixtures("saml2_config_auto")
def test_sso_login_chromium_with_auto_config(
    params: Params, chromium_browser: Browser
) -> None:
    """
    Test SSO login functionality using Chromium browser and automatic IdP configuration
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
    assert "saml2_name_id" in session
    assert "username" in session
    assert session["username"] == session["saml2_name_id"]
    assert session["saml2_name_id"] == params.idp_expected_nameid


@pytest.mark.usefixtures("pfsense_user_group")
@pytest.mark.usefixtures("saml2_config_manual")
def test_sso_login_chromium_with_manual_config(
    params: Params, chromium_browser: Browser
) -> None:
    """
    Test SSO login functionality using Chromium browser and manual IdP configuration
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
    assert "saml2_name_id" in session
    assert "username" in session
    assert session["username"] == session["saml2_name_id"]
    assert session["saml2_name_id"] == params.idp_expected_nameid


@pytest.mark.usefixtures("pfsense_user_group")
@pytest.mark.usefixtures("saml2_config_auto")
def test_sso_login_firefox_with_auto_config(
    params: Params, firefox_browser: Browser
) -> None:
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
    assert "saml2_name_id" in session
    assert "username" in session
    assert session["username"] == session["saml2_name_id"]
    assert session["saml2_name_id"] == params.idp_expected_nameid


@pytest.mark.usefixtures("pfsense_user_group")
@pytest.mark.usefixtures("saml2_config_manual")
def test_sso_login_firefox_with_manual_config(
    params: Params, firefox_browser: Browser
) -> None:
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
    assert "saml2_name_id" in session
    assert "username" in session
    assert session["username"] == session["saml2_name_id"]
    assert session["saml2_name_id"] == params.idp_expected_nameid
