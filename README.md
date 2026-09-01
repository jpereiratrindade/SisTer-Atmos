# SisTer Atmos

**SisTer Atmos — inteligência climática e territorial para pesquisa.**

- `system_id`: `sister_atmos`
- Linguagem: C++23
- Papel: subsistema independente do ecossistema SisTer.

## Estado atual

<!-- praxis-state:start -->
- Fase atual: **A2 — produto climático vertical**.
- Último marco verificado: **A1-E003 — Spatial Representation & Operational Sampling** (`fad18c1`).
- Marco ativo: **A2-MVP-001 — Governed Precipitation Explorer**.
- A2-MVP-001 está **constituído, autorizado e ativo**.
<!-- praxis-state:end -->

A fonte canônica desse estado é `.hoa/project-state.yaml`. O README é uma
visão humana e deve ser validado contra esse manifesto.

## O que já está comprovado

- A0 — Constituição: verificado;
- A0-E001 — oráculo científico Sister-Clima: qualificado;
- `N01–N18` e `GC-001–GC-008`: baseline científico;
- A1-E001: `GC-001–GC-003` implementados e testados nativamente em C++23;
- A1-E002: `GC-006–GC-007` implementados e testados nativamente em C++23;
- A1-E003: `GC-004`, `GC-005` e `GC-008` implementados e testados nativamente em C++23;
- `GC-001–GC-008` estão nativos no núcleo científico A1;
- produto solicitado/usado e período solicitado/efetivo permanecem explícitos;
- amostragem operacional, elegibilidade de superfície e agregação de apresentação permanecem semanticamente distintas;
- warnings-as-errors, ASan e UBSan: gateados;
- runtime sem dependência de Python, Sister-Clima, H3, banco, HTTP ou frontend no A1.

## Governed Precipitation Explorer

O corte A2 ativo oferece busca de localidades, séries diárias de precipitação
Best Match, ERA5 e NASA POWER PRECTOTCORR, período solicitado versus efetivo, ausências explícitas,
KPIs, gráficos, tabela, exportação CSV, proveniência, limite municipal IBGE,
distribuição operacional H3 r6→r5, exportação GeoJSON e comparação territorial
do RS por município, bioma predominante, COREDE e Região Funcional.
O backend de aquisição e análise é C++23; o frontend HTML/CSS/JS é servido pelo
mesmo runtime e adota a linguagem visual do SisTer Nexo.

## Pronto, mas incompleto

Atmos separa **readiness operacional** de **completude funcional**. Cada corte
promovido deve permanecer utilizável no escopo que declara, mesmo enquanto o
produto continua incorporando novas capacidades. O runtime não deve fingir
saúde quando uma capacidade declarada quebra; o que não pode ocorrer é tratar
backlog futuro como indisponibilidade presente.

- `GET /` oferece uma superfície HTML mínima e sempre utilizável;
- `GET /_sister/ready` mede o escopo corrente;
- `GET /api/status` explicita que readiness e completude são estados distintos.

A decisão arquitetural completa está em
`docs/architecture/ATMOS_READY_INCOMPLETE.md`.

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
