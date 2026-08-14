# A-004 — Horizonte Computacional e de Dados do SisTer Atmos

## Status

Decisão arquitetural prospectiva.

Este documento registra capacidades tecnológicas aprovadas para adoção,
experimentação ou avaliação futura.

Ele NÃO transforma essas tecnologias em dependências do A1-E001.

## 1. Princípio fundamental

A sequência de decisão do SisTer Atmos é:

domínio científico
→ requisito mensurável
→ hipótese tecnológica
→ experimento
→ evidência
→ adoção

Nunca:

tecnologia disponível
→ procurar onde utilizá-la.

O radar arquitetural existe para impedir tanto a adoção prematura quanto
a descoberta tardia de que decisões estruturais bloquearam uma evolução
cientificamente necessária.

## 2. Estados do radar

### ADOPT

Direção arquitetural aprovada.

Uma tecnologia em ADOPT pode ainda não estar instalada ou vinculada ao
runtime atual.

A ativação depende do marco que efetivamente necessite dela.

### TRIAL

Tecnologia autorizada para experimento governado.

Sua incorporação exige:

- hipótese explícita;
- benchmark ou caso científico;
- critério de sucesso;
- evidência;
- decisão posterior.

### ASSESS

Tecnologia considerada relevante, mas ainda sem evidência suficiente
para experimento ou adoção.

### HOLD

Tecnologia explicitamente fora do caminho atual.

HOLD não significa rejeição permanente.

## 3. Três significados distintos de vetor

O Atmos deve preservar três conceitos independentes.

### 3.1 Vetor ou matriz numérica

Usado para:

- séries temporais;
- matrizes espaço × tempo;
- grades meteorológicas;
- variáveis multivariadas;
- operações científicas vetorizáveis.

### 3.2 Geometria vetorial SIG

Usada para:

- pontos;
- linhas;
- polígonos;
- pixels representados como geometria;
- territórios;
- interseções e coberturas.

### 3.3 Vetor de características ou embedding

Usado para:

- assinatura climática;
- similaridade ambiental;
- representação semântica;
- recuperação por proximidade.

Esses três conceitos não compartilham um tipo genérico `Vector`.

A semelhança terminológica não implica equivalência de domínio.

## 4. Computação científica C++23

### 4.1 Tipos fortes — ADOPT

Grandezas científicas devem possuir semântica explícita no sistema de
tipos.

Exemplos:

- Latitude;
- Longitude;
- PrecipitationMm;
- RepresentedArea;
- TemperatureCelsius;
- TemporalResolution;
- ClimateProduct.

### 4.2 `std::expected` — ADOPT

Rejeições esperadas do domínio devem utilizar resultados explícitos e
tipados quando isso representar melhor a semântica da operação.

Exceções não constituem fluxo normal para violações científicas
esperadas.

### 4.3 `std::span` — ADOPT

Algoritmos científicos podem receber views não proprietárias sobre
sequências contíguas quando ownership externo for explícito.

### 4.4 `std::mdspan` — TRIAL

`std::mdspan` é candidato preferencial para views multidimensionais
não proprietárias sobre estruturas como:

- tempo × célula;
- tempo × latitude × longitude;
- célula × variável;
- produto × tempo × célula.

Sua adoção operacional depende de:

- suporte efetivo da toolchain;
- ergonomia;
- benchmarks;
- compatibilidade com os layouts utilizados.

O armazenamento proprietário permanece separado da view.

### 4.5 Ranges e algoritmos — ADOPT

APIs devem favorecer composição clara, views e algoritmos da biblioteca
padrão quando isso melhorar legibilidade e segurança sem esconder o
significado científico.

### 4.6 Data-oriented design — ADOPT

Estruturas de grandes volumes devem considerar:

- localidade de cache;
- dados contíguos;
- separação entre ownership e views;
- AoS versus SoA conforme benchmark;
- minimização de alocações transitórias.

A escolha de layout deve ser medida, não presumida.

### 4.7 Vetorização CPU — TRIAL

O código deve inicialmente favorecer:

- loops simples;
- layouts contíguos;
- ausência de aliasing desnecessário;
- otimização do compilador;
- medição objetiva.

SIMD explícito somente será promovido após benchmark demonstrar ganho
relevante sem comprometer portabilidade ou correção.

### 4.8 Paralelismo — ASSESS

Paralelismo deve entrar somente em operações:

- suficientemente grandes;
- determinísticas ou com política de redução explicitada;
- mensuravelmente limitadas por CPU.

Resultados científicos não podem depender da ordem acidental de
execução concorrente.

### 4.9 GPU e aceleradores — HOLD

