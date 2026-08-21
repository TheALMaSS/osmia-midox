#!/usr/bin/env python3
"""Verify generated Doxygen HTML and report unresolved local links."""

from __future__ import annotations

import html
import sys
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit


EXPECTED_TERMS = (
    "Osmia_Population_Manager",
    "Osmia_Base",
    "Osmia_Egg",
    "Osmia_Larva",
    "Osmia_Prepupa",
    "Osmia_Pupa",
    "Osmia_InCocoon",
    "Osmia_Female",
    "Osmia_Nest",
    "Osmia_Nest_Manager",
)


class ReferenceParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.references: list[tuple[str, str]] = []
        self.anchors: set[str] = set()

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        values = dict(attrs)
        if values.get("id"):
            self.anchors.add(values["id"] or "")
        if values.get("name"):
            self.anchors.add(values["name"] or "")
        if tag in {"a", "link"} and values.get("href"):
            self.references.append((tag, values["href"] or ""))
        if tag in {"script", "img", "source"} and values.get("src"):
            self.references.append((tag, values["src"] or ""))


def local_target(page: Path, reference: str, html_root: Path) -> tuple[Path, str] | None:
    reference = html.unescape(reference).strip()
    if not reference or reference.startswith(("javascript:", "mailto:", "data:")):
        return None
    parsed = urlsplit(reference)
    if parsed.scheme or parsed.netloc:
        return None
    path_text = unquote(parsed.path)
    if not path_text:
        target = page
    elif path_text.startswith("/"):
        target = html_root / path_text.lstrip("/")
    else:
        target = page.parent / path_text
    return target.resolve(), unquote(parsed.fragment)


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "doxygen_output").resolve()
    html_root = root / "html"
    report = root / "link_check_report.txt"
    lines: list[str] = []

    if not (html_root / "index.html").is_file():
        lines.append("FAIL: html/index.html is missing")
        report.parent.mkdir(parents=True, exist_ok=True)
        report.write_text("\n".join(lines) + "\n", encoding="utf-8")
        print(lines[-1])
        return 1

    pages = sorted(html_root.rglob("*.html"))
    parsed_pages: dict[Path, ReferenceParser] = {}
    corpus_parts: list[str] = []
    for page in pages:
        content = page.read_text(encoding="utf-8", errors="replace")
        parser = ReferenceParser()
        parser.feed(content)
        parsed_pages[page.resolve()] = parser
        corpus_parts.append(content)

    broken_files: list[str] = []
    broken_anchors: list[str] = []
    checked_references = 0
    for page, parser in parsed_pages.items():
        for _, reference in parser.references:
            target_info = local_target(page, reference, html_root)
            if target_info is None:
                continue
            checked_references += 1
            target, fragment = target_info
            if not target.exists():
                broken_files.append(f"{page.relative_to(html_root)} -> {reference}")
                continue
            if fragment and target.suffix.lower() in {".html", ".htm"}:
                target_parser = parsed_pages.get(target)
                if target_parser is not None and fragment not in target_parser.anchors:
                    broken_anchors.append(f"{page.relative_to(html_root)} -> {reference}")

    corpus = "\n".join(corpus_parts)
    missing_terms = [term for term in EXPECTED_TERMS if term not in corpus]
    warning_file = root / "doxygen_warnings.log"
    warning_count = 0
    if warning_file.exists():
        warning_count = sum(1 for line in warning_file.read_text(encoding="utf-8", errors="replace").splitlines() if line.strip())

    lines.extend(
        (
            "Osmia MIDox Doxygen link check",
            f"HTML pages: {len(pages)}",
            f"Local references checked: {checked_references}",
            f"Broken file references: {len(broken_files)}",
            f"Broken anchor references: {len(broken_anchors)}",
            f"Expected documentation terms missing: {len(missing_terms)}",
            f"Non-empty Doxygen warning lines: {warning_count}",
        )
    )
    for heading, items in (
        ("Broken file references", broken_files),
        ("Broken anchor references", broken_anchors),
        ("Missing expected terms", missing_terms),
    ):
        if items:
            lines.extend(("", heading + ":", *(f"- {item}" for item in items)))

    failed = bool(broken_files or broken_anchors or missing_terms)
    lines.extend(("", "RESULT: " + ("FAIL" if failed else "PASS")))
    report.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
