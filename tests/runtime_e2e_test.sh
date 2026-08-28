#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUNTIME="${ROOT}/scripts/runtime.sh"
ATMOS_BINARY="${ATMOS_BINARY:-${ROOT}/build/sister-atmos-http}"

[[ -x "${ATMOS_BINARY}" ]] || {
  printf '[FAIL] binário Atmos E2E ausente/executável: %s\n' "${ATMOS_BINARY}" >&2
  exit 1
}

command -v curl >/dev/null 2>&1 || {
  printf '[FAIL] curl é necessário para o E2E do runtime.\n' >&2
  exit 1
}

TMP="$(mktemp -d)"
cleanup() {
  SISTER_RESOLVED_DEPLOYMENT_FILE="${TMP}/deployment.json" \
  SISTER_RUNTIME_STATE_DIR="${TMP}/state" \
  SISTER_RUNTIME_RUN_DIR="${TMP}/run" \
  ATMOS_BINARY="${ATMOS_BINARY}" \
    "${RUNTIME}" stop >/dev/null 2>&1 || true
  rm -rf "${TMP}"
}
trap cleanup EXIT

mkdir -p "${TMP}/state" "${TMP}/run"

PORT="$(python3 - <<'PY'
import socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.bind(("127.0.0.1", 0))
    print(sock.getsockname()[1])
PY
)"

cat > "${TMP}/deployment.json" <<EOF_JSON
{
  "components": [
    {
      "system_id": "sister_atmos",
      "runtime": {
        "transport": "tcp",
        "listen": "127.0.0.1",
        "port": ${PORT}
      }
    }
  ]
}
EOF_JSON

run_runtime() {
  SISTER_RESOLVED_DEPLOYMENT_FILE="${TMP}/deployment.json" \
  SISTER_RUNTIME_STATE_DIR="${TMP}/state" \
  SISTER_RUNTIME_RUN_DIR="${TMP}/run" \
  ATMOS_BINARY="${ATMOS_BINARY}" \
    "${RUNTIME}" "$@"
}

run_runtime start >/dev/null
run_runtime status | grep -Eq '^running pid=[0-9]+$'

ready=0
for _ in {1..30}; do
  if run_runtime readiness >/dev/null 2>&1; then
    ready=1
    break
  fi
  sleep 0.1
done
[[ "${ready}" -eq 1 ]] || {
  printf '[FAIL] runtime real não ficou ready.\n' >&2
  exit 1
}

health="$(run_runtime health)"
grep -Fq '"system_id":"sister_atmos"' <<< "${health}"
grep -Fq '"status":"ok"' <<< "${health}"

ready_body="$(run_runtime readiness)"
grep -Fq '"system_id":"sister_atmos"' <<< "${ready_body}"
grep -Fq '"status":"ready"' <<< "${ready_body}"

run_runtime restart >/dev/null
run_runtime status | grep -Eq '^running pid=[0-9]+$'
run_runtime stop >/dev/null

if run_runtime status >/dev/null 2>&1; then
  printf '[FAIL] status deveria falhar após stop no E2E.\n' >&2
  exit 1
fi

printf '[PASS] runtime Atmos real + deployment resolvido + observabilidade canônica.\n'
