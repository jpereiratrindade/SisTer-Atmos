#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BUILD_DIR="$ROOT/.build/a0-e001"
RUN_EVIDENCE_FILE="$BUILD_DIR/verification.txt"

mkdir -p "$BUILD_DIR"
: > "$RUN_EVIDENCE_FILE"

exec > >(tee -a "$RUN_EVIDENCE_FILE") 2>&1

on_exit() {
    rc=$?
    if (( rc != 0 )); then
        printf '\nAtmos A0-E001: FAIL (rc=%d)\n' "$rc"
        printf 'Evidência operacional preservada em: %s\n' \
            "$RUN_EVIDENCE_FILE"
    fi
}
trap on_exit EXIT

pass() {
    printf '[PASS] %s\n' "$*"
}

fail() {
    printf '[FAIL] %s\n' "$*" >&2
    exit 1
}

require_file() {
    [[ -f "$1" ]] || fail "arquivo ausente: $1"
}

require_text() {
    local file="$1"
    local text="$2"

    grep -Fq -- "$text" "$file" ||
        fail "texto esperado ausente em $file: $text"
}

printf '=== SisTer Atmos A0-E001 ===\n'
printf 'root=%s\n' "$ROOT"
printf 'timestamp=%s\n\n' "$(date --iso-8601=seconds)"

# ------------------------------------------------------------
# 1. A0 continua íntegro
# ------------------------------------------------------------

./scripts/verify_a0.sh
pass "A0 constitucional permanece íntegro"

# ------------------------------------------------------------
# 2. Constituição do experimento
# ------------------------------------------------------------

require_file .hoa/atmosphere-a0-e001.yaml
require_file docs/experiments/A0-E001-sister-clima-baseline.md
require_file docs/experiments/evidence/a0-e001/oracle-baseline.txt

require_text .hoa/atmosphere-a0-e001.yaml "id: A0-E001"
require_text .hoa/atmosphere-a0-e001.yaml "status: qualified"

require_text \
    .hoa/atmosphere-a0-e001.yaml \
    "scientific_equivalence_in_observed_scope: true"

require_text \
    .hoa/atmosphere-a0-e001.yaml \
    "runtime_dependency_on_legacy: false"

require_text \
    .hoa/atmosphere-a0-e001.yaml \
    "runtime_dependency_on_python: false"

pass "constituição A0-E001"

# ------------------------------------------------------------
# 3. Identidade qualificada do oráculo
# ------------------------------------------------------------

BASELINE="docs/experiments/evidence/a0-e001/oracle-baseline.txt"

require_text "$BASELINE" \
    "release_commit=279ba3e9b084020c23c36e63bec933f1dbc705e6"

require_text "$BASELINE" \
    "observed_head=d8e89bb61da3d54936b17e2f6185f309792ef771"

require_text "$BASELINE" \
    "scientific_diff_v2.5_to_observed_head=none"

require_text "$BASELINE" "test_count=34"
require_text "$BASELINE" "test_result=PASS"
require_text "$BASELINE" "legacy_worktree_changed_by_tests=false"
require_text "$BASELINE" "qualification=scientific_regression_oracle"

pass "oráculo científico qualificado"

# ------------------------------------------------------------
# 4. Invariantes N01-N18
# ------------------------------------------------------------

BASE_INVARIANTS="specs/invariants/precipitation-spatial-contract.yaml"
DERIVED_INVARIANTS="specs/invariants/legacy-derived-invariants.yaml"

require_file "$BASE_INVARIANTS"
require_file "$DERIVED_INVARIANTS"

for n in {01..08}; do
    require_text "$BASE_INVARIANTS" "id: N$n"
done

for n in {09..18}; do
    require_text "$DERIVED_INVARIANTS" "id: N$n"
done

pass "invariantes científicos N01-N18 presentes"

# ------------------------------------------------------------
# 5. Golden Cases
# ------------------------------------------------------------

python3 - <<'PY'
from pathlib import Path
import json
import math
import sys

root = Path("specs/golden-cases")

expected_files = [
    "GC-001-native-cell-deduplication.json",
    "GC-002-conflicting-native-cell.json",
    "GC-003-consolidated-area-weighting.json",
    "GC-004-operational-unique-mean.json",
    "GC-005-point-only-no-surface.json",
    "GC-006-direct-era5-selection.json",
    "GC-007-common-temporal-coverage.json",
    "GC-008-h3-mean-not-spatial-sum.json",
]

expected_ancestry = {
    "GC-001": "test_repeated_native_cell_is_deduplicated",
    "GC-002": "test_divergent_values_for_same_native_cell_are_rejected",
    "GC-003": "test_precipitation_uses_intersection_weighted_mean",
    "GC-004": "test_operational_sampling_uses_mean_and_declares_no_native_surface",
    "GC-005": "test_point_only_product_cannot_generate_h3_surface",
    "GC-006": "test_era5_is_selected_directly_for_precipitation",
    "GC-007": "test_temporal_accumulation_uses_only_dates_common_to_all_cells",
    "GC-008": "test_h3_uses_mean_of_point_accumulations_instead_of_spatial_sum",
}

