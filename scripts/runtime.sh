#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Stable installed-runtime boundary for sister.component/sister.runtime.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

ACTION="${1:-run}"

load_deployment_binding() {
  local resolved="${SISTER_RESOLVED_DEPLOYMENT_FILE:-}"
  [[ -n "${resolved}" ]] || return 0
  [[ -f "${resolved}" ]] || {
    printf '[FAIL] deployment resolvido ausente: %s\n' "${resolved}" >&2
    return 1
  }
  command -v jq >/dev/null 2>&1 || {
    printf '[FAIL] jq é necessário para consumir deployment resolvido.\n' >&2
    return 1
  }

  local system_id transport
  system_id="$(jq -er '.system_id' "${PROJECT_DIR}/.sister/component.json")"
  transport="$(jq -er --arg id "${system_id}" \
    '.components[] | select(.system_id == $id) | .runtime.transport' \
    "${resolved}")"
  [[ "${transport}" == "tcp" ]] || {
    printf '[FAIL] runtime Atmos ainda requer binding TCP.\n' >&2
    return 1
  }
  export ATMOS_ADDRESS
  export ATMOS_PORT
  ATMOS_ADDRESS="$(jq -er --arg id "${system_id}" \
    '.components[] | select(.system_id == $id) | .runtime.listen' \
    "${resolved}")"
  ATMOS_PORT="$(jq -er --arg id "${system_id}" \
    '.components[] | select(.system_id == $id) | .runtime.port' \
    "${resolved}")"
}

load_deployment_binding

ATMOS_ADDRESS="${ATMOS_ADDRESS:-127.0.0.1}"
ATMOS_PORT="${ATMOS_PORT:-8095}"
ATMOS_STATE_DIR="${ATMOS_STATE_DIR:-${SISTER_RUNTIME_STATE_DIR:-${XDG_STATE_HOME:-${HOME}/.local/state}/sister/workstation/atmos}}"
ATMOS_RUNTIME_DIR="${ATMOS_RUNTIME_DIR:-${SISTER_RUNTIME_RUN_DIR:-${XDG_RUNTIME_DIR:-${TMPDIR:-/tmp}}/sister/atmos}}"
export SISTER_RUNTIME_STATE_DIR="${ATMOS_STATE_DIR}"
export SISTER_RUNTIME_RUN_DIR="${ATMOS_RUNTIME_DIR}"

ATMOS_BINARY="${ATMOS_BINARY:-${PROJECT_DIR}/build/sister-atmos-http}"
ATMOS_PID_FILE="${ATMOS_PID_FILE:-${ATMOS_RUNTIME_DIR}/sister-atmos.pid}"
ATMOS_LOG_PATH="${ATMOS_LOG_PATH:-${ATMOS_RUNTIME_DIR}/sister-atmos.log}"

mkdir -p "${ATMOS_STATE_DIR}" "${ATMOS_RUNTIME_DIR}"

require_binary() {
  [[ -x "${ATMOS_BINARY}" ]] || {
    printf '[FAIL] sister-atmos-http não encontrado/executável: %s\n' "${ATMOS_BINARY}" >&2
    return 1
  }
}

read_pid() {
  [[ -r "${ATMOS_PID_FILE}" ]] || return 1

  local pid
  IFS= read -r pid < "${ATMOS_PID_FILE}"

  [[ "${pid}" =~ ^[0-9]+$ ]] || return 1
  printf '%s\n' "${pid}"
}

pid_is_alive() {
  local pid="$1"
  kill -0 "${pid}" 2>/dev/null
}

pid_matches_runtime() {
  local pid="$1"
  local cmdline="/proc/${pid}/cmdline"

  [[ -r "${cmdline}" ]] || return 1

  tr '\0' '\n' < "${cmdline}" \
    | grep -Fqx -- "${ATMOS_BINARY}"
}

running_pid() {
  local pid

  pid="$(read_pid)" || return 1

  if ! pid_is_alive "${pid}"; then
    rm -f "${ATMOS_PID_FILE}"
    return 1
  fi

  if ! pid_matches_runtime "${pid}"; then
    printf '[FAIL] PID %s não pertence ao runtime Atmos esperado; recusando operar.\n' "${pid}" >&2
    return 2
  fi

  printf '%s\n' "${pid}"
}

