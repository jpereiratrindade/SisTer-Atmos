#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BUILD_DIR="$ROOT/.build/a0"
RUN_EVIDENCE_FILE="$BUILD_DIR/verification.txt"

mkdir -p "$BUILD_DIR"
: > "$RUN_EVIDENCE_FILE"

exec > >(tee -a "$RUN_EVIDENCE_FILE") 2>&1

on_exit() {
  rc=$?
  if (( rc != 0 )); then
    printf '\nAtmos A0: FAIL (rc=%d)\n' "$rc"
    printf 'Evidência operacional preservada em: %s\n' "$RUN_EVIDENCE_FILE"
  fi
}
trap on_exit EXIT

pass() { printf '[PASS] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

require_file() {
  [[ -f "$1" ]] || fail "arquivo ausente: $1"
}

require_dir() {
  [[ -d "$1" ]] || fail "diretório ausente: $1"
}

require_text() {
  local file="$1"
  local text="$2"
  grep -Fq -- "$text" "$file" || fail "texto esperado ausente em $file: $text"
}

printf '=== SisTer Atmos A0 ===\n'
printf 'root=%s\n' "$ROOT"
printf 'timestamp=%s\n\n' "$(date --iso-8601=seconds)"

require_file .hoa/project.yaml
require_file contracts/system_manifest.json
require_text .hoa/project.yaml "id: sister_atmos"
require_text contracts/system_manifest.json '"system_id": "sister_atmos"'
pass "identidade"

for dir in domain specs/domain specs/contracts specs/invariants docs/architecture docs/lineage contracts include src tests harness; do
  require_dir "$dir"
done
require_file docs/architecture/A-001-system-boundary.md
pass "estrutura arquitetural"

require_file CMakeLists.txt
require_text CMakeLists.txt "set(CMAKE_CXX_STANDARD 23)"
require_text CMakeLists.txt "cxx_std_23"
pass "C++23"

cmake -S . -B "$BUILD_DIR" -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build "$BUILD_DIR"
pass "CMake"

ctest --test-dir "$BUILD_DIR" --output-on-failure
pass "CTest"

require_file specs/invariants/precipitation-spatial-contract.yaml
for invariant in N01 N02 N03 N04 N05 N06 N07 N08; do
  require_text specs/invariants/precipitation-spatial-contract.yaml "id: $invariant"
done
require_text specs/domain/precipitation.md "não é uma quantidade extensiva no espaço"
pass "contrato científico registrado"

require_file docs/lineage/sister-clima-v2.5.md
require_text docs/lineage/sister-clima-v2.5.md "status: legacy_reference"
require_text docs/lineage/sister-clima-v2.5.md "replacement: sister-atmos"
require_text docs/lineage/sister-clima-v2.5.md "migration_mode: behavioral_equivalence"
pass "linhagem Sister-Clima 2.5"

require_file docs/architecture/A-002-nexo-integration-boundary.md
require_text docs/architecture/A-002-nexo-integration-boundary.md "Nexo-aware, não Nexo-owned"
require_file contracts/nexo_research_integration.schema.json
pass "fronteira Nexo"

require_file .hoa/project.yaml
require_file .hoa/atmosphere-a0.yaml
require_file domain/REFLEXIVITY.md
require_text .hoa/project.yaml "runtime_dependency_on_praxis: false"
require_text .hoa/project.yaml "authorized_decision"
pass "Praxis scaffold"

printf '\nAtmos A0: PASS\n'
printf 'Evidência operacional: %s\n' "$RUN_EVIDENCE_FILE"
trap - EXIT
