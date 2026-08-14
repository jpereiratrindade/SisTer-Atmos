# A1-E003 — Spatial Representation & Operational Sampling

## Estado

**Constituído, autorizado e ativo.**

Baseline verificado:

- marco: `A1-E002`;
- commit científico: `9a321fb`;
- gate: `scripts/verify_a1_e002.sh`.

## Objetivo

Fechar o núcleo científico A1 com a semântica de amostragem operacional,
elegibilidade de superfície e agregação territorial para apresentação.

## Golden Cases candidatos

- `GC-004` — média de amostras operacionais únicas;
- `GC-005` — produto pontual não gera superfície;
- `GC-008` — média espacial H3, nunca soma espacial.

## Invariantes candidatos

- `N02`;
- `N05`;
- `N06`;
- `N09`;
- `N10`;
- `N16`;
- `N18`.

## Decisões constitutivas

1. A1-E003 testa semântica espacial sem exigir biblioteca H3 de runtime.
2. Amostragem operacional e superfície meteorológica são representações
   cientificamente distintas.
3. Bucket de apresentação nunca autoriza soma espacial de precipitação em mm.
4. Resolução de amostragem e resolução de apresentação permanecem separadas.
5. Qualquer adaptação de amostragem deve ser explícita na proveniência.
6. O núcleo recebe coordenadas canônicas; normalização entre providers fica
   fora de A1 e deverá ser constituída na camada de aquisição.

## Fronteira da prova

Um futuro `A1-E003: PASS` deverá significar apenas que as semânticas acima foram
submetidas à prova no núcleo nativo C++23.

Não significará que o Atmos possui provider real, H3 operacional, API,
persistência, frontend, escala de produção ou integração operacional com Nexo.

## Critérios futuros de promoção

A1-E003 somente poderá tornar-se `verified` após:

- autorização explícita em commit separado;
- implementação nativa C++23 de GC-004, GC-005 e GC-008;
- ligação dos invariantes aos testes;
- preservação integral de A1-E002;
- warnings-as-errors;
- ASan e UBSan;
- gate próprio em PASS;
- evidência governada e checksum.

## Horizonte pós-A1

Depois do fechamento de A1-E003, o primeiro objetivo de produto será
`A2-MVP-001 — Governed Precipitation Explorer`: uma fatia vertical utilizável
do Atmos que combine aquisição governada, núcleo científico, API e frontend,
sem reivindicar capacidades que não tenham sido comprovadas.

A constituição deste documento não autoriza A2-MVP-001.
