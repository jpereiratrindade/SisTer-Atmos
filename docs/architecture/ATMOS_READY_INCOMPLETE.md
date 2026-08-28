# Atmos — Ready, not complete

## Status

Architectural invariant for incremental delivery.

## Decision

SisTer Atmos distinguishes **operational readiness** from **product completeness**.

- **Ready** means the running system can serve the capabilities declared by the current cut.
- **Incomplete** means the product remains open to additional scientific, territorial, acquisition, API and presentation capabilities.
- Missing future capability is not, by itself, a readiness failure.
- A broken declared capability *is* a readiness failure and must never be hidden by this principle.

The invariant is therefore not “always return 200”. It is:

> every promoted cut must be usable in its declared scope, while Atmos never needs to claim that its scientific product is finished.

## Observable contract

`GET /_sister/ready` remains the canonical runtime readiness endpoint and reports readiness only for the current declared capability scope.

`GET /api/status` exposes the orthogonal evolution state:

```json
{
  "system_id": "sister_atmos",
  "operational_status": "ready",
  "readiness_scope": "current_declared_capabilities",
  "complete": false,
  "evolution_status": "incomplete",
  "completion_policy": "continuous_evolution"
}
```

`GET /` is a minimal self-hosted HTML shell. It is deliberately independent of future explorer features so `sister-infra dev preview` always has a valid presentation surface for a promoted Atmos cut.

## Consequences

1. New features must enter behind a working vertical cut, not behind a blank root page.
2. Roadmap completeness cannot be used as the health/readiness condition.
3. A feature already declared as available must have its failure reflected by the appropriate operational contract.
4. The shell may evolve into the precipitation explorer, but the ready-versus-complete distinction remains.
5. No external CDN, framework or separate frontend process is required for the baseline shell.
