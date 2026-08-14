# A-002 — Fronteira Atmos ↔ Nexo

## Decisão

O Atmos nasce **Nexo-aware, não Nexo-owned**.

O Nexo fornece contexto de pesquisa, como `project_id`, e recebe referências a
resultados científicos produzidos pelo Atmos.

Fluxo conceitual:

Nexo project_id
→ Atmos climate_analysis
→ analysis_id + provenance + indicators + spatial_layer
→ Nexo evidence

## Regra

O Nexo não deve conhecer detalhes internos de providers, H3 ou agregação.
O Atmos não deve assumir propriedade sobre projetos, atividades ou evidências.
