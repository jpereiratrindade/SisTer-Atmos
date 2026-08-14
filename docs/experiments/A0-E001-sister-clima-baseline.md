# A0-E001 — Baseline científico do Sister-Clima

## Objetivo

Qualificar o Sister-Clima como implementação de referência temporária
para a migração científica ao SisTer Atmos.

## Resultado

O Sister-Clima v2.5 foi observado estaticamente e teve sua suíte de testes
executada sem modificação do legado ou do SisTer Atmos.

Foram executados 34 testes com resultado integralmente positivo.

## Identidade do oráculo

A tag anotada `v2.5` referencia o commit:

`279ba3e9b084020c23c36e63bec933f1dbc705e6`

O HEAD operacional observado foi:

`d8e89bb61da3d54936b17e2f6185f309792ef771`

Entre a release v2.5 e o HEAD observado não há diferenças nos escopos
Python, documentação científica, contratos ou dados. A diferença
commitada posterior à release pertence à operação do runner.

## Papel do legado

O Sister-Clima não é dependência de runtime do Atmos.

Durante a migração ele atua como oráculo temporário de regressão para
comportamentos científicos selecionados.

## Achados

O contrato espacial e os testes existentes revelaram invariantes sobre:

- identidade e escala dos produtos meteorológicos;
- deduplicação de células nativas;
- coerência temporal;
- semântica de superfícies meteorológicas;
- distinção entre ERA5 consolidado e Best Match operacional;
- agregação ponderada por área;
- proibição de soma espacial de precipitação em milímetros;
- proveniência explícita;
- uso de H3 como índice, amostragem e síntese, nunca como aumento de
  resolução meteorológica.

## Próximo passo

Converter os comportamentos selecionados em Golden Cases independentes
da implementação Python e utilizá-los posteriormente contra o núcleo
C++23 do Atmos.
