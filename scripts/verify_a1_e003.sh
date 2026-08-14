#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BUILD_ROOT="$ROOT/.build/a1-e003"
NORMAL_BUILD="$BUILD_ROOT/normal"
SANITIZED_BUILD="$BUILD_ROOT/sanitized"
RUN_EVIDENCE_FILE="$BUILD_ROOT/verification.txt"

mkdir -p "$BUILD_ROOT"
: > "$RUN_EVIDENCE_FILE"

exec > >(tee -a "$RUN_EVIDENCE_FILE") 2>&1

on_exit() {
    rc=$?

    if (( rc != 0 )); then
        printf '\nAtmos A1-E003: FAIL (rc=%d)\n' "$rc"
        printf 'Evidência operacional: %s\n' \
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

printf '=== SisTer Atmos A1-E003 ===\n'
printf 'root=%s\n' "$ROOT"
printf 'timestamp=%s\n\n' "$(date --iso-8601=seconds)"

# ------------------------------------------------------------
# 1. Herança governada
# ------------------------------------------------------------

# A cadeia constitucional até A1-E001 continua reproduzível.
./scripts/verify_a1_e001.sh

pass "A1-E001 e baselines anteriores permanecem íntegros"

# A1-E002 já é um baseline histórico verificado. Seu gate original
# é sensível à posição canônica em que A1-E002 era o último marco;
# portanto, após promoção de um sucessor, preservamos sua prova por
# manifesto + evidência governada + checksum + ancestralidade.
A1_E002_MANIFEST=".hoa/atmosphere-a1-e002.yaml"
A1_E002_EVIDENCE="docs/experiments/evidence/a1-e002/verification.txt"
A1_E002_CHECKSUM="docs/experiments/evidence/a1-e002/verification.sha256"
A1_E002_VERIFIED_COMMIT="9a321fb"

require_file "$A1_E002_MANIFEST"
require_file "$A1_E002_EVIDENCE"
require_file "$A1_E002_CHECKSUM"

A1_E002_STATUS="$(
    sed -n \
        '/^experiment:/,/^[^[:space:]]/ {
            s/^  status:[[:space:]]*//p
        }' \
        "$A1_E002_MANIFEST" |
        head -n1
)"

[[ "$A1_E002_STATUS" == "verified" ]] ||
    fail "baseline A1-E002 não está verified"

require_text     "$A1_E002_EVIDENCE"     "Atmos A1-E002: PASS"

A1_E002_EXPECTED_HASH="$(
    awk 'NR == 1 { print $1 }' "$A1_E002_CHECKSUM"
)"

A1_E002_OBSERVED_HASH="$(
    sha256sum "$A1_E002_EVIDENCE" |
        awk '{ print $1 }'
)"

[[ "$A1_E002_EXPECTED_HASH" == "$A1_E002_OBSERVED_HASH" ]] ||
    fail "checksum da evidência governada A1-E002 inválido"

git merge-base     --is-ancestor     "$A1_E002_VERIFIED_COMMIT"     HEAD ||
    fail "commit verificado A1-E002 saiu da ancestralidade"

pass "baseline governado A1-E002 preservado"

unset A1_E002_MANIFEST
unset A1_E002_EVIDENCE
unset A1_E002_CHECKSUM
unset A1_E002_VERIFIED_COMMIT
unset A1_E002_STATUS
unset A1_E002_EXPECTED_HASH
unset A1_E002_OBSERVED_HASH

# ------------------------------------------------------------
# 2. Constituição e autorização
# ------------------------------------------------------------

require_file .hoa/atmosphere-a1-e003.yaml
require_file specs/domain/spatial-representation-operational-sampling.md
require_file \
    docs/experiments/A1-E003-spatial-representation-operational-sampling.md

require_text .hoa/atmosphere-a1-e003.yaml "id: A1-E003"
require_text .hoa/atmosphere-a1-e003.yaml "authorized: true"

if ! grep -Eq \
    '^  status: (active|verified)$' \
    .hoa/atmosphere-a1-e003.yaml
then
    fail "A1-E003 deve estar active ou verified"
fi