CUDA, HIP, SYCL, Kokkos ou outras soluções não integram o baseline
atual.

Entretanto, layouts e APIs não devem deliberadamente impedir futura
experimentação com aceleradores.

## 5. Grafos

Grafos são tratados como estrutura de domínio antes de serem tratados
como escolha de banco.

### 5.1 Grafo de vizinhança espacial — ASSESS

Possíveis relações:

célula → células vizinhas

Aplicações potenciais:

- conectividade;
- propagação;
- agrupamento;
- continuidade espacial;
- análise de extremos espacialmente conectados.

### 5.2 Grafo territorial — ASSESS

Possíveis relações:

município
→ COREDE
→ Região Funcional
→ classificações ambientais

Uma entidade pode participar de múltiplas relações sem que isso exija
uma hierarquia única rígida.

### 5.3 DAG de proveniência — TRIAL

A cadeia:

fonte
→ consulta
→ produto
→ transformação
→ indicador
→ análise
→ evidência

é naturalmente representável como grafo acíclico dirigido quando as
transformações forem imutáveis.

O modelo de proveniência deve permitir rastrear exatamente de quais
entradas e transformações um resultado científico deriva.

### 5.4 Grafo em memória — TRIAL

Representações C++ simples devem ser preferidas inicialmente:

- IDs tipados;
- listas de adjacência;
- arestas tipadas;
- estruturas imutáveis quando adequado.

Uma biblioteca externa de grafos só será incorporada após necessidade
demonstrada.

### 5.5 Banco orientado a grafos — HOLD

Nenhum graph database faz parte da arquitetura operacional atual.

Persistência relacional de nós e relações é suficiente até que um
padrão de consulta demonstre objetivamente a necessidade de engine de
grafos.

Apache AGE permanece em HOLD enquanto houver desalinhamento relevante
com a versão estratégica do PostgreSQL ou ausência de necessidade
demonstrada.

## 6. PostgreSQL

### 6.1 PostgreSQL — ADOPT

PostgreSQL é a direção aprovada para persistência transacional e
metadados científicos do Atmos.

Sua ativação ocorrerá em marco posterior ao núcleo científico puro.

Responsabilidades candidatas:

- catálogos;
- análises;
- produtos;
- proveniência;
- metadados;
- identidade de datasets;
- vínculos com pesquisa;
- índices e referências espaciais.

## 7. PostGIS

### 7.1 PostGIS Geometry/Geography — ADOPT

PostGIS é a direção aprovada para persistência e consulta espacial
quando a camada de dados for constituída.

Casos candidatos:

- limites municipais;
- unidades territoriais;
- geometrias de pixels nativos;
- células H3 materializadas quando necessário;
- interseções;
- coberturas;
- índices espaciais.

O domínio científico permanece independente da extensão.

### 7.2 PostGIS Raster — ASSESS

Raster não será habilitado automaticamente.

Deverá ser comparado com:

- geometria de pixels;
- arquivos colunares;
- formatos científicos externos;
- processamento C++.

Critérios incluem:

- volume;
- custo de consulta;
- reprodutibilidade;
- interoperabilidade;
- facilidade de versionamento.

### 7.3 PostGIS Topology — ASSESS

Topology será considerada somente caso surjam requisitos de integridade
topológica que não sejam bem atendidos por geometrias convencionais.

## 8. pgvector

### 8.1 Embeddings semânticos — TRIAL

Possíveis aplicações futuras:

- recuperação de documentação;
- semelhança entre análises;
- pesquisa em evidências;
- apoio semântico ao Nexo.

Embedding sem proveniência, modelo e versão é inválido como evidência
científica.

### 8.2 Assinaturas climáticas — TRIAL

O Atmos pode representar uma observação territorial ou temporal por um
vetor de características científicas.

Exemplo conceitual:

[
  precipitação acumulada,
  sazonalidade,
  anomalia,
  frequência de extremos,
  duração de períodos secos,
  temperatura,
  déficit hídrico
]

Isso pode permitir consultas como:

- períodos historicamente análogos;
- territórios climaticamente semelhantes;
- agrupamentos de padrões ambientais.

### 8.3 Regra científica para similaridade

Nenhuma busca vetorial é válida sem declarar:

- features;
- unidades;
- transformação;
- normalização;
- tratamento de ausentes;
- métrica;
- versão da assinatura.

A proximidade matemática não deve ser apresentada automaticamente como
semelhança científica.

### 8.4 Índices aproximados — TRIAL

HNSW e IVFFlat podem ser avaliados quando houver volume que justifique
busca aproximada.

Resultados aproximados devem possuir critérios explícitos de recall,
latência e reprodutibilidade.

