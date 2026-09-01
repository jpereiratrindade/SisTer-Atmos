#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${ATMOS_BUILD_DIR:-$ROOT/build}"

cmake -S "$ROOT" -B "$BUILD" -G Ninja
cmake --build "$BUILD"
ctest --test-dir "$BUILD" --output-on-failure

rg -q 'authorized: true' "$ROOT/.hoa/atmosphere-a2-mvp-001.yaml"
rg -q 'location_search' "$ROOT/contracts/atmosphere.openapi.yaml"
rg -q 'Explorador climático' "$ROOT/web/index.html"
rg -q 'Operação territorial RS' "$ROOT/web/index.html"
rg -q 'Território e distribuição H3' "$ROOT/web/index.html"
rg -q 'rs_territorial_units.csv' "$ROOT/CMakeLists.txt"
node --check "$ROOT/web/atmos.js"
python3 "$ROOT/tests/frontend_contract_test.py"

echo "Atmos A2-MVP-001: PASS"