for gc in GC-004 GC-005 GC-008; do
    require_text .hoa/atmosphere-a1-e003.yaml "$gc"
done

for invariant in N02 N05 N06 N09 N10 N16 N18; do
    require_text .hoa/atmosphere-a1-e003.yaml "$invariant"
    require_text \
        specs/domain/spatial-representation-operational-sampling.md \
        "$invariant"
done

# Estado corrente permitido:
# - antes da promoção: A1-E003 é next_milestone active;
# - depois da promoção: A1-E003 é last_verified_milestone.
if grep -Fq \
    "next_milestone: A1-E003" \
    .hoa/project-state.yaml
then
    require_text .hoa/project-state.yaml \
        "next_milestone_status: active"

    require_text .hoa/project-state.yaml \
        "next_milestone_authorized: true"

elif grep -Fq \
    "last_verified_milestone: A1-E003" \
    .hoa/project-state.yaml
then
    require_text .hoa/project-state.yaml \
        "last_verified_gate: scripts/verify_a1_e003.sh"

else
    fail "estado canônico não posiciona A1-E003 no ciclo de vida esperado"
fi

if ! grep -Eq \
    '^[[:space:]]+(future_gate|gate): scripts/verify_a1_e003\.sh$' \
    .hoa/atmosphere-a1-e003.yaml
then
    fail "manifesto A1-E003 não referencia seu gate"
fi

pass "A1-E003 constituído, autorizado e governável no ciclo de vida"

# ------------------------------------------------------------
# 3. Fronteira científica e tecnológica
# ------------------------------------------------------------

for token in \
    "runtime_python: false" \
    "runtime_legacy: false" \
    "providers_real: false" \
    "h3_library: false" \
    "http: false" \
    "database: false" \
    "nexo: false" \
    "frontend: false"
do
    require_text .hoa/atmosphere-a1-e003.yaml "$token"
done

require_text .hoa/atmosphere-a1-e003.yaml \
    "h3_runtime_required_for_semantic_test: false"

require_text .hoa/atmosphere-a1-e003.yaml \
    "sampling_resolution_distinct_from_presentation_resolution: true"

require_text .hoa/atmosphere-a1-e003.yaml \
    "adaptive_sampling_provenance_explicit: true"

require_text .hoa/atmosphere-a1-e003.yaml \
    "core_inputs_are_canonical: true"

require_text .hoa/atmosphere-a1-e003.yaml \
    "provider_normalization_outside_core: true"

require_text .hoa/atmosphere-a1-e003.yaml \
    "fuzzy_coordinate_identity_in_core: false"

if grep -RInEi \
    '(libpq|postgres|postgis|pgvector|arrow|parquet|boost/graph|#include.*h3|Python\.h|Sister-Clima)' \
    include/sister/atmos/precipitation \
    src/precipitation \
    CMakeLists.txt
then
    fail "dependência proibida detectada no núcleo A1-E003"
fi

pass "fronteira tecnológica A1-E003 preservada"

# ------------------------------------------------------------
# 4. GC-004 — Operational Sampling
# ------------------------------------------------------------

require_file \
    include/sister/atmos/precipitation/operational_sampling.hpp
require_file \
    src/precipitation/operational_sampling.cpp
require_file \
    tests/operational_sampling_tests.cpp

require_text \
    include/sister/atmos/precipitation/operational_sampling.hpp \
    "operational_sampling_mean"

require_text \
    include/sister/atmos/precipitation/operational_sampling.hpp \
    "unweighted_mean_unique_sample_points"

require_text \
    include/sister/atmos/precipitation/operational_sampling.hpp \
    "operational_sampling_not_native_pixel_surface"

require_text \
    tests/operational_sampling_tests.cpp \
    "GC-004"

require_text \
    tests/operational_sampling_tests.cpp \
    "precipitação média = 20 mm"

pass "GC-004 ligado ao núcleo nativo C++23"

# ------------------------------------------------------------
# 5. GC-005 — Surface Eligibility
# ------------------------------------------------------------

require_file \
    include/sister/atmos/precipitation/surface_eligibility.hpp
require_file \
    src/precipitation/surface_eligibility.cpp
require_file \
    tests/surface_eligibility_tests.cpp

