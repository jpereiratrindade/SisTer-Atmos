# SisTer Atmos — roadmap científico e de engenharia

## Fonte canônica do estado

O estado corrente do projeto é registrado exclusivamente em
`.hoa/project-state.yaml`.

Este roadmap descreve direção e critérios conceituais. Se houver divergência
entre este texto e o manifesto de estado, o manifesto deve prevalecer e a
divergência deve falhar no gate de governança.

## Marcos concluídos

### A0 — Constituição

Identidade, fronteiras, linhagem Sister-Clima, contrato científico inicial,
fronteira Atmos ↔ Nexo e independência de runtime do Praxis.

### A0-E001 — Oráculo científico

Qualificação do Sister-Clima 2.5 como referência temporária e constituição dos
invariantes `N01–N18` e Golden Cases `GC-001–GC-008`.

### A1-E001 — Native precipitation scientific core

Primeira migração científica nativa C++23: tipos fortes, deduplicação,
conflito e agregação ponderada por área. Golden Cases `GC-001–GC-003` nativos.

## Próximo marco proposto — A1-E002

**Product Selection & Temporal Coverage**.

Escopo candidato:

- identidade de produto solicitado e efetivamente utilizado;
- seleção/fallback cientificamente explícitos;
- período solicitado versus período efetivamente coberto;
- `GC-006` e `GC-007`.

O marco permanece **proposto, não autorizado**, até sua constituição formal.

## Horizonte seguinte — A1-E003

**Spatial Representation & Operational Sampling**.

Escopo candidato: `GC-004`, `GC-005`, `GC-008`, distinguindo amostragem,
superfície meteorológica e representação territorial/H3.

## Fronteira posterior ao núcleo científico

Providers, persistência, API, frontend e integração operacional com o Nexo só
devem ser ativados depois que as respectivas semânticas científicas estiverem
constituídas e verificáveis no núcleo nativo.
