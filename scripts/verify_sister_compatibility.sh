#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

BUILD_DIR="${SISTER_ATMOS_COMPAT_BUILD_DIR:-${ROOT}/.build/sister-compat}"
INFRA_ROOT="${SISTER_INFRA_ROOT:-}"

printf '=== SisTer Atmos — compatibility gate ===\n'
printf 'root=%s\n' "${ROOT}"
printf 'build=%s\n\n' "${BUILD_DIR}"

./tests/component_runtime_contract_test.sh

rm -rf "${BUILD_DIR}"
cmake \
  -S . \
  -B "${BUILD_DIR}" \
  -G Ninja \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DSISTER_ATMOS_WARNINGS_AS_ERRORS=ON

cmake --build "${BUILD_DIR}" --parallel 2
ctest --test-dir "${BUILD_DIR}" --output-on-failure

if [[ -n "${INFRA_ROOT}" ]]; then
  COMPONENT_TOOL="${INFRA_ROOT}/bin/sister-component"
  [[ -x "${COMPONENT_TOOL}" ]] || {
    printf '[FAIL] sister-component ausente/executável: %s\n' "${COMPONENT_TOOL}" >&2
    exit 1
  }

  printf '\n=== Qualificação pelo sister-infra ===\n'
  "${COMPONENT_TOOL}" inspect "${ROOT}"
  "${COMPONENT_TOOL}" validate "${ROOT}"
  "${COMPONENT_TOOL}" qualify "${ROOT}"
else
  printf '\n[INFO] qualificação externa não executada; defina SISTER_INFRA_ROOT para habilitá-la.\n'
fi

printf '\nAtmos SisTer compatibility: PASS\n'
