#!/usr/bin/env python3
"""Check the hand-written Doxygen references before generation."""

from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PAGE = ROOT / "docs" / "index.md"
SOURCE = ROOT / "src"


def main() -> int:
    page = PAGE.read_text(encoding="utf-8")
    source = "\n".join(
        path.read_text(encoding="utf-8-sig", errors="replace")
        for path in SOURCE.rglob("*")
        if path.suffix.lower() in {".h", ".hpp", ".cpp", ".cc", ".cxx"}
    )

    failures: list[str] = []
    if "](@ref" in page:
        failures.append("legacy Markdown/Doxygen hybrid link syntax remains")

    anchors = re.findall(r"@anchor\s+([A-Za-z_][A-Za-z0-9_]*)", page)
    duplicates = sorted({name for name in anchors if anchors.count(name) > 1})
    if duplicates:
        failures.append("duplicate anchors: " + ", ".join(duplicates))

    references = re.findall(r"@ref\s+([^\s\"]+)", page)
    for reference in references:
        if reference in anchors:
            continue
        base = reference.split("::", 1)[0]
        if not re.search(rf"\b(?:class|struct|enum(?:\s+class)?)\s+{re.escape(base)}\b", source):
            failures.append(f"unrecognised referenced type: {reference}")
            continue
        if "::" in reference:
            member = reference.split("::", 1)[1].split("(", 1)[0]
            if not re.search(rf"\b{re.escape(member)}\s*\(", source):
                failures.append(f"unrecognised referenced member: {reference}")

    print(f"Checked {len(anchors)} anchors and {len(references)} Doxygen references.")
    if failures:
        for failure in failures:
            print(f"ERROR: {failure}")
        return 1
    print("Source-level Doxygen reference checks passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
