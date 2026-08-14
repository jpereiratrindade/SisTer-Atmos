# A-003 — Política de engenharia C++23 do SisTer Atmos

## Objetivo

O código científico do SisTer Atmos deve privilegiar correção,
auditabilidade, comportamento determinístico e estados válidos por
construção.

A implementação não deve apenas reproduzir resultados do legado.
Ela deve tornar explícitas, no sistema de tipos e nas APIs, as
restrições científicas que sustentam esses resultados.

## 1. Fronteira entre domínio e infraestrutura

O núcleo científico não pode depender de:

- HTTP;
- filesystem;
- banco de dados;
- interface gráfica;
- H3;
- providers meteorológicos;
- SisTer-Nexo;
- Python;
- Sister-Clima.

Essas dependências pertencem às camadas externas.

## 2. Tipos de domínio

Grandezas semanticamente diferentes não devem ser representadas
indistintamente apenas por `double`.

O núcleo deve utilizar tipos explícitos para conceitos como:

- latitude;
- longitude;
- precipitação em milímetros;
- área representada;
- produto meteorológico;
- identidade de célula meteorológica nativa.

Sempre que possível, valores inválidos devem ser impossíveis após
a construção do tipo.

## 3. Validação numérica

Entradas numéricas provenientes de fronteiras externas devem rejeitar:

- NaN;
- infinito;
- latitude fora de [-90, 90];
- longitude fora de [-180, 180];
- precipitação negativa;
- área não finita;
- área menor ou igual a zero.

Operações agregadoras devem verificar também overflow e resultados
não finitos antes de produzir valores de domínio.

## 4. Tratamento de erros

Rejeições previstas pelo domínio devem ser explícitas e tipadas.

O A1 utiliza `std::expected` para resultados que podem falhar por
violação de regra científica.

Exceções não são usadas como fluxo normal para rejeições do domínio.

Falhas extraordinárias de infraestrutura ou de alocação permanecem
fora desse contrato e não são artificialmente convertidas em códigos
de domínio.

## 5. Propriedade e memória

Aplicam-se as seguintes regras:

- RAII;
- nenhum `new` ou `delete` manual;
- nenhum ponteiro proprietário cru;
- entradas não proprietárias podem usar `std::span`;
- ownership deve ser evidente pela assinatura da API.

## 6. Determinismo

O núcleo científico deve produzir o mesmo resultado para a mesma
entrada válida.

Não deve depender de:

- relógio;
- rede;
- variáveis globais mutáveis;
- ordem não especificada de infraestrutura;
- locale implícito.

Identidades de células meteorológicas devem possuir comparação
determinística.

## 7. Conversão implícita

Tipos científicos devem evitar conversões implícitas capazes de
misturar grandezas semanticamente distintas.

Construtores de tipos fortes devem ser explícitos ou controlados por
fábricas validadoras.

## 8. Contratos do A1-E001

O A1-E001 implementa inicialmente:

- GC-001 — deduplicação de célula nativa;
- GC-002 — rejeição de valor divergente para a mesma célula;
- GC-003 — precipitação ponderada por área representada.

O código não deve conter condicionais especiais para IDs de Golden
Cases.

Os Golden Cases verificam comportamento; não constituem algoritmo.

## 9. Compilação

Código próprio deve ser compilado com conjunto rigoroso de warnings.

Warnings do código SisTer Atmos devem ser tratados como erros durante
os gates de desenvolvimento.

Dependências externas futuras não devem herdar automaticamente essa
política.

## 10. Sanitização

O gate de engenharia deve executar build separado com:

- AddressSanitizer;
- UndefinedBehaviorSanitizer;
- frame pointers preservados para diagnóstico.

A ausência de achados dos sanitizers faz parte do aceite do marco.

## 11. Testes

Além dos Golden Cases, testes devem cobrir condições adversas e
fronteiras numéricas.

No mínimo:

- coordenada não finita;
- coordenada fora do domínio;
- precipitação negativa;
- área zero;
- área negativa;
- entrada vazia;
- duplicata consistente;
- duplicata divergente.

## 12. Evolução

Esta política pode ser endurecida conforme novos riscos surgirem.

Relaxamentos de segurança ou de validação exigem decisão
arquitetural explícita e evidência correspondente.
