#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BUILD_ROOT="$ROOT/.build/a1-e001"
NORMAL_BUILD="$BUILD_ROOT/normal"
SANITIZED_BUILD="$BUILD_ROOT/sanitized"
RUN_EVIDENCE_FILE="$BUILD_ROOT/verification.txt"

mkdir -p "$BUILD_ROOT"
: > "$RUN_EVIDENCE_FILE"

exec > >(tee -a "$RUN_EVIDENCE_FILE") 2>&1

on_exit() {
    rc=$?

    if (( rc != 0 )); then
        printf '\nAtmos A1-E001: FAIL (rc=%d)\n' "$rc"
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

printf '=== SisTer Atmos A1-E001 ===\n'
printf 'root=%s\n' "$ROOT"
printf 'timestamp=%s\n\n' "$(date --iso-8601=seconds)"

# ------------------------------------------------------------
# 1. Herança: A0 e baseline científico permanecem verdadeiros
# ------------------------------------------------------------

./scripts/verify_a0_e001.sh

pass "A0 e A0-E001 permanecem íntegros"

# ------------------------------------------------------------
# 2. Constituição arquitetural do A1-E001
# ------------------------------------------------------------

require_file .hoa/atmosphere-a1-e001.yaml
require_file .hoa/technology-radar.yaml
require_file docs/architecture/A-003-cpp23-engineering-policy.md
require_file docs/architecture/A-004-computational-data-horizon.md
require_file specs/domain/native-precipitation-core.md

require_text .hoa/atmosphere-a1-e001.yaml "id: A1-E001"
require_text .hoa/atmosphere-a1-e001.yaml "language: C++23"
require_text .hoa/atmosphere-a1-e001.yaml "strong_domain_types: true"
require_text .hoa/atmosphere-a1-e001.yaml "warnings_as_errors: true"
require_text .hoa/atmosphere-a1-e001.yaml "sanitizer_gate: true"

require_text .hoa/atmosphere-a1-e001.yaml "runtime_python: false"
require_text .hoa/atmosphere-a1-e001.yaml "runtime_legacy: false"
require_text .hoa/atmosphere-a1-e001.yaml "database: false"
require_text .hoa/atmosphere-a1-e001.yaml "h3: false"
require_text .hoa/atmosphere-a1-e001.yaml "http: false"

pass "arquitetura A1-E001 presente"

# ------------------------------------------------------------
# 3. Hardening C++23
# ------------------------------------------------------------

require_file cmake/ProjectOptions.cmake

require_text CMakeLists.txt "set(CMAKE_CXX_STANDARD 23)"
require_text CMakeLists.txt "CMAKE_CXX_EXTENSIONS OFF"

require_text \
    cmake/ProjectOptions.cmake \
    "SISTER_ATMOS_WARNINGS_AS_ERRORS"

require_text \
    cmake/ProjectOptions.cmake \
    "SISTER_ATMOS_ENABLE_SANITIZERS"

require_text \
    cmake/ProjectOptions.cmake \
    "-fsanitize=address,undefined"

require_text \
    cmake/ProjectOptions.cmake \
    "SISTER_ATMOS_SANITIZER_RUNTIME_AVAILABLE"

require_text \
    cmake/ProjectOptions.cmake \
    "-Werror"

pass "hardening C++23 registrado"

# ------------------------------------------------------------
# 4. Núcleo científico
# ------------------------------------------------------------

require_file \
    include/sister/atmos/precipitation/domain_types.hpp

require_file \
    include/sister/atmos/precipitation/native_samples.hpp

require_file \
    include/sister/atmos/precipitation/area_weighting.hpp

require_file src/precipitation/domain_types.cpp
require_file src/precipitation/native_samples.cpp
require_file src/precipitation/area_weighting.cpp

require_text \
    include/sister/atmos/precipitation/domain_types.hpp \
    "class PrecipitationMm final"

require_text \
    include/sister/atmos/precipitation/domain_types.hpp \
    "class RepresentedArea final"

require_text \
    include/sister/atmos/precipitation/domain_types.hpp \
    "struct NativeCellKey final"

require_text \
    include/sister/atmos/precipitation/native_samples.hpp \
    "deduplicate_native_samples"

require_text \
    include/sister/atmos/precipitation/area_weighting.hpp \
    "area_weighted_precipitation"

require_text \
    include/sister/atmos/precipitation/native_samples.hpp \
    "std::expected"

require_text \
    include/sister/atmos/precipitation/native_samples.hpp \
    "std::span"

require_text \
    include/sister/atmos/precipitation/area_weighting.hpp \
    "std::expected"

require_text \
    include/sister/atmos/precipitation/area_weighting.hpp \
    "std::span"

pass "núcleo científico C++23 presente"

# ------------------------------------------------------------
# 5. Golden Cases implementados
# ------------------------------------------------------------

for case_file in \
    specs/golden-cases/GC-001-native-cell-deduplication.json \
    specs/golden-cases/GC-002-conflicting-native-cell.json \
    specs/golden-cases/GC-003-consolidated-area-weighting.json
do
    require_file "$case_file"
done

require_text \
    tests/precipitation_native_samples_tests.cpp \
    "GC-001"

require_text \
    tests/precipitation_native_samples_tests.cpp \
    "GC-002"

require_text \
    tests/precipitation_area_weighting_tests.cpp \
    "GC-003"

pass "GC-001, GC-002 e GC-003 ligados aos testes C++"

# ------------------------------------------------------------
# 6. Domínio não conhece IDs dos Golden Cases
# ------------------------------------------------------------

if grep -RInE \
    'GC-00[123]' \
    include/sister/atmos/precipitation \
    src/precipitation
then
    fail "implementação de domínio contém IDs de Golden Cases"
fi

pass "implementação independente dos IDs de teste"

# ------------------------------------------------------------
# 7. Nenhuma dependência proibida entrou no núcleo
# ------------------------------------------------------------

if grep -RInEi \
    '(libpq|postgres|postgis|pgvector|arrow|parquet|boost/graph|#include.*h3|Sister-Clima|Python\.h)' \
    CMakeLists.txt \
    include/sister/atmos/precipitation \
    src/precipitation
then
    fail "dependência proibida detectada no núcleo A1-E001"
fi

pass "fronteira tecnológica A1-E001 preservada"

# ------------------------------------------------------------
# 8. Ownership explícito
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

if ! grep -Eq '^trial\.mdspan=(AVAILABLE|UNAVAILABLE)$' "$CAPABILITIES_FILE"; then
    fail "estado TRIAL de std::mdspan não foi inventariado"
fi

pass "capabilities C++23 REQUIRED comprovadas"
pass "std::mdspan inventariado como capacidade TRIAL"

cmake \
    --build "$NORMAL_BUILD" \
    --parallel 2 \
    --target \
        sister_atmos_precipitation_core \
        sister_atmos_precipitation_domain_types_tests \
        sister_atmos_precipitation_native_samples_tests \
        sister_atmos_precipitation_area_weighting_tests

ctest \
    --test-dir "$NORMAL_BUILD" \
    --output-on-failure \
    --tests-regex \
    '^sister_atmos_precipitation_(domain_types|native_samples|area_weighting)_tests$'

pass "build normal A1-E001"

# ------------------------------------------------------------
# 9.1 Estado global mutável no artefato compilado
# ------------------------------------------------------------

command -v nm >/dev/null 2>&1 ||
    fail "nm indisponível para auditoria de símbolos"

PRECIPITATION_LIBRARY="$NORMAL_BUILD/libsister_atmos_precipitation_core.a"

[[ -f "$PRECIPITATION_LIBRARY" ]] ||
    fail "biblioteca A1-E001 ausente: $PRECIPITATION_LIBRARY"

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
    fail "estado global/estático mutável detectado no núcleo científico"
fi

unset MUTABLE_DATA_SYMBOLS
unset PRECIPITATION_LIBRARY

pass "sem estado global ou estático mutável no artefato A1"

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
        sister_atmos_precipitation_area_weighting_tests

ASAN_OPTIONS='detect_leaks=1:halt_on_error=1' \
UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1' \
ctest \
    --test-dir "$SANITIZED_BUILD" \
    --output-on-failure \
    --tests-regex \
    '^sister_atmos_precipitation_(domain_types|native_samples|area_weighting)_tests$'

pass "ASan + UBSan A1-E001"

# ------------------------------------------------------------
# 11. A0 continua sem link para o núcleo A1
# ------------------------------------------------------------

if grep -Fq \
    "sister_atmos_precipitation_core" \
    src/main.cpp
then
    fail "executável A0 conhece o núcleo A1"
fi

if grep -Fq \
    "precipitation" \
    include/sister/atmos/identity.hpp
then
    fail "identidade A0 conhece domínio A1"
fi

require_text \
    scripts/verify_a0.sh \
    "sister_atmos_identity_tests"

require_text \
    scripts/verify_a0.sh \
    --tests-regex

pass "A1 não contamina semanticamente o A0"

unset CAPABILITIES_FILE

# ------------------------------------------------------------
# Resultado
# ------------------------------------------------------------

printf '\nAtmos A1-E001: PASS\n'
printf 'Evidência operacional: %s\n' \
    "$RUN_EVIDENCE_FILE"

trap - EXIT
