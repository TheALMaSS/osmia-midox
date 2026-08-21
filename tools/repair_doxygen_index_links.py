#!/usr/bin/env python3
"""Repair malformed alphabet-index links emitted by some Doxygen releases."""

from __future__ import annotations

import html
import re
import sys
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit


HREF_RE = re.compile(r'href="([^"]+)"')
LI_RE = re.compile(r"<li>(.*?)</li>", re.DOTALL | re.IGNORECASE)
TAG_RE = re.compile(r"<[^>]+>")


class AnchorParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.anchors: set[str] = set()

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        values = dict(attrs)
        for key in ("id", "name"):
            if values.get(key):
                self.anchors.add(values[key] or "")


def anchors(path: Path) -> set[str]:
    parser = AnchorParser()
    parser.feed(path.read_text(encoding="utf-8", errors="replace"))
    return parser.anchors


def encoded_fragment(fragment: str) -> str:
    if fragment == "index_~":
        return "index__7E"
    if fragment == "index__":
        return "index__5F"
    return fragment


def visible_initial(fragment: str) -> str:
    suffix = fragment.removeprefix("index_")
    if suffix in {"_5F", "_"}:
        return "_"
    if suffix in {"_7E", "~"}:
        return "~"
    return suffix[:1].casefold()


def insert_missing_anchor(content: str, fragment: str) -> tuple[str, bool]:
    initial = visible_initial(fragment)
    for match in LI_RE.finditer(content):
        item = html.unescape(TAG_RE.sub("", match.group(1))).lstrip()
        if item and item[0].casefold() == initial:
            marker = f'<a id="{fragment}"></a>\n'
            return content[: match.start()] + marker + content[match.start() :], True
    return content, False


def main() -> int:
    html_root = Path(sys.argv[1] if len(sys.argv) > 1 else "doxygen_output/html").resolve()
    report_path = html_root.parent / "index_link_repair.log"
    pages = sorted(html_root.rglob("*.html"))
    anchor_map = {page.resolve(): anchors(page) for page in pages}
    changed_links = 0
    inserted_anchors = 0
    unresolved: list[str] = []

    for page in pages:
        content = page.read_text(encoding="utf-8", errors="replace")

        def replace(match: re.Match[str]) -> str:
            nonlocal changed_links
            reference = html.unescape(match.group(1))
            parsed = urlsplit(reference)
            if parsed.scheme or parsed.netloc or not parsed.fragment.startswith("index_"):
                return match.group(0)
            path_text = unquote(parsed.path)
            target = (page if not path_text else page.parent / path_text).resolve()
            fragment = unquote(parsed.fragment)
            if target.exists() and fragment in anchor_map.get(target, set()):
                return match.group(0)

            escaped = encoded_fragment(fragment)
            if target.exists() and escaped in anchor_map.get(target, set()):
                changed_links += 1
                new_ref = (parsed.path + "#" + escaped) if parsed.path else ("#" + escaped)
                return f'href="{new_ref}"'

            # Doxygen writes globals__.html for the underscore category although
            # the content is held in globals.html under index__5F.
            if path_text.endswith("__.html"):
                base_name = Path(path_text).name[:-7] + ".html"
                base = (page.parent / base_name).resolve()
                if base.exists() and "index__5F" in anchor_map.get(base, set()):
                    changed_links += 1
                    return f'href="{base_name}#index__5F"'

            # Doxygen can point globals.html#index_a at the split page
            # globals_a.html#index_a.
            if target.exists() and path_text and len(fragment) == len("index_") + 1:
                split_name = Path(path_text).stem + "_" + fragment[-1] + ".html"
                split = (page.parent / split_name).resolve()
                if split.exists() and fragment in anchor_map.get(split, set()):
                    changed_links += 1
                    return f'href="{split_name}#{fragment}"'

            return match.group(0)

        content = HREF_RE.sub(replace, content)
        page.write_text(content, encoding="utf-8")

    # Some unsplit Doxygen index pages contain qindex links but omit their
    # anchors. Insert each missing anchor before the matching first list item.
    for page in pages:
        content = page.read_text(encoding="utf-8", errors="replace")
        current_anchors = anchors(page)
        self_fragments = {
            unquote(urlsplit(html.unescape(ref)).fragment)
            for ref in HREF_RE.findall(content)
            if not urlsplit(html.unescape(ref)).path
            and urlsplit(html.unescape(ref)).fragment.startswith("index_")
        }
        for fragment in sorted(self_fragments):
            if fragment in current_anchors:
                continue
            content, inserted = insert_missing_anchor(content, fragment)
            if inserted:
                current_anchors.add(fragment)
                inserted_anchors += 1
            else:
                unresolved.append(f"{page.name}#{fragment}")
        page.write_text(content, encoding="utf-8")

    lines = [
        "Osmia MIDox Doxygen index-link repair",
        f"HTML pages examined: {len(pages)}",
        f"Generated qindex links repaired: {changed_links}",
        f"Missing generated qindex anchors inserted: {inserted_anchors}",
        f"Unresolved generated qindex targets: {len(unresolved)}",
    ]
    if unresolved:
        lines.extend(("", "Unresolved targets:", *(f"- {item}" for item in unresolved)))
    lines.extend(("", "RESULT: " + ("FAIL" if unresolved else "PASS")))
    report_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines))
    return 1 if unresolved else 0


if __name__ == "__main__":
    sys.exit(main())
