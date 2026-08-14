# A1-E003 — Spatial Representation & Operational Sampling

## Propósito

Fechar a semântica espacial do núcleo científico de precipitação antes da
introdução de providers reais, biblioteca H3, HTTP, persistência ou frontend.

A1-E003 é a última fatia planejada do A1 e deve tornar nativos os Golden Cases
GC-004, GC-005 e GC-008.

## Amostragem operacional não é superfície meteorológica

No modo operacional, pontos Best Match únicos representam amostras de consulta.
A síntese espacial é a média simples dos pontos únicos participantes.

Essa média:

- não representa média areal;
- não cria pixel meteorológico nativo;
- não autoriza reconstruir superfície onde a geometria nativa não é conhecida.

### GC-004

Para duas amostras operacionais únicas com 10 mm e 30 mm:

- existem dois pontos efetivos;
- a precipitação representativa é 20 mm;
- a agregação é média simples;
- a representação deve declarar que não é uma superfície de pixels nativos.

## Elegibilidade de superfície

Uma superfície meteorológica somente é admissível quando a geometria da grade
nativa do produto está explicitamente representada.

### GC-005

Produto pontual sem geometria nativa conhecida deve rejeitar pedido de
superfície H3 com erro científico explícito `native_geometry_required`.

H3 não cria resolução meteorológica e não transforma ponto em pixel nativo.

## Representação H3 e agregação espacial

A1-E003 governa a semântica de agrupamento para apresentação sem exigir uma
biblioteca H3 no núcleo. O identificador de bucket pode ser opaco nos testes; a
regra científica não depende da implementação do índice.

### GC-008

Quando dois acumulados pontuais de 10 mm e 30 mm pertencem ao mesmo bucket de
apresentação:

- pontos monitorados = 2;
- média espacial = 20 mm;
- soma espacial de 40 mm é proibida.

Precipitação em milímetros pode ser acumulada no tempo em intervalos
compatíveis, mas não deve ser somada no espaço como quantidade extensiva.

## Resolução de amostragem versus resolução de apresentação

A resolução utilizada para escolher/coletar pontos meteorológicos e a resolução
utilizada para apresentar resultados territorialmente são conceitos distintos.
Uma não deve ser inferida da outra.

Se uma estratégia futura adaptar a resolução de amostragem, a adaptação deve
ser registrada explicitamente na proveniência. A1-E003 não obriga a existência
de algoritmo adaptativo; obriga que ele não possa ser silencioso quando existir.

## Identidade espacial e fronteira com providers

O núcleo A1-E003 recebe coordenadas já canônicas. Não fará comparação fuzzy ou
normalização implícita de coordenadas para decidir identidade espacial.

Normalização, quantização ou identidade específica de um provider pertencem à
futura camada de provider e devem ser constituídas antes de dados reais de
fontes distintas serem combinados.

Essa fronteira preserva a deduplicação determinística do núcleo sem transformar
uma decisão de integração em regra científica acidental.

## Invariantes

- N02 — precipitação em mm não é somável espacialmente;
- N05 — resolução de amostragem e apresentação são distintas;
- N06 — NASA POWER não produz superfície H3 sem grade MERRA-2 explícita;
- N09 — H3 não cria resolução meteorológica;
- N10 — superfície exige geometria nativa conhecida;
- N16 — modo operacional usa média simples de amostras únicas;
- N18 — adaptação de amostragem deve aparecer na proveniência.

## Fora do escopo

A1-E003 não inclui:

- provider meteorológico real;
- escolha de protocolo de aquisição;
- biblioteca H3 de runtime;
- reconstrução completa de grade ERA5/MERRA-2;
- persistência;
- HTTP/API;
- frontend;
- autenticação;
- integração operacional com Nexo;
- normalização entre coordenadas provenientes de providers diferentes.

## Critério de verificabilidade

A futura implementação C++23 deverá reproduzir GC-004, GC-005 e GC-008,
preservar os invariantes declarados, permanecer determinística e manter a
fronteira de prova explícita.
