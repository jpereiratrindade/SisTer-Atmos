#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BUILD_ROOT="$ROOT/.build/a1-e002"
NORMAL_BUILD="$BUILD_ROOT/normal"
SANITIZED_BUILD="$BUILD_ROOT/sanitized"
RUN_EVIDENCE_FILE="$BUILD_ROOT/verification.txt"

mkdir -p "$BUILD_ROOT"
: > "$RUN_EVIDENCE_FILE"

exec > >(tee -a "$RUN_EVIDENCE_FILE") 2>&1

on_exit() {
    rc=$?

    if (( rc != 0 )); then
        printf '\nAtmos A1-E002: FAIL (rc=%d)\n' "$rc"
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

printf '=== SisTer Atmos A1-E002 ===\n'
printf 'root=%s\n' "$ROOT"
printf 'timestamp=%s\n\n' "$(date --iso-8601=seconds)"

# ------------------------------------------------------------
# 1. Herança: A1-E001 permanece verdadeiro
# ------------------------------------------------------------

./scripts/verify_a1_e001.sh

pass "A1-E001 permanece íntegro"

# ------------------------------------------------------------
# 2. Constituição, autorização e ciclo de vida
# ------------------------------------------------------------

require_file .hoa/atmosphere-a1-e002.yaml
require_file specs/domain/product-selection-temporal-coverage.md
require_file docs/experiments/A1-E002-product-selection-temporal-coverage.md

require_text .hoa/atmosphere-a1-e002.yaml "id: A1-E002"
require_text .hoa/atmosphere-a1-e002.yaml "authorized: true"
require_text .hoa/atmosphere-a1-e002.yaml "GC-006"
require_text .hoa/atmosphere-a1-e002.yaml "GC-007"

for invariant in N04 N07 N08 N12 N17; do
    require_text .hoa/atmosphere-a1-e002.yaml "$invariant"
done

EXPERIMENT_STATUS="$(
    sed -n         '/^experiment:/,/^[^[:space:]]/ {
            s/^  status:[[:space:]]*//p
        }'         .hoa/atmosphere-a1-e002.yaml |
        head -n1
)"

[[ -n "$EXPERIMENT_STATUS" ]] ||
    fail "status de A1-E002 ausente"

case "$EXPERIMENT_STATUS" in
    active)
        require_text .hoa/project-state.yaml             "next_milestone: A1-E002"

        require_text .hoa/project-state.yaml             "next_milestone_status: active"

        require_text .hoa/project-state.yaml             "next_milestone_authorized: true"

        pass "A1-E002 constituído, autorizado e ativo"
        ;;

    verified)
        require_text .hoa/project-state.yaml             "last_verified_milestone: A1-E002"

        require_text .hoa/project-state.yaml             "last_verified_gate: scripts/verify_a1_e002.sh"

        pass "A1-E002 registrado como marco verificado"
        ;;

    *)
        fail             "estado de ciclo de vida A1-E002 inválido: $EXPERIMENT_STATUS"
        ;;
esac

# ------------------------------------------------------------
# 3. Fronteiras científicas e tecnológicas
# ------------------------------------------------------------

for token in \
    "runtime_python: false" \
    "runtime_legacy: false" \
    "providers_real: false" \
    "h3: false" \
    "http: false" \
    "database: false" \
    "nexo: false" \
    "frontend: false"
do
    require_text .hoa/atmosphere-a1-e002.yaml "$token"
done

if grep -RInEi \
    '(libpq|postgres|postgis|pgvector|arrow|parquet|boost/graph|#include.*h3|Python\.h|Sister-Clima)' \
    include/sister/atmos/precipitation \
    src/precipitation \
    CMakeLists.txt
then
    fail "dependência proibida detectada no núcleo A1-E002"
fi

pass "fronteira tecnológica A1-E002 preservada"

# ------------------------------------------------------------
# 4. Núcleo Product Selection
# ------------------------------------------------------------

require_file \
    include/sister/atmos/precipitation/product_selection.hpp

require_file \
    src/precipitation/product_selection.cpp

require_file \
    tests/product_selection_tests.cpp

require_text \
    include/sister/atmos/precipitation/product_selection.hpp \
    "struct ProductSelectionRequest final"

require_text \
    include/sister/atmos/precipitation/product_selection.hpp \
    "MeteorologicalProduct requested_product"

require_text \
    include/sister/atmos/precipitation/product_selection.hpp \
    "MeteorologicalProduct used_product"

require_text \
    include/sister/atmos/precipitation/product_selection.hpp \
    "ProductSelectionSemantics"

