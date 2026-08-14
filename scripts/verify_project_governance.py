#!/usr/bin/env python3
from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def fail(message: str) -> None:
    print(f"[FAIL] {message}", file=sys.stderr)
    raise SystemExit(1)


def passed(message: str) -> None:
    print(f"[PASS] {message}")


def scalar(path: Path, key: str) -> str:
    pattern = re.compile(rf"^\s*{re.escape(key)}:\s*(.*?)\s*$")
    for raw in path.read_text(encoding="utf-8").splitlines():
        match = pattern.match(raw)
        if match:
            value = match.group(1).strip()
            if len(value) >= 2 and value[0] == value[-1] and value[0] in {'\"', "'"}:
                value = value[1:-1]
            return value
    fail(f"campo ausente em {path.relative_to(ROOT)}: {key}")
    return ""


def require_file(relative: str) -> Path:
    path = ROOT / relative
    if not path.is_file():
        fail(f"arquivo ausente: {relative}")
    return path


def git(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *args], cwd=ROOT, text=True, capture_output=True, check=False
    )


state_path = require_file(".hoa/project-state.yaml")
project_path = require_file(".hoa/project.yaml")
manifest_path = require_file("contracts/system_manifest.json")
readme_path = require_file("README.md")

project = scalar(state_path, "project")
governance_method = scalar(state_path, "governance_method")
bootstrap_script = scalar(state_path, "bootstrap_script")
bootstrap_policy = scalar(state_path, "bootstrap_policy")
phase = scalar(state_path, "phase")
last_milestone = scalar(state_path, "last_verified_milestone")
last_commit = scalar(state_path, "last_verified_commit")
last_gate = scalar(state_path, "last_verified_gate")
last_evidence = scalar(state_path, "last_verified_evidence")
next_milestone = scalar(state_path, "next_milestone")
next_status = scalar(state_path, "next_milestone_status")
next_authorized = scalar(state_path, "next_milestone_authorized")

if project != "sister_atmos":
    fail(f"project-state identifica projeto inesperado: {project}")
if governance_method != "praxis-governed-engineering/0.1":
    fail(f"método de governança inesperado: {governance_method}")
if bootstrap_policy != "create_once":
    fail(f"política de bootstrap inesperada: {bootstrap_policy}")
require_file(bootstrap_script)

if scalar(project_path, "id") != project:
    fail(".hoa/project.yaml diverge do project-state em project/id")

if scalar(project_path, "phase") != phase:
    fail(".hoa/project.yaml diverge do project-state em phase")

manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
if manifest.get("system_id") != project:
    fail("system_manifest diverge do project-state em system_id")
if manifest.get("phase") != phase:
    fail("system_manifest diverge do project-state em phase")

passed("estado canônico consistente entre .hoa e system manifest")

readme = readme_path.read_text(encoding="utf-8")
required_readme = [
    f"Fase atual: **{phase}",
    f"Último marco verificado: **{last_milestone}",
    next_milestone,
    ".hoa/project-state.yaml",
]
for required_text in required_readme:
    if required_text not in readme:
        fail(
            f"README não reflete estado canônico: {required_text}"
        )

if next_status == "proposed":
    if next_authorized != "false":
        fail("marco proposed não pode estar autorizado")

    human_state = (
        f"{next_milestone} está "
        "**proposto e ainda não autorizado**"
    )

elif next_status == "active":
    if next_authorized != "true":
        fail("marco active exige autorização explícita")

    human_state = (
        f"{next_milestone} está "
        "**constituído, autorizado e ativo**"
    )

    active_manifest = (
        ROOT / ".hoa" /
        f"atmosphere-{next_milestone.lower()}.yaml"
    )

    if not active_manifest.is_file():
        fail(
            "marco ativo sem manifesto: "
            f"{active_manifest.relative_to(ROOT)}"
        )

    if scalar(active_manifest, "status") != "active":
        fail(
            f"manifesto do marco ativo não está active: "
            f"{next_milestone}"
        )

    if scalar(active_manifest, "authorized") != "true":
        fail(
            f"manifesto do marco ativo não está autorizado: "
            f"{next_milestone}"
        )

elif next_status == "blocked":
    human_state = next_milestone

elif next_status == "none":
    if next_authorized != "false":
        fail("next_milestone none não pode estar autorizado")
    human_state = next_milestone

else:
    fail(f"estado de próximo marco inválido: {next_status}")

if human_state not in readme:
    fail("README não representa status/autorização do próximo marco")

passed("README reflete o estado canônico")
passed("transição/autorização do próximo marco é válida")

experiment_file = ROOT / ".hoa" / f"atmosphere-{last_milestone.lower()}.yaml"
if not experiment_file.is_file():
    fail(f"manifesto do último marco ausente: {experiment_file.relative_to(ROOT)}")
if scalar(experiment_file, "status") != "verified":
    fail(f"último marco não está verified: {last_milestone}")
passed("último marco possui manifesto verified")

require_file(last_gate)
evidence_path = require_file(last_evidence)
if f"Atmos {last_milestone}: PASS" not in evidence_path.read_text(encoding="utf-8"):
    fail("evidência governada não contém PASS do último marco")
passed("evidência governada do último marco preservada")

checksum_path = evidence_path.with_suffix(".sha256")
if not checksum_path.is_file():
    fail(f"checksum da evidência ausente: {checksum_path.relative_to(ROOT)}")

# Valida checksum sem depender de ferramenta externa.
import hashlib
expected_hash = checksum_path.read_text(encoding="utf-8").split()[0]
observed_hash = hashlib.sha256(evidence_path.read_bytes()).hexdigest()
if expected_hash != observed_hash:
    fail("checksum da evidência governada diverge")
passed("checksum da evidência governada íntegro")

ancestor = git("merge-base", "--is-ancestor", last_commit, "HEAD")
if ancestor.returncode != 0:
    fail(f"commit verificado {last_commit} não é ancestral de HEAD")
passed("commit do último marco permanece na ancestralidade de HEAD")

# Referências conhecidas do A1-E001 devem apontar para artefatos reais.
a1 = require_file(".hoa/atmosphere-a1-e001.yaml").read_text(encoding="utf-8")
expected_tests = [
    "tests/precipitation_domain_types_tests.cpp",
    "tests/precipitation_native_samples_tests.cpp",
    "tests/precipitation_area_weighting_tests.cpp",
]
for test in expected_tests:
    if test not in a1:
        fail(f"A1-E001 não referencia teste real: {test}")
    require_file(test)
if "tests/precipitation_core_tests.cpp" in a1:
    fail("A1-E001 ainda referencia teste agregado inexistente")
passed("metadados A1-E001 referenciam testes reais")

ignored = git("check-ignore", ".build")
if ignored.returncode != 0:
    fail(".build não está protegido pelo .gitignore")
tracked_build = git("ls-files", ".build")
if tracked_build.stdout.strip():
    fail("há artefatos .build versionados")
passed("build permanece transitório e fora da memória governada")

bootstrap = require_file("scripts/bootstrap_sister_atmos_a0.sh").read_text(encoding="utf-8")
if "bootstrap A0 não altera projeto existente" not in bootstrap:
    fail("bootstrap A0 não declara proteção create-once")
passed("bootstrap A0 possui proteção monotônica/create-once")

print("\nAtmos governance state: PASS")
