#!/usr/bin/env python3
"""Verify the source-level invariants of the 2026-08-21 controlled amendment."""

from __future__ import annotations

import csv
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OSMIA_CPP = (ROOT / "src/Osmia/Osmia.cpp").read_text(encoding="utf-8")
OSMIA_H = (ROOT / "src/Osmia/Osmia.h").read_text(encoding="utf-8")
MANAGER_CPP = (ROOT / "src/Osmia/Osmia_Population_Manager.cpp").read_text(encoding="utf-8")
SOURCE = OSMIA_CPP + OSMIA_H + MANAGER_CPP


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FAIL: {message}")


require(
    OSMIA_CPP.count("m_EggsThisNest = PlanEggsPerNest() - (2 * m_NestOrder);") == 1,
    "the per-nest draw and nest-order reduction are not present exactly once",
)
require(OSMIA_CPP.count("m_NestOrder = 0;") == 1, "nest order is not initialised exactly once")
require(OSMIA_CPP.count("m_NestOrder++;") == 1, "nest order is not incremented exactly once")
require(
    OSMIA_CPP.count("if (g_rand_uni_fnc() > 0.55) shift = 2;") == 1,
    "the calibrated 0.45 two-egg shift was changed or removed",
)
require(
    "m_EggsThisNest = PlanEggsPerNest() + 2;" not in OSMIA_H,
    "PlanEggsPerNest is still called during female initialisation",
)

removed_symbols = (
    "OsmiaForageMask",
    "OsmiaForageMaskDetailed",
    "OSMIA_DETAILEDMASKSTEP",
    "OSMIA_FORAGESTEPS",
    "OSMIA_FORAGEMASKSTEPSZ",
)
for symbol in removed_symbols:
    require(
        re.search(rf"\b{re.escape(symbol)}\b", SOURCE) is None,
        f"removed legacy symbol remains in source: {symbol}",
    )

ledger = ROOT / "evidence/parameter_ledger_step3.csv"
with ledger.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))
keys = [row["config_key"] for row in rows]
variables = [row["configuration_variable"] for row in rows]
require(len(rows) == 100, f"parameter ledger contains {len(rows)} rows rather than 100")
require(len(set(keys)) == 100, "parameter ledger configuration keys are not unique")
require(len(set(variables)) == 100, "parameter ledger variable names are not unique")
for variable in variables:
    require(variable in SOURCE, f"ledger variable is absent from the amended source: {variable}")
for symbol in removed_symbols[2:]:
    require(symbol not in keys, f"removed configuration key remains in the ledger: {symbol}")

print("Controlled amendment source checks passed.")
print("Per-nest planning: fresh calibrated draw, reduced by two eggs per preceding nest.")
print("Legacy forage masks: both classes and three mask-only configuration keys removed.")
print("Parameter ledger: 100 unique keys.")
