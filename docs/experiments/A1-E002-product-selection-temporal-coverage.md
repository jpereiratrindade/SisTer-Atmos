# A1-E002 — Product Selection & Temporal Coverage

## Estado

**Constituído como proposta. Ainda não autorizado para implementação.**

Baseline verificado:

- marco: `A1-E001`;
- commit: `7dc0115`.

## Perguntas científicas

1. Produto solicitado e produto utilizado permanecem distinguíveis?
2. A semântica e a razão da seleção permanecem explícitas?
3. O estado de fallback pode ser representado sem confundir seleção direta e
   fallback?
4. Período solicitado e período efetivamente coberto permanecem distinguíveis?
5. Lacunas temporais permanecem explícitas?
6. A cobertura comum utiliza apenas datas presentes em todas as células
   nativas únicas participantes?
7. Produto utilizado e período efetivo podem formar um contexto científico
   único reutilizável?

## Golden Cases

- `GC-006` — seleção direta de ERA5;
- `GC-007` — cobertura temporal efetiva comum.

## Invariantes

- `N04`;
- `N07`;
- `N08`;
- `N12`;
- `N17`.

## Limitações deliberadas

GC-006 possui `fallback_used=false`; portanto não comprova um caminho positivo
de fallback.

N17 é tratado neste marco pela constituição de um contexto compartilhado de
produto e período efetivo. Série, indicadores, mapa e frontend continuam fora
do escopo.

## Critérios futuros de promoção

A1-E002 somente poderá tornar-se `verified` após:

- autorização explícita;
- implementação nativa C++23 de GC-006 e GC-007;
- ligação dos invariantes aos testes;
- warnings-as-errors;
- ASan e UBSan;
- preservação integral de A1-E001;
- gate próprio em PASS;
- evidência governada;
- checksum da evidência.

## Autorização

**Pendente.**

A existência deste documento constitui o experimento, mas não autoriza sua
implementação.