base_text = Path(
    "specs/invariants/precipitation-spatial-contract.yaml"
).read_text(encoding="utf-8")

derived_text = Path(
    "specs/invariants/legacy-derived-invariants.yaml"
).read_text(encoding="utf-8")

all_invariants = base_text + "\n" + derived_text

observed_ids = []

for filename in expected_files:
    path = root / filename

    if not path.is_file():
        raise SystemExit(f"[FAIL] golden case ausente: {path}")

    data = json.loads(path.read_text(encoding="utf-8"))

    required = {
        "id",
        "title",
        "experiment",
        "ancestral_test",
        "invariants",
        "input",
        "expected",
    }

    missing = required - data.keys()
    if missing:
        raise SystemExit(
            f"[FAIL] {path}: campos ausentes: {sorted(missing)}"
        )

    case_id = data["id"]
    observed_ids.append(case_id)

    if data["experiment"] != "A0-E001":
        raise SystemExit(
            f"[FAIL] {case_id}: experiment != A0-E001"
        )

    if data["ancestral_test"] != expected_ancestry[case_id]:
        raise SystemExit(
            f"[FAIL] {case_id}: ancestral_test inesperado"
        )

    if not data["invariants"]:
        raise SystemExit(
            f"[FAIL] {case_id}: nenhum invariante associado"
        )

    for invariant in data["invariants"]:
        if f"id: {invariant}" not in all_invariants:
            raise SystemExit(
                f"[FAIL] {case_id}: invariante inexistente {invariant}"
            )

expected_ids = [f"GC-{n:03d}" for n in range(1, 9)]

if observed_ids != expected_ids:
    raise SystemExit(
        f"[FAIL] IDs inesperados: {observed_ids}"
    )

# --------------------------------------------------------
# Autoconsistência científica mínima dos fixtures
# --------------------------------------------------------

def load(case_id):
    filename = next(
        p for p in root.glob(f"{case_id}-*.json")
    )
    return json.loads(filename.read_text(encoding="utf-8"))

gc1 = load("GC-001")
samples = gc1["input"]["samples"]

keys = {
    (
        sample["model"],
        sample["native_latitude"],
        sample["native_longitude"],
    )
    for sample in samples
}

if len(keys) != gc1["expected"]["unique_native_cells"]:
    raise SystemExit(
        "[FAIL] GC-001 não demonstra deduplicação"
    )

gc3 = load("GC-003")
contrib = gc3["input"]["contributions"]

weighted = sum(
    item["precipitation_mm"] * item["represented_area"]
    for item in contrib
) / sum(
    item["represented_area"]
    for item in contrib
)

expected = gc3["expected"]["precipitation_mm"]
tolerance = gc3["expected"]["tolerance"]

if not math.isclose(weighted, expected, abs_tol=tolerance):
    raise SystemExit(
        f"[FAIL] GC-003 inconsistente: {weighted} != {expected}"
    )

gc4 = load("GC-004")

operational_mean = sum(
    item["total_mm"]
    for item in gc4["input"]["samples"]
) / len(gc4["input"]["samples"])

if not math.isclose(
    operational_mean,
    gc4["expected"]["precipitation_mm"],
):
    raise SystemExit(
        "[FAIL] GC-004 média operacional inconsistente"
    )

gc7 = load("GC-007")

common_dates = None

for cell in gc7["input"]["native_cells"]:
    available = {
        date
        for date, value in cell["daily_mm"].items()
        if value is not None
    }

    common_dates = (
        available
        if common_dates is None
        else common_dates & available
    )

if len(common_dates) != gc7["expected"]["available_days"]:
    raise SystemExit(
        "[FAIL] GC-007 cobertura temporal inconsistente"
    )

gc8 = load("GC-008")
values = gc8["input"]["point_accumulations_mm"]

mean_value = sum(values) / len(values)
sum_value = sum(values)

if not math.isclose(
    mean_value,
    gc8["expected"]["mean_accumulation_mm"],
):
    raise SystemExit(
        "[FAIL] GC-008 média espacial inconsistente"
    )

if not math.isclose(
    sum_value,
    gc8["expected"]["forbidden_spatial_sum_mm"],
):
    raise SystemExit(
        "[FAIL] GC-008 soma proibida não representa o fixture"
    )

if math.isclose(mean_value, sum_value):
    raise SystemExit(
        "[FAIL] GC-008 não distingue média de soma"
    )

for case_id in expected_ids:
    print(f"[PASS] {case_id}")

print("[PASS] golden cases estrutural e semanticamente consistentes")
PY

pass "GC-001–GC-008 válidos"

# ------------------------------------------------------------
# 6. Independência da implementação Python
# ------------------------------------------------------------

if grep -RInE \
    --include='GC-*.json' \
    '(/Sister-Clima/|\.py["'\'']|python[0-9]*|venv/)' \
    specs/golden-cases
then
    fail "golden case contém dependência de implementação Python"
fi

pass "golden cases independentes da implementação Python"

# ------------------------------------------------------------
# Resultado
# ------------------------------------------------------------

printf '\nAtmos A0-E001: PASS\n'
printf 'Evidência operacional: %s\n' "$RUN_EVIDENCE_FILE"

trap - EXIT