require_text \
    src/precipitation/product_selection.cpp \
    "request.requested_product"

require_text \
    tests/product_selection_tests.cpp \
    "GC-006"

require_text \
    tests/product_selection_tests.cpp \
    "direct_product_selection"

require_text \
    tests/product_selection_tests.cpp \
    "ungoverned_requested_product"

require_text \
    tests/product_selection_tests.cpp \
    "ungoverned_era5_land_precipitation_path"

pass "GC-006 ligado ao núcleo nativo C++23"

# ------------------------------------------------------------
# 5. Núcleo Temporal Coverage
# ------------------------------------------------------------

require_file \
    include/sister/atmos/precipitation/temporal_coverage.hpp

require_file \
    src/precipitation/temporal_coverage.cpp

require_file \
    tests/temporal_coverage_tests.cpp

require_text \
    include/sister/atmos/precipitation/temporal_coverage.hpp \
    "class CalendarDate final"

require_text \
    include/sister/atmos/precipitation/temporal_coverage.hpp \
    "class TemporalPeriod final"

require_text \
    include/sister/atmos/precipitation/temporal_coverage.hpp \
    "struct AnalyticalTemporalContext final"

require_text \
    include/sister/atmos/precipitation/temporal_coverage.hpp \
    "common_temporal_coverage"

require_text \
    tests/temporal_coverage_tests.cpp \
    "GC-007"

require_text \
    tests/temporal_coverage_tests.cpp \
    "célula A acumula 3 mm"

require_text \
    tests/temporal_coverage_tests.cpp \
    "célula B acumula apenas 30 mm"

require_text \
    tests/temporal_coverage_tests.cpp \
    "03/07 não contamina acumulação de B"

require_text \
    tests/temporal_coverage_tests.cpp \
    "non_contiguous_common_temporal_coverage"

pass "GC-007 ligado ao núcleo nativo C++23"

# ------------------------------------------------------------
# 6. Golden Cases são testes, não algoritmo
# ------------------------------------------------------------

if grep -RInE \
    'GC-006|GC-007' \
    include/sister/atmos/precipitation \
    src/precipitation
then
    fail "implementação contém IDs de Golden Cases A1-E002"
fi

pass "implementação independente de IDs dos Golden Cases"

# ------------------------------------------------------------
# 7. Ownership e estado global
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
# 8. Build normal rigoroso
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
        sister_atmos_temporal_coverage_tests

ctest \
    --test-dir "$NORMAL_BUILD" \
    --output-on-failure \
    --tests-regex \
    '^sister_atmos_(precipitation_(domain_types|native_samples|area_weighting)|product_selection|temporal_coverage)_tests$'

pass "build normal A1-E002"

# ------------------------------------------------------------
# 9. Estado global/estático mutável
# ------------------------------------------------------------

command -v nm >/dev/null 2>&1 ||
    fail "nm indisponível para auditoria de símbolos"

PRECIPITATION_LIBRARY="$NORMAL_BUILD/libsister_atmos_precipitation_core.a"

[[ -f "$PRECIPITATION_LIBRARY" ]] ||
    fail "biblioteca A1-E002 ausente"

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
# 10. Build ASan + UBSan
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
        sister_atmos_temporal_coverage_tests

ASAN_OPTIONS='detect_leaks=1:halt_on_error=1' \
UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1' \
ctest \
    --test-dir "$SANITIZED_BUILD" \
    --output-on-failure \
    --tests-regex \
    '^sister_atmos_(precipitation_(domain_types|native_samples|area_weighting)|product_selection|temporal_coverage)_tests$'

pass "ASan + UBSan A1-E002"

# ------------------------------------------------------------
# 11. A0 permanece isolado
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

pass "A1-E002 não contamina semanticamente A0"

# ------------------------------------------------------------
# 12. Limitações declaradas permanecem verdadeiras
# ------------------------------------------------------------

require_text \
    .hoa/atmosphere-a1-e002.yaml \
    "positive_fallback_verified_by_gc006: false"

require_text \
    .hoa/atmosphere-a1-e002.yaml \
    "analytical_views_implemented_here: false"

require_text \
    docs/experiments/A1-E002-product-selection-temporal-coverage.md \
    "não comprova um caminho positivo"

pass "A1-E002 não reivindica ciência fora da evidência"

unset CAPABILITIES_FILE

printf '\nAtmos A1-E002: PASS\n'
printf 'Evidência operacional: %s\n' \
    "$RUN_EVIDENCE_FILE"

trap - EXIT
