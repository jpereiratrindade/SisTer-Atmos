#!/usr/bin/env python3
"""Static frontend wiring contract for the dependency-free Atmos UI."""

from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
html = (root / "web/index.html").read_text(encoding="utf-8")
javascript = (root / "web/atmos.js").read_text(encoding="utf-8")

html_ids = set(re.findall(r'\bid="([^"]+)"', html))
literal_id_selectors = set(re.findall(r"\$\('#([A-Za-z0-9_-]+)'", javascript))
missing = sorted(literal_id_selectors - html_ids)
if missing:
    print(f"[FAIL] seletores JavaScript sem elemento HTML: {', '.join(missing)}", file=sys.stderr)
    raise SystemExit(1)

views = set(re.findall(r'\bid="view-([^"]+)"', html))
links = set(re.findall(r'\bdata-view-link="([^"]+)"', html))
if views != links:
    print(f"[FAIL] views e navegação divergem: views={sorted(views)} links={sorted(links)}", file=sys.stderr)
    raise SystemExit(1)

required_assets = {"/assets/atmos.css", "/assets/atmos-overrides.css", "/assets/atmos.js"}
missing_assets = sorted(asset for asset in required_assets if asset not in html)
if missing_assets:
    print(f"[FAIL] assets ausentes no HTML: {', '.join(missing_assets)}", file=sys.stderr)
    raise SystemExit(1)

if "/assets/rs-territories.csv" not in javascript:
    print("[FAIL] catálogo territorial RS não é carregado pelo frontend", file=sys.stderr)
    raise SystemExit(1)

print("[PASS] frontend Atmos: seletores, views e assets consistentes")
