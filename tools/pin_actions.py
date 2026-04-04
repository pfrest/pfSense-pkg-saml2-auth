#!/usr/bin/env python3
"""
Script to parse GitHub Actions workflow files and fetch the latest version tags
and SHA hashes for each action using the GitHub API.

Usage:
    python tools/pin_actions.py [--token GITHUB_TOKEN] [--update]

Options:
    --token     GitHub personal access token (or set GITHUB_TOKEN env var)
    --update    Automatically update workflow files with SHA-pinned versions

No external dependencies required - uses only Python standard library.
"""

import argparse
import json
import os
import re
import ssl
import sys
import urllib.request
import urllib.error
from pathlib import Path


def get_github_token():
    """Get GitHub token from environment or return None."""
    return os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")


def parse_action_reference(action_ref: str) -> tuple[str, str, str] | None:
    """
    Parse an action reference like 'owner/repo@version' or 'owner/repo/path@version'.
    Returns (owner, repo, version) or None if not a valid reference.
    """
    # Skip local actions (starting with ./)
    if action_ref.startswith("./"):
        return None

    # Skip docker actions
    if action_ref.startswith("docker://"):
        return None

    # Match owner/repo@version or owner/repo/path@version
    match = re.match(r"^([^/]+)/([^/@]+)(?:/[^@]+)?@(.+)$", action_ref)
    if match:
        return match.group(1), match.group(2), match.group(3)
    return None


def extract_actions_from_workflow(workflow_path: Path) -> list[dict]:
    """
    Extract all action references from a workflow file.
    Returns list of dicts with action info and location.
    """
    actions = []

    with open(workflow_path, "r") as f:
        content = f.read()

    lines = content.splitlines()
    for line_num, line in enumerate(lines, 1):
        # Match 'uses:' lines
        match = re.search(r"uses:\s*([^\s#]+)", line)
        if match:
            action_ref = match.group(1).strip()
            parsed = parse_action_reference(action_ref)
            if parsed:
                owner, repo, version = parsed
                actions.append(
                    {
                        "owner": owner,
                        "repo": repo,
                        "version": version,
                        "original_ref": action_ref,
                        "file": workflow_path,
                        "line": line_num,
                        "full_line": line,
                    }
                )

    return actions


def make_api_request(url: str, token: str | None = None) -> dict | list | None:
    """Make a request to the GitHub API and return JSON response."""
    headers = {"Accept": "application/vnd.github+json", "User-Agent": "pin-actions-script"}
    if token:
        headers["Authorization"] = f"Bearer {token}"

    req = urllib.request.Request(url, headers=headers)
    try:
        ctx = ssl.create_default_context()
        with urllib.request.urlopen(req, timeout=10, context=ctx) as response:
            return json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as e:
        if e.code == 404:
            return None
        print(f"Warning: API request failed ({e.code}): {url}", file=sys.stderr)
        return None
    except urllib.error.URLError as e:
        print(f"Warning: API request failed: {url} - {e}", file=sys.stderr)
        return None


def get_latest_release(owner: str, repo: str, token: str | None = None) -> dict | None:
    """
    Fetch the latest release from GitHub API.
    Returns dict with tag_name and other info, or None if not found.
    """
    url = f"https://api.github.com/repos/{owner}/{repo}/releases/latest"
    return make_api_request(url, token)


def get_all_tags(owner: str, repo: str, token: str | None = None) -> list[dict]:
    """Fetch all tags from GitHub API."""
    url = f"https://api.github.com/repos/{owner}/{repo}/tags?per_page=100"
    result = make_api_request(url, token)
    return result if result else []


def get_tag_sha(owner: str, repo: str, tag: str, token: str | None = None) -> str | None:
    """Get the commit SHA for a specific tag."""
    url = f"https://api.github.com/repos/{owner}/{repo}/git/refs/tags/{tag}"
    data = make_api_request(url, token)

    if data:
        obj = data.get("object", {})
        sha = obj.get("sha")
        obj_type = obj.get("type")

        # If it's an annotated tag, we need to get the commit it points to
        if obj_type == "tag":
            tag_url = f"https://api.github.com/repos/{owner}/{repo}/git/tags/{sha}"
            tag_data = make_api_request(tag_url, token)
            if tag_data:
                return tag_data.get("object", {}).get("sha")
        return sha
    return None


def parse_version(version: str) -> tuple:
    """Parse a version string for comparison."""
    v = version.lstrip("v")
    parts = re.split(r"[.\-_]", v)
    result = []
    for part in parts:
        try:
            result.append((0, int(part)))
        except ValueError:
            result.append((1, part))
    return tuple(result)