## 9. Arrow e Parquet

### 9.1 Apache Arrow — ASSESS/TRIAL

Arrow é candidato para representação colunar eficiente e interoperável
de grandes conjuntos analíticos.

Possíveis usos:

- batches de observações;
- intercâmbio entre componentes;
- processamento colunar;
- integração futura com ferramentas científicas.

### 9.2 Apache Parquet — ASSESS/TRIAL

Parquet é candidato para armazenamento de grandes datasets imutáveis ou
versionados.

Possível divisão futura:

PostgreSQL/PostGIS
→ catálogo, identidade, geometria, relações e proveniência

Parquet
→ grandes séries, matrizes e produtos analíticos

Essa divisão somente será adotada após benchmark e avaliação
operacional.

## 10. Arquitetura de dados candidata

```text
                 SisTer Atmos
                      │
             domínio científico
                      │
          ┌───────────┼───────────┐
          │           │           │
      PostgreSQL    PostGIS    Arrow/Parquet
          │           │           │
     metadados     geometrias    grandes dados
     proveniência  relações      colunares
          │           │           │
          └──────┬────┴────┬──────┘
                 │         │
              pgvector   grafos
                 │         │
           similaridade  relações
```

Nenhuma seta no diagrama anterior implica dependência do núcleo
científico puro.

## 11. Segurança da camada de dados

Quando a persistência for ativada:

- privilégios mínimos;
- roles separadas por responsabilidade;
- migrations versionadas;
- queries parametrizadas;
- nenhuma construção SQL por concatenação de entrada externa;
- extensões permitidas por allowlist;
- schema de aplicação separado de schemas de extensões quando
  apropriado;
- credenciais fora do repositório;
- TLS quando a fronteira de implantação exigir;
- backups e restauração testados;
- proveniência e auditoria preservadas.

Extensões PostgreSQL não são habilitadas apenas porque estão
disponíveis.

## 12. Segurança científica

Além de segurança computacional, o Atmos protege interpretação
científica.

O sistema deve impedir ou tornar explícitos:

- mistura silenciosa de unidades;
- comparação de produtos incompatíveis;
- interpolação não declarada;
- ganho fictício de resolução;
- agregação espacial inadequada;
- perda de proveniência;
- comparação de períodos incompatíveis;
- similaridade vetorial sem especificação de features.

## 13. Critérios de promoção

Uma tecnologia muda de estado somente quando houver evidência.

### ASSESS → TRIAL

Exige:

- caso científico identificado;
- hipótese explícita;
- fixture ou dataset representativo;
- métrica de avaliação.

### TRIAL → ADOPT

Exige:

- experimento reproduzível;
- ganho demonstrado;
- riscos conhecidos;
- impacto operacional avaliado;
- segurança avaliada;
- decisão registrada.

### Qualquer estado → HOLD

Pode ocorrer quando:

- benefício não for demonstrado;
- custo operacional for excessivo;
- compatibilidade for insuficiente;
- risco científico superar o benefício.

## 14. Não objetivos

Este documento não autoriza no A1-E001:

- linkagem com PostgreSQL;
- criação de banco;
- uso de PostGIS;
- uso de pgvector;
- dependência Arrow;
- biblioteca de grafos;
- execução paralela;
- GPU.

O A1-E001 continua sendo um núcleo C++23 pequeno, determinístico e
independente de infraestrutura.

## 15. Fontes técnicas verificadas

As decisões prospectivas deste documento devem permanecer vinculadas
a documentação técnica primária.

Referências de base:

- WG21 P0009R18 — `std::mdspan`;
- WG21 — biblioteca padrão C++23 e `std::expected`;
- PostgreSQL — política de versionamento e release notes;
- PostGIS — documentação oficial;
- pgvector — documentação oficial do projeto;
- Apache Arrow — formato colunar e implementação C++;
- Apache Parquet — implementação C++;
- Apache AGE — documentação oficial de compatibilidade.

URLs e versões concretas devem ser revistas no momento da promoção de
cada tecnologia de TRIAL/ASSESS para ADOPT, evitando congelar no
documento arquitetural informação operacional que pode envelhecer.

## 16. Regra de revisão

O radar deve ser revisto quando:

- um novo marco científico exigir capacidade não prevista;
- uma tecnologia em TRIAL produzir evidência;
- uma dependência estratégica mudar de versão principal;
- surgir incompatibilidade relevante;
- benchmarks invalidarem uma hipótese;
- houver nova restrição de segurança;
- uma tecnologia deixar de ser mantida adequadamente.

A revisão do radar não altera silenciosamente dependências do runtime.

Mudanças de estado devem ser explícitas, versionadas e justificadas.
