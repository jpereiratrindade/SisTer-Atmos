#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

STATE=".hoa/project-state.yaml"
[[ -f "$STATE" ]] || { echo "[FAIL] estado canônico ausente: $STATE" >&2; exit 1; }

GATE="$(sed -n 's/^last_verified_gate:[[:space:]]*//p' "$STATE" | head -n1)"
MILESTONE="$(sed -n 's/^last_verified_milestone:[[:space:]]*//p' "$STATE" | head -n1)"

[[ -n "$GATE" ]] || { echo "[FAIL] last_verified_gate ausente" >&2; exit 1; }
[[ -x "$GATE" ]] || { echo "[FAIL] gate não executável: $GATE" >&2; exit 1; }

printf '=== Verificando estado corrente do SisTer Atmos ===\n'
printf 'último marco=%s\n' "$MILESTONE"
printf 'gate=%s\n\n' "$GATE"

"./$GATE"
./scripts/verify_project_governance.py

printf '\nAtmos current state: PASS\n'