require_text \
    include/sister/atmos/precipitation/surface_eligibility.hpp \
    "native_geometry_required"

require_text \
    include/sister/atmos/precipitation/surface_eligibility.hpp \
    "validate_spatial_representation"

require_text \
    tests/surface_eligibility_tests.cpp \
    "GC-005"

require_text \
    tests/surface_eligibility_tests.cpp \
    "rejeita superfície sem geometria nativa"

pass "GC-005 ligado ao núcleo nativo C++23"

# ------------------------------------------------------------
# 6. GC-008 — Presentation Aggregation
# ------------------------------------------------------------

require_file \
    include/sister/atmos/precipitation/presentation_aggregation.hpp
require_file \
    src/precipitation/presentation_aggregation.cpp
require_file \
    tests/presentation_aggregation_tests.cpp

require_text \
    include/sister/atmos/precipitation/presentation_aggregation.hpp \
    "PresentationBucketId"

require_text \
    include/sister/atmos/precipitation/presentation_aggregation.hpp \
    "spatial_mean"

require_text \
    include/sister/atmos/precipitation/presentation_aggregation.hpp \
    "aggregate_presentation_bucket"

require_text \
    tests/presentation_aggregation_tests.cpp \
    "GC-008"

require_text \
    tests/presentation_aggregation_tests.cpp \
    "média espacial = 20 mm"

require_text \
    tests/presentation_aggregation_tests.cpp \
    "soma espacial de 40 mm é proibida"

pass "GC-008 ligado ao núcleo nativo C++23"

# ------------------------------------------------------------
# 7. Golden Cases pertencem à prova, não ao algoritmo
# ------------------------------------------------------------

if grep -RInE \
    'GC-004|GC-005|GC-008' \
    include/sister/atmos/precipitation \
    src/precipitation
then
    fail "implementação contém IDs de Golden Cases A1-E003"
fi

pass "implementação independente dos IDs dos Golden Cases"

# ------------------------------------------------------------
# 8. Ownership manual
# ------------------------------------------------------------

if grep -RInE \
    '(^|[^[:alnum:]_])(new|delete)[[:space:]\[]' \
    include/sister/atmos/precipitation \
    src/precipitation
then
    fail "ownership manual detectado no núcleo científico"
fi

pass "sem ownership manual"

# ------------------------------------------------------------
# 9. Build normal rigoroso
# ------------------------------------------------------------

rm -rf "$NORMAL_BUILD"

cmake \
    -S . \
    -B "$NORMAL_BUILD" \
    -DBUILD_TESTING=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSISTER_ATMOS_WARNINGS_AS_ERRORS=ON \
    -DSISTER_ATMOS_ENABLE_SANITIZERS=OFF \
    -DSISTER_ATMOS_ENABLE_PRECIPITATION_CORE=ON

CAPABILITIES_FILE="$NORMAL_BUILD/sister-atmos-cxx23-capabilities.txt"

require_file "$CAPABILITIES_FILE"
require_text "$CAPABILITIES_FILE" "required.language_cxx23=PASS"
require_text "$CAPABILITIES_FILE" "required.expected=PASS"
require_text "$CAPABILITIES_FILE" "required.span=PASS"
require_text "$CAPABILITIES_FILE" "required.to_underlying=PASS"

pass "capacidades C++23 REQUIRED comprovadas"

cmake \
    --build "$NORMAL_BUILD" \
    --parallel 2 \
    --target \
        sister_atmos_precipitation_core \
        sister_atmos_precipitation_domain_types_tests \
        sister_atmos_precipitation_native_samples_tests \
        sister_atmos_precipitation_area_weighting_tests \
        sister_atmos_product_selection_tests \
        sister_atmos_temporal_coverage_tests \
        sister_atmos_operational_sampling_tests \
        sister_atmos_surface_eligibility_tests \
        sister_atmos_presentation_aggregation_tests

ctest \
    --test-dir "$NORMAL_BUILD" \
    --output-on-failure \
    --tests-regex \
    '^sister_atmos_(precipitation_(domain_types|native_samples|area_weighting)|product_selection|temporal_coverage|operational_sampling|surface_eligibility|presentation_aggregation)_tests$'

