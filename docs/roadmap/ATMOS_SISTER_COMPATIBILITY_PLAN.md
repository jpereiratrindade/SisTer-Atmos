# SisTer Atmos — Compatibility Closure Plan

## Status

Execution target: `ATMOS-COMPAT-CLOSE`
Baseline: `cb6bda2`
Scientific authority: unchanged through `A1-E003`
A2 authorization: **not granted**

## Objective

Make SisTer Atmos a first-class SisTer component without teaching Atmos about
DEV/LAB/PROD, gateway, DNS, TLS, public URLs, concrete host topology, or sibling
components.

The component owns identity, artifacts, runtime lifecycle and canonical
observability. `sister-infra` owns discovery, qualification, composition,
deployment resolution, reconcile and publication.

## Invariants

1. `.sister/component.json` remains network-neutral.
2. Runtime binding comes from `SISTER_RESOLVED_DEPLOYMENT_FILE` when supplied.
3. Runtime state/run directories come from SisTer runtime bindings when supplied.
4. `/_sister/health` and `/_sister/ready` are operational adapter endpoints.
5. The operational adapter does not create a second scientific OpenAPI authority.
6. The C++23 scientific core remains independent from HTTP/deployment concerns.
7. No A2 capability is authorized by this compatibility work.

## Deliverables

### C1 — Local contract closure

- validate `sister.component/1.0.0` and `sister.runtime/1.0.0`;
- prove exactly one deployment binding for `sister_atmos`;
- reject missing/duplicated/invalid bindings before process mutation;
- remove the `workstation` fallback from component-owned paths.

### C2 — Real runtime witness

- start the real `sister-atmos-http` from `scripts/runtime.sh`;
- consume a synthetic resolved deployment;
- prove status, health, readiness, restart and stop on an isolated ephemeral port.

### C3 — Canonical gate integration

- register contract and E2E tests in CTest;
- make `verify_current_state.sh` include the local SisTer contract smoke;
- provide `scripts/verify_sister_compatibility.sh` as the complete local gate.

### C4 — External qualification witness

When `SISTER_INFRA_ROOT` is supplied, execute the generic `sister-component
qualify` witness against this repository. Qualification must succeed without an
Atmos special-case in the Infra engine.

## Stop condition

This compatibility patch is complete when:

```text
verify_current_state.sh                 PASS
verify_sister_compatibility.sh          PASS
SISTER_INFRA_ROOT=... verify_sister...  PASS
```

Composition, DEV preview, LAB reconcile and production publication remain
`sister-infra` responsibilities and must be proven there, using this unchanged
component contract.
