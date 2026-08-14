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

## Marco verificado — A1-E002

**Product Selection & Temporal Coverage**.

Escopo autorizado:

- identidade de produto solicitado e efetivamente utilizado;
- seleção cientificamente explícita;
- estado de fallback explicitamente representável;
- período solicitado versus período efetivamente coberto;
- lacunas e completude temporal explícitas;
- `GC-006` e `GC-007`;
- invariantes `N04`, `N07`, `N08`, `N12` e `N17`.

Limites deliberados:

- GC-006 verifica seleção direta, não fallback positivo;
- N17 constitui contexto comum de produto/período sem implementar views;
- providers, H3, superfície, persistência, rede, frontend e Nexo operacional
  permanecem fora do marco.

O marco está **verificado** no commit `9a321fb`.

## Próximo marco proposto — A1-E003

**Spatial Representation & Operational Sampling**.

O marco está constituído como proposta, mas permanece **não autorizado**.

Escopo candidato:

- `GC-004` — média das amostras operacionais únicas, sem reivindicar superfície nativa;
- `GC-005` — produto pontual rejeita superfície sem geometria nativa conhecida;
- `GC-008` — bucket H3 usa média espacial, nunca soma espacial de precipitação em mm;
- invariantes `N02`, `N05`, `N06`, `N09`, `N10`, `N16` e `N18`;
- identidade espacial canônica na entrada do núcleo, deixando normalização entre providers para a camada de aquisição.

A1-E003 deve encerrar a fase A1 ao tornar nativos os oito Golden Cases do baseline.

## Primeiro horizonte de produto — A2-MVP-001

**Governed Precipitation Explorer**.

Depois de A1-E003, a primeira fatia vertical do Atmos deverá permitir uma análise
de precipitação utilizável e rastreável, conectando aquisição governada, núcleo
científico, API e frontend leve.

Escopo mínimo esperado:

- um caminho de aquisição meteorológica explicitamente governado;
- território e período como entrada;
- produto solicitado e produto usado visíveis;
- período solicitado e período efetivo visíveis;
- precipitação e representação espacial compatíveis com o produto;
- proveniência científica retornada junto ao resultado;
- `GET /api/health`;
- um endpoint de análise de precipitação;
- frontend HTML/CSS/JS sem framework pesado;
- execução local independente de Nexo e Praxis.

Persistência, autenticação, múltiplos providers e integração operacional com Nexo
não são pré-condições do primeiro MVP, salvo evidência posterior em contrário.

A2-MVP-001 permanece apenas como horizonte e **não está autorizado**.

## Fronteira posterior ao núcleo científico

Providers, persistência, API, frontend e integração operacional com o Nexo só
devem ser ativados depois que as respectivas semânticas científicas estiverem
constituídas e verificáveis no núcleo nativo.