server_args=(
  --bind "${ATMOS_ADDRESS}"
  --port "${ATMOS_PORT}"
)

start_background() {
  require_binary

  local pid
  local rc=0

  pid="$(running_pid)" || rc="$?"

  if [[ "${rc}" -eq 0 ]]; then
    printf '[PASS] SisTer-Atmos já está ativo (pid=%s).\n' "${pid}"
    return 0
  fi

  if [[ "${rc}" -eq 2 ]]; then
    return 1
  fi

  rm -f "${ATMOS_PID_FILE}"

  nohup "${ATMOS_BINARY}" "${server_args[@]}" \
    >> "${ATMOS_LOG_PATH}" 2>&1 &

  pid="$!"
  printf '%s\n' "${pid}" > "${ATMOS_PID_FILE}"

  sleep 0.2

  if ! pid_is_alive "${pid}"; then
    printf '[FAIL] SisTer-Atmos encerrou durante o start. Log: %s\n' "${ATMOS_LOG_PATH}" >&2
    rm -f "${ATMOS_PID_FILE}"
    return 1
  fi

  printf '[PASS] SisTer-Atmos iniciado (pid=%s).\n' "${pid}"
}

stop_runtime() {
  local pid
  local rc=0

  pid="$(running_pid)" || rc="$?"

  if [[ "${rc}" -eq 2 ]]; then
    return 1
  fi

  if [[ "${rc}" -ne 0 ]]; then
    rm -f "${ATMOS_PID_FILE}"
    printf '[PASS] SisTer-Atmos já estava parado.\n'
    return 0
  fi

  kill -TERM "${pid}"

  local i
  for i in {1..50}; do
    if ! pid_is_alive "${pid}"; then
      rm -f "${ATMOS_PID_FILE}"
      printf '[PASS] SisTer-Atmos parado.\n'
      return 0
    fi
    sleep 0.1
  done

  if pid_matches_runtime "${pid}"; then
    kill -KILL "${pid}" 2>/dev/null || true
  fi

  rm -f "${ATMOS_PID_FILE}"
  printf '[PASS] SisTer-Atmos parado após timeout de encerramento gracioso.\n'
}

status_runtime() {
  local pid
  local rc=0

  pid="$(running_pid)" || rc="$?"

  if [[ "${rc}" -eq 0 ]]; then
    printf 'running pid=%s\n' "${pid}"
    return 0
  fi

  if [[ "${rc}" -eq 2 ]]; then
    return 1
  fi

  printf 'stopped\n'
  return 3
}

http_observation() {
  local path="$1"

  command -v curl >/dev/null 2>&1 || {
    printf '[FAIL] curl é necessário para observar o adapter HTTP corrente.\n' >&2
    return 1
  }

  curl \
    --fail \
    --silent \
    --show-error \
    --max-time 3 \
    "http://${ATMOS_ADDRESS}:${ATMOS_PORT}${path}"
}

health_runtime() {
  http_observation "/_sister/health"
  printf '\n'
}

readiness_runtime() {
  local body
  body="$(http_observation "/_sister/ready")"

  printf '%s\n' "${body}"

  grep -Eq '"status"[[:space:]]*:[[:space:]]*"ready"' <<< "${body}"
}

run_foreground() {
  require_binary

  printf '%s\n' "$$" > "${ATMOS_PID_FILE}"

  exec "${ATMOS_BINARY}" "${server_args[@]}"
}

case "${ACTION}" in
  start)
    start_background
    ;;
  stop)
    stop_runtime
    ;;
  restart)
    stop_runtime
    start_background
    ;;
  status)
    status_runtime
    ;;
  health)
    health_runtime
    ;;
  readiness)
    readiness_runtime
    ;;
  run)
    run_foreground
    ;;
  *)
    printf 'Uso: %s {start|stop|restart|status|health|readiness|run}\n' "$0" >&2
    exit 2
    ;;
esac
