#!/usr/bin/env python3

import hashlib
import http.client
import json
import os
import sys
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path


API_ROOT = "https://api.github.com"
REPOSITORY = "moneymanagerex/moneymanagerex"
RELEASE_TAG = "nightly"
API_HEADERS = {
    "Accept": "application/vnd.github+json",
    "X-GitHub-Api-Version": "2022-11-28",
}


def api_request(method, path, token, body=None, allow_not_found=False):
    url = f"{API_ROOT}{path}"
    scheme = urllib.parse.urlsplit(url).scheme
    if scheme not in ("http", "https"):
        raise RuntimeError(f"Unsupported URL scheme: {scheme}")

    headers = {**API_HEADERS, "Authorization": f"Bearer {token}"}
    data = json.dumps(body).encode() if body is not None else None
    if data is not None:
        headers["Content-Type"] = "application/json"
    request = urllib.request.Request(
        f"{API_ROOT}{path}", data=data, headers=headers, method=method
    )
    try:
        with urllib.request.urlopen(request, timeout=30) as response:
            if response.status == 204:
                return None
            return json.load(response)
    except urllib.error.HTTPError as error:
        details = error.read().decode("utf-8", errors="replace")
        if error.code == 404 and allow_not_found:
            return None
        raise RuntimeError(
            f"GitHub API {method} {path} failed ({error.code}): {details}"
        ) from error


def upload_asset(upload_url, path, token):
    parsed_url = urllib.parse.urlsplit(upload_url)
    query = urllib.parse.urlencode({"name": path.name})
    connection = http.client.HTTPSConnection(parsed_url.netloc, timeout=120)
    connection.putrequest(
        "POST", f"{parsed_url.path}?{query}", skip_accept_encoding=True
    )
    connection.putheader("Authorization", f"Bearer {token}")
    connection.putheader("Accept", "application/vnd.github+json")
    connection.putheader("Content-Type", "application/octet-stream")
    connection.putheader("Content-Length", str(path.stat().st_size))
    connection.endheaders()

    with path.open("rb") as asset:
        while chunk := asset.read(1024 * 1024):
            connection.send(chunk)

    response = connection.getresponse()
    details = response.read().decode("utf-8", errors="replace")
    connection.close()
    if response.status != 201:
        raise RuntimeError(
            f"GitHub asset upload failed ({response.status}): {details}"
        )


def main():
    if len(sys.argv) != 3:
        raise SystemExit("Usage: publish_github_release.py <flatpak> <commit-sha>")

    token = os.environ.get("CIRCLECI_GITHUB_TOKEN")
    if not token:
        raise RuntimeError(
            "CIRCLECI_GITHUB_TOKEN is required to publish the development release"
        )

    artifact = Path(sys.argv[1])
    commit_sha = sys.argv[2]
    if not artifact.is_file():
        raise RuntimeError(f"Flatpak artifact not found: {artifact}")

    api_base = f"/repos/{REPOSITORY}"
    ref_path = f"{api_base}/git/ref/tags/{RELEASE_TAG}"
    tag_ref = api_request("GET", ref_path, token, allow_not_found=True)
    if tag_ref is None:
        api_request(
            "POST",
            f"{api_base}/git/refs",
            token,
            {"ref": f"refs/tags/{RELEASE_TAG}", "sha": commit_sha},
        )
    elif tag_ref["object"]["sha"] != commit_sha:
        api_request(
            "PATCH",
            ref_path,
            token,
            {"sha": commit_sha, "force": True},
        )

    release_path = f"{api_base}/releases/tags/{RELEASE_TAG}"
    release = api_request("GET", release_path, token, allow_not_found=True)
    release_body = (
        "Automated development build of MMEX for Linux.\n\n"
        f"Commit: `{commit_sha}`\n"
        f"CircleCI build: {os.environ.get('CIRCLE_BUILD_URL', 'not available')}\n"
        "This prerelease is replaced by the next successful build on `master`."
    )
    release_data = {
        "tag_name": RELEASE_TAG,
        "target_commitish": commit_sha,
        "name": "Latest MMEX development build",
        "body": release_body,
        "prerelease": True,
        "draft": False,
    }
    if release is None:
        release = api_request("POST", f"{api_base}/releases", token, release_data)
    else:
        release = api_request(
            "PATCH", f"{api_base}/releases/{release['id']}", token, release_data
        )

    digest = hashlib.sha256()
    with artifact.open("rb") as artifact_file:
        while chunk := artifact_file.read(1024 * 1024):
            digest.update(chunk)
    checksum = digest.hexdigest()
    checksum_path = artifact.with_name("SHA256SUMS")
    checksum_path.write_text(f"{checksum}  {artifact.name}\n", encoding="ascii")
    asset_names = {artifact.name, checksum_path.name}
    for existing_asset in release["assets"]:
        if existing_asset["name"] in asset_names:
            api_request(
                "DELETE",
                f"{api_base}/releases/assets/{existing_asset['id']}",
                token,
            )

    upload_url = release["upload_url"].split("{", 1)[0]
    upload_asset(upload_url, artifact, token)
    upload_asset(upload_url, checksum_path, token)
    print(f"Published {artifact.name} to {REPOSITORY}/releases/tag/{RELEASE_TAG}")


if __name__ == "__main__":
    main()
