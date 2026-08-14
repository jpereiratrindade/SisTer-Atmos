# A1-E002 — Product Selection & Temporal Coverage

## Propósito

Constituir a semântica científica para identidade de produto climático,
decisão de seleção e cobertura temporal efetiva antes da introdução de
providers, rede, persistência, H3 ou frontend.

## Product Selection

Uma análise deve manter separadamente:

- produto solicitado;
- produto efetivamente utilizado;
- semântica da seleção;
- estado de fallback;
- razão científica da seleção.

Produto solicitado e produto utilizado não são sinônimos.

Seleção ou fallback nunca pode apagar a identidade do produto efetivamente
utilizado.

### GC-006

GC-006 estabelece que, para precipitação consolidada:

- ERA5 é selecionado diretamente;
- `fallback_used` é falso;
- a semântica é `direct_product_selection`;
- a razão científica da escolha permanece explícita.

GC-006 não constitui evidência de um caminho de fallback positivo.

## Temporal Coverage

Uma análise deve manter separadamente:

- período solicitado;
- datas disponíveis;
- datas ausentes;
- período efetivamente coberto;
- completude.

Ao combinar múltiplas células meteorológicas nativas únicas, a cobertura
temporal efetiva é formada pelas datas disponíveis em todas as células
participantes.

### GC-007

Para o período solicitado de 2026-07-01 a 2026-07-03:

- 2026-07-01 está disponível em A e B;
- 2026-07-02 está disponível em A e B;
- 2026-07-03 está ausente em A;
- o período efetivo termina em 2026-07-02;
- há dois dias disponíveis;
- 2026-07-03 permanece explicitamente ausente;
- a cobertura é incompleta.

## Consistência analítica

Produto utilizado e período efetivo pertencem ao contexto científico da
análise.

Visões analíticas posteriores devem consumir esse contexto comum, sem
reconstruí-lo independentemente.

A1-E002 não implementa essas visões.

## Invariantes

- N04;
- N07;
- N08;
- N12;
- N17.

## Fora do escopo

A1-E002 não inclui:

- GC-004;
- GC-005;
- GC-008;
- providers meteorológicos reais;
- HTTP;
- persistência;
- H3;
- superfícies meteorológicas;
- frontend;
- integração operacional com Nexo;
- fallback positivo ainda não coberto por Golden Case governado.

## Critério de verificabilidade

A futura implementação C++23 deverá reproduzir GC-006 e GC-007, preservar os
invariantes declarados e permanecer determinística, sem I/O oculto ou
dependências operacionais externas.