def is_prerelease(tag_name: str) -> bool:
    """
    Check if a tag name indicates a pre-release version.
    Pre-release indicators: alpha, beta, rc, dev, preview, canary, nightly, etc.
    """
    prerelease_patterns = [
        r"alpha",
        r"beta",
        r"[\.\-_]?rc\d*",       # e.g., 1.0-rc1, 1.0.rc2
        r"[\.\-_]dev",          # e.g., 1.0-dev, 1.0.dev
        r"preview",
        r"canary",
        r"nightly",
        r"(?<=[0-9])a\d+$",     # e.g., 26.1a1 (alpha with number suffix)
        r"(?<=[0-9])b\d+$",     # e.g., 1.0b2 (beta with number suffix)
    ]
    tag_lower = tag_name.lower()
    for pattern in prerelease_patterns:
        if re.search(pattern, tag_lower):
            return True
    return False


def get_latest_version_tag(
    owner: str, repo: str, current_version: str, token: str | None = None,
    include_prereleases: bool = False
) -> tuple[str, str] | None:
    """
    Get the latest version tag and its SHA.
    Returns (tag_name, sha) or None.
    """
    # Check if current version is already a SHA
    if re.match(r"^[a-f0-9]{40}$", current_version):
        return None

    tags = get_all_tags(owner, repo, token)
    if not tags:
        return None

    # Determine the major version prefix from current version
    current_clean = current_version.lstrip("v")
    major_match = re.match(r"^(\d+)", current_clean)

    # Filter and sort tags
    version_tags = []
    for tag in tags:
        name = tag["name"]
        if not re.match(r"^v?\d", name):
            continue

        # Skip pre-releases unless explicitly included
        if not include_prereleases and is_prerelease(name):
            continue

        if major_match:
            tag_clean = name.lstrip("v")
            tag_major = re.match(r"^(\d+)", tag_clean)
            if tag_major and tag_major.group(1) == major_match.group(1):
                version_tags.append((name, tag["commit"]["sha"]))
        else:
            version_tags.append((name, tag["commit"]["sha"]))

    if not version_tags:
        for tag in tags:
            name = tag["name"]
            if re.match(r"^v?\d", name):
                # Skip pre-releases unless explicitly included
                if not include_prereleases and is_prerelease(name):
                    continue
                version_tags.append((name, tag["commit"]["sha"]))

    if not version_tags:
        return None

    version_tags.sort(key=lambda x: parse_version(x[0]), reverse=True)
    latest_tag, commit_sha = version_tags[0]

    actual_sha = get_tag_sha(owner, repo, latest_tag, token)
    if actual_sha:
        return latest_tag, actual_sha

    return latest_tag, commit_sha


def find_workflow_files(workflows_dir: Path) -> list[Path]:
    """Find all YAML workflow files in the workflows directory."""
    files = []
    if workflows_dir.exists():
        for pattern in ["*.yml", "*.yaml"]:
            files.extend(workflows_dir.glob(pattern))
    return sorted(files)


def update_workflow_file(file_path: Path, updates: list[dict]) -> bool:
    """Update a workflow file with SHA-pinned action references."""
    with open(file_path, "r") as f:
        lines = f.readlines()

    # Sort updates by line number in reverse order to avoid index shifting
    updates_sorted = sorted(updates, key=lambda x: x["line"], reverse=True)
    modified = False

    for update in updates_sorted:
        line_idx = update["line"] - 1
        old_line = lines[line_idx]

        owner = update["owner"]
        repo = update["repo"]
        new_tag = update["latest_tag"]
        new_sha = update["latest_sha"]

        # Check if there's a path component in the action
        original_ref = update["original_ref"]
        path_match = re.match(rf"^{re.escape(owner)}/{re.escape(repo)}(/[^@]+)?@", original_ref)
        path_component = path_match.group(1) if path_match and path_match.group(1) else ""

        new_ref = f"{owner}/{repo}{path_component}@{new_sha}"
        release_url = f"https://github.com/{owner}/{repo}/releases/tag/{new_tag}"

        # Replace the uses line
        new_line = re.sub(
            r"(uses:\s*)[^\s#]+(\s*#.*)?$",
            rf"\1{new_ref} # {new_tag}",
            old_line.rstrip(),
        )

        # Get indentation
        indent_match = re.match(r"^(\s*)", old_line)
        indent = indent_match.group(1) if indent_match else "      "

        # Check if previous line already has a release URL comment
        prev_line = lines[line_idx - 1] if line_idx > 0 else ""
        lines[line_idx] = new_line + "\n"

        modified = True

    if modified:
        with open(file_path, "w") as f:
            f.writelines(lines)

    return modified


