# A1-E001 — Native precipitation scientific core

## Objetivo

Migrar para C++23 a primeira fatia do contrato científico de precipitação,
sem introduzir providers, rede, persistência, H3 ou dependência de runtime do
Sister-Clima/Praxis.

## Escopo científico

Golden Cases nativos:

- `GC-001` — deduplicação de célula meteorológica nativa;
- `GC-002` — rejeição de conflito para a mesma célula nativa;
- `GC-003` — agregação de precipitação ponderada por área representada.

Invariantes centrais: `N02`, `N03`, `N13`, `N14` e `N15`.

## Resultado

`A1-E001` foi verificado e promovido por fast-forward para `main` no commit
`7dc0115`.

O gate comprovou simultaneamente:

- preservação de A0 e A0-E001;
- capacidades C++23 REQUIRED;
- `std::mdspan` apenas como capacidade TRIAL;
- warnings-as-errors;
- ASan + UBSan;
- ausência de estado global/estático mutável no núcleo A1;
- ausência de contaminação semântica A1 → A0.

## Evidência governada

- gate: `scripts/verify_a1_e001.sh`;
- evidência preservada: `docs/experiments/evidence/a1-e001/verification.txt`;
- checksum: `docs/experiments/evidence/a1-e001/verification.sha256`.
