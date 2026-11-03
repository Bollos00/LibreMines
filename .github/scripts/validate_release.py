#!/usr/bin/env python3

import argparse
from datetime import date
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[2]
APPDATA_PATH = ROOT / "share" / "metainfo" / "io.github.Bollos00.LibreMines.appdata.xml"
CMAKE_PATH = ROOT / "CMakeLists.txt"
RELEASES_URL = "https://github.com/Bollos00/LibreMines/releases/tag/"


def fail(message: str) -> None:
    print(f"::error::{message}")
    raise SystemExit(1)


def local_name(tag: str) -> str:
    return tag.rsplit("}", 1)[-1]


def get_cmake_version(path: Path) -> str:
    try:
        cmake_content = path.read_text(encoding="utf-8")
    except OSError as error:
        fail(f"Cannot read {path}: {error}")

    project = re.search(
        r"\bproject\s*\(\s*libremines\b(?P<arguments>.*?)\)",
        cmake_content,
        re.IGNORECASE | re.DOTALL,
    )
    if project is None:
        fail(f"Cannot find the LibreMines project declaration in {path}.")

    version = re.search(
        r'\bVERSION\s+(?:"([^"]+)"|([^\s)]+))',
        project.group("arguments"),
        re.IGNORECASE,
    )
    if version is None:
        fail(f"Cannot find VERSION in the LibreMines project declaration in {path}.")

    return version.group(1) or version.group(2)


def validate_appdata(version: str, tag: str, path: Path) -> None:
    try:
        root = ET.parse(path).getroot()
    except (OSError, ET.ParseError) as error:
        fail(f"Cannot read valid AppStream metadata from {path}: {error}")

    releases_containers = [
        child for child in root if local_name(child.tag) == "releases"
    ]
    if len(releases_containers) != 1:
        fail("AppStream metadata must contain exactly one <releases> element.")

    matching_releases = [
        child
        for child in releases_containers[0]
        if local_name(child.tag) == "release"
        and child.attrib.get("version") == version
    ]
    if len(matching_releases) != 1:
        fail(
            f"AppStream metadata must contain exactly one release entry "
            f"for version {version}; found {len(matching_releases)}."
        )

    release = matching_releases[0]
    release_date = release.attrib.get("date", "")
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", release_date):
        fail(f"Release {version} must have a date in YYYY-MM-DD format.")
    try:
        date.fromisoformat(release_date)
    except ValueError:
        fail(f"Release {version} has an invalid calendar date: {release_date}.")

    detail_urls = [
        child
        for child in release
        if local_name(child.tag) == "url" and child.attrib.get("type") == "details"
    ]
    expected_url = f"{RELEASES_URL}{tag}"
    if len(detail_urls) != 1 or (detail_urls[0].text or "").strip() != expected_url:
        fail(
            f"Release {version} must have exactly one details URL equal to "
            f"{expected_url}."
        )


def validate_release(tag: str, root: Path) -> None:
    if not tag.startswith("v") or len(tag) == 1:
        fail(f"Release tag must begin with 'v' and include a version; got {tag!r}.")

    version = tag[1:]
    cmake_version = get_cmake_version(root / "CMakeLists.txt")
    if version != cmake_version:
        fail(
            f"Release tag {tag!r} does not match CMake VERSION "
            f"{cmake_version!r} after removing the leading 'v'."
        )

    validate_appdata(
        version,
        tag,
        root / "share" / "metainfo" / "io.github.Bollos00.LibreMines.appdata.xml",
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Validate a release tag against CMake and AppStream metadata."
    )
    parser.add_argument("tag", help="The complete release tag, including its leading v.")
    args = parser.parse_args()

    validate_release(args.tag, ROOT)
    print(f"Release tag {args.tag} and AppStream metadata are valid.")


if __name__ == "__main__":
    main()