pass "build normal e regressão integral do núcleo A1"

# ------------------------------------------------------------
# 10. Estado global/estático mutável
# ------------------------------------------------------------

command -v nm >/dev/null 2>&1 ||
    fail "nm indisponível para auditoria de símbolos"

PRECIPITATION_LIBRARY="$NORMAL_BUILD/libsister_atmos_precipitation_core.a"

[[ -f "$PRECIPITATION_LIBRARY" ]] ||
    fail "biblioteca A1-E003 ausente"

MUTABLE_DATA_SYMBOLS="$(
    nm -C --defined-only "$PRECIPITATION_LIBRARY" |
        awk '
            NF >= 3 &&
            $2 ~ /^[BbCcDdGgSs]$/ {
                print
            }
        '
)"

if [[ -n "$MUTABLE_DATA_SYMBOLS" ]]; then
    printf '%s\n' "$MUTABLE_DATA_SYMBOLS" >&2
    fail "estado global/estático mutável detectado"
fi

unset MUTABLE_DATA_SYMBOLS
unset PRECIPITATION_LIBRARY

pass "sem estado global ou estático mutável"

# ------------------------------------------------------------
# 11. ASan + UBSan
# ------------------------------------------------------------

rm -rf "$SANITIZED_BUILD"

cmake \
    -S . \
    -B "$SANITIZED_BUILD" \
    -DBUILD_TESTING=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSISTER_ATMOS_WARNINGS_AS_ERRORS=ON \
    -DSISTER_ATMOS_ENABLE_SANITIZERS=ON \
    -DSISTER_ATMOS_ENABLE_PRECIPITATION_CORE=ON

cmake \
    --build "$SANITIZED_BUILD" \
    --parallel 2 \
    --target \
        sister_atmos_precipitation_core \
        sister_atmos_precipitation_domain_types_tests \
        sister_atmos_precipitation_native_samples_tests \
        sister_atmos_precipitation_area_weighting_tests \
        sister_atmos_product_selection_tests \
        sister_atmos_temporal_coverage_tests \
        sister_atmos_operational_sampling_tests \
        sister_atmos_surface_eligibility_tests \
        sister_atmos_presentation_aggregation_tests

ASAN_OPTIONS='detect_leaks=1:halt_on_error=1' \
UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1' \
ctest \
    --test-dir "$SANITIZED_BUILD" \
    --output-on-failure \
    --tests-regex \
    '^sister_atmos_(precipitation_(domain_types|native_samples|area_weighting)|product_selection|temporal_coverage|operational_sampling|surface_eligibility|presentation_aggregation)_tests$'

pass "ASan + UBSan e regressão integral do núcleo A1"

# ------------------------------------------------------------
# 12. A0 permanece isolado
# ------------------------------------------------------------

if grep -Fq \
    "sister_atmos_precipitation_core" \
    src/main.cpp
then
    fail "executável A0 conhece núcleo A1"
fi

if grep -Fq \
    "precipitation" \
    include/sister/atmos/identity.hpp
then
    fail "identidade A0 conhece domínio A1"
fi

pass "A1-E003 não contamina semanticamente A0"

# ------------------------------------------------------------
# 13. Fronteira da prova permanece explícita
# ------------------------------------------------------------

require_text \
    specs/domain/spatial-representation-operational-sampling.md \
    "A1-E003 não inclui:"

require_text \
    specs/domain/spatial-representation-operational-sampling.md \
    "biblioteca H3 de runtime"

require_text \
    specs/domain/spatial-representation-operational-sampling.md \
    "frontend"

require_text \
    specs/domain/spatial-representation-operational-sampling.md \
    "normalização entre coordenadas provenientes de providers diferentes"

require_text \
    .hoa/atmosphere-a1-e003.yaml \
    "A1-E003 prova semântica espacial; não prova provider, H3 runtime, API ou frontend"

pass "A1-E003 não reivindica ciência ou produto fora da evidência"

unset CAPABILITIES_FILE

printf '\nAtmos A1-E003: PASS\n'
printf 'Evidência operacional: %s\n' \
    "$RUN_EVIDENCE_FILE"

trap - EXIT