def main():
    parser = argparse.ArgumentParser(
        description="Parse GitHub Actions workflows and fetch latest version SHAs"
    )
    parser.add_argument(
        "--token",
        help="GitHub personal access token (or set GITHUB_TOKEN env var)",
        default=None,
    )
    parser.add_argument(
        "--update",
        action="store_true",
        help="Automatically update workflow files with SHA-pinned versions",
    )
    parser.add_argument(
        "--include-prereleases",
        action="store_true",
        help="Include pre-release versions (alpha, beta, rc, dev, etc.)",
    )
    parser.add_argument(
        "--workflows-dir",
        type=Path,
        default=None,
        help="Path to workflows directory (default: .github/workflows)",
    )
    args = parser.parse_args()

    # Find repository root
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent

    workflows_dir = args.workflows_dir or repo_root / ".github" / "workflows"

    if not workflows_dir.exists():
        print(f"Error: Workflows directory not found: {workflows_dir}", file=sys.stderr)
        sys.exit(1)

    token = args.token or get_github_token()
    if not token:
        print(
            "Warning: No GitHub token provided. API rate limits may apply.",
            file=sys.stderr,
        )
        print(
            "Set GITHUB_TOKEN env var or use --token option for higher limits.\n",
            file=sys.stderr,
        )

    # Find all workflow files
    workflow_files = find_workflow_files(workflows_dir)
    if not workflow_files:
        print(f"No workflow files found in {workflows_dir}")
        sys.exit(0)

    print(f"Found {len(workflow_files)} workflow file(s)\n")

    # Extract all actions
    all_actions = []
    for wf in workflow_files:
        actions = extract_actions_from_workflow(wf)
        all_actions.extend(actions)

    # Deduplicate by owner/repo
    unique_actions = {}
    for action in all_actions:
        key = f"{action['owner']}/{action['repo']}"
        if key not in unique_actions:
            unique_actions[key] = action

    print(f"Found {len(unique_actions)} unique action(s)\n")
    print("=" * 80)

    # Fetch latest versions
    results = []
    for key, action in sorted(unique_actions.items()):
        owner = action["owner"]
        repo = action["repo"]
        current = action["version"]

        print(f"\n{owner}/{repo}")
        print(f"  Current: @{current}")

        result = get_latest_version_tag(owner, repo, current, token, args.include_prereleases)
        if result:
            latest_tag, latest_sha = result
            print(f"  Latest:  @{latest_sha} # {latest_tag}")
            print(f"  Release: https://github.com/{owner}/{repo}/releases/tag/{latest_tag}")

            is_sha = re.match(r"^[a-f0-9]{40}$", current)
            if is_sha and current == latest_sha:
                print("  Status:  ✓ Already pinned to latest")
            elif current == latest_tag or current == latest_tag.lstrip("v"):
                print("  Status:  ⚠ Using version tag, should be SHA-pinned")
            else:
                print("  Status:  ⚠ Update available")

            results.append(
                {
                    "owner": owner,
                    "repo": repo,
                    "current": current,
                    "latest_tag": latest_tag,
                    "latest_sha": latest_sha,
                }
            )
        else:
            print("  Status:  ? Could not determine latest version")

    print("\n" + "=" * 80)

    if results:
        print("\n## SHA-Pinned References\n")
        print("Copy these to update your workflow files:\n")
        print("```yaml")
        for r in results:
            owner = r["owner"]
            repo = r["repo"]
            tag = r["latest_tag"]
            sha = r["latest_sha"]
            print(f"# https://github.com/{owner}/{repo}/releases/tag/{tag}")
            print(f"- uses: {owner}/{repo}@{sha} # {tag}")
            print()
        print("```")

        if args.update:
            print("\n## Updating workflow files...\n")
            updates_by_file: dict[Path, list[dict]] = {}
            for action in all_actions:
                key = f"{action['owner']}/{action['repo']}"
                for r in results:
                    if f"{r['owner']}/{r['repo']}" == key:
                        file_path = action["file"]
                        if file_path not in updates_by_file:
                            updates_by_file[file_path] = []
                        updates_by_file[file_path].append({**action, **r})
                        break

            for file_path, updates in updates_by_file.items():
                if update_workflow_file(file_path, updates):
                    print(f"  Updated: {file_path}")

            print("\nDone! Please review the changes.")


if __name__ == "__main__":
    main()




