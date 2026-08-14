# SisTer Atmos

**SisTer Atmos — inteligência climática e territorial para pesquisa.**

- `system_id`: `sister_atmos`
- Linguagem: C++23
- Papel: subsistema independente do ecossistema SisTer.

## Estado atual

<!-- praxis-state:start -->
- Fase atual: **A1 — núcleo científico nativo**.
- Último marco verificado: **A1-E002 — Product Selection & Temporal Coverage** (`9a321fb`).
- Próximo marco proposto: **A1-E003 — Spatial Representation & Operational Sampling**.
- A1-E003 está **proposto e ainda não autorizado**.
<!-- praxis-state:end -->

A fonte canônica desse estado é `.hoa/project-state.yaml`. O README é uma
visão humana e deve ser validado contra esse manifesto.

## O que já está comprovado

- A0 — Constituição: verificado;
- A0-E001 — oráculo científico Sister-Clima: qualificado;
- `N01–N18` e `GC-001–GC-008`: baseline científico;
- A1-E001: `GC-001–GC-003` implementados e testados nativamente em C++23;
- warnings-as-errors, ASan e UBSan: gateados;
- runtime sem dependência de Python, Sister-Clima, H3, banco ou HTTP no A1-E001.

## Princípio de migração

Sister-Clima 2.5 permanece como `legacy_reference` e oráculo temporário de
regressão. O objetivo da migração é equivalência comportamental quando a regra
legada estiver correta, ou diferença explicitamente justificada quando não
estiver.

## Praxis e memória governada

`.hoa/`, `domain/`, `specs/`, `harness/` e `docs/experiments/` formam a memória
governada de engenharia. O runtime do Atmos não depende do Praxis.

Build é transitório (`.build/`). Evidência de promoção é preservada em
`docs/experiments/evidence/`.

## Verificação do estado corrente

```bash
./scripts/verify_current_state.sh
```

O gate resolve o último verificador comprovado a partir do estado canônico e,
em seguida, verifica consistência de manifests, README, evidência, metadados e
segurança do bootstrap.

## Roadmap

Ver `docs/roadmap/ATMOS_ROADMAP.md`.
