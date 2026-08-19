"""Tests for ensuring disabled SAML2 auth does not start an SSO session."""

import pytest
from playwright.sync_api import Browser

from tests.helpers.params import Params


def assert_disabled_sso_attempt_shows_error(params: Params, browser: Browser) -> None:
    """Assert that opening the SSO endpoint while disabled shows the error page and creates no session."""
    page = browser.new_page()

    # Attempt to start SSO directly and ensure we are redirected to the package error page
    page.goto(f"{params.pfsense_url}/saml2_auth/sso/")
    page.wait_for_selector("#saml2_error_notice")

    assert "/saml2_auth/sso/error/" in page.url
    assert page.title() == "pfSense - SSO Authentication Failed"
    assert (
        page.locator("#saml2_error_notice").inner_text() == "SSO Authentication Failed"
    )

    # Ensure the failed auth attempt did not create an SSO-backed session
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" in session
    assert session_resp.status == 401


def assert_enabled_sso_attempt_starts_auth(params: Params, browser: Browser) -> None:
    """Assert that opening the SSO endpoint while enabled starts and completes SSO auth."""
    page = browser.new_page()

    # Starting at the SSO endpoint should proceed through auth instead of failing on the package error page
    page.goto(f"{params.pfsense_url}/saml2_auth/sso/")
    page.get_by_role("link", name="Dashboard").click()

    assert "/saml2_auth/sso/error/" not in page.url

    # Ensure a successful SSO session exists after the flow completes
    session_resp = page.goto(f"{params.pfsense_url}/saml2_auth/sso/session/")
    session = session_resp.json()
    assert "error" not in session


@pytest.mark.usefixtures("saml2_config_disabled")
def test_sso_login_disabled_chromium(params: Params, chromium_browser: Browser) -> None:
    """Ensure Chromium sees the SSO failure page when the package is disabled."""
    assert_disabled_sso_attempt_shows_error(params, chromium_browser)


@pytest.mark.usefixtures("saml2_config_disabled")
def test_sso_login_disabled_firefox(params: Params, firefox_browser: Browser) -> None:
    """Ensure Firefox sees the SSO failure page when the package is disabled."""
    assert_disabled_sso_attempt_shows_error(params, firefox_browser)


@pytest.mark.usefixtures("pfsense_user_group")
@pytest.mark.usefixtures("saml2_config_default")
def test_sso_login_enabled_chromium(params: Params, chromium_browser: Browser) -> None:
    """Ensure Chromium can start SSO from the SSO endpoint when the package is enabled."""
    assert_enabled_sso_attempt_starts_auth(params, chromium_browser)


@pytest.mark.usefixtures("pfsense_user_group")
@pytest.mark.usefixtures("saml2_config_default")
def test_sso_login_enabled_firefox(params: Params, firefox_browser: Browser) -> None:
    """Ensure Firefox can start SSO from the SSO endpoint when the package is enabled."""
    assert_enabled_sso_attempt_starts_auth(params, firefox_browser)
