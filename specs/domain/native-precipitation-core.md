# Native Precipitation Core — A1-E001

## Propósito

Constituir o primeiro núcleo científico executável do SisTer Atmos
independente do Sister-Clima.

## Conceitos

### Produto meteorológico

Identifica semanticamente a origem/modelo da célula meteorológica.

No domínio interno deve ser representado por tipo enumerado, não por
string arbitrária.

### Célula meteorológica nativa

É identificada por:

- produto meteorológico;
- latitude nativa;
- longitude nativa.

A identidade usa as coordenadas retornadas e validadas do produto.

Não há deduplicação por distância ou tolerância espacial implícita.

### Precipitação

Grandeza em milímetros.

Deve ser:

- finita;
- maior ou igual a zero.

### Área representada

Grandeza usada exclusivamente para ponderação espacial.

Deve ser:

- finita;
- estritamente maior que zero.

## Operação 1 — deduplicação

Entrada:

uma sequência de amostras meteorológicas nativas válidas.

Para amostras com a mesma identidade de célula:

- valores de precipitação iguais representam uma única observação
  espacial;
- `sample_count` registra quantas consultas originaram aquela
  observação;
- valores divergentes invalidam o conjunto.

A ordem das consultas não altera o resultado científico.

### Relação

GC-001.

Invariantes:

- N03;
- N13.

## Operação 2 — conflito

Duas amostras com a mesma identidade de célula nativa e precipitação
divergente constituem conflito científico.

A implementação deve rejeitar o conjunto.

Não é permitido:

- usar a primeira;
- usar a última;
- calcular média;
- escolher arbitrariamente uma observação.

### Relação

GC-002.

Invariante:

- N14.

## Operação 3 — média ponderada pela área

Para contribuições válidas:

P = Σ(P_i × A_i) / Σ(A_i)

onde:

- `P_i` é a precipitação da célula nativa;
- `A_i` é a área efetivamente representada.

O resultado deve registrar:

- precipitação ponderada;
- área total representada;
- número de contribuintes.

A soma espacial direta dos milímetros não é um resultado válido.

### Relação

GC-003.

Invariantes:

- N02;
- N15.

## Política numérica

A implementação deve:

- rejeitar grandezas não finitas;
- evitar perda desnecessária de precisão durante acumulação;
- verificar resultado antes de converter para a representação pública;
- não empregar epsilon arbitrário para definir identidade de célula.

## Não objetivos do A1-E001

Não fazem parte desta implementação:

- H3;
- geometria;
- interseção espacial real;
- GeoJSON;
- providers;
- HTTP;
- persistência;
- séries temporais;
- Best Match;
- NASA POWER;
- interface.
