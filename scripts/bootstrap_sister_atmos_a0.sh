#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="${SISTER_ATMOS_ROOT:-/run/media/jpereiratrindade/labeco10T/dev/cpp/sister-atmos}"
PROJECT_ID="sister_atmos"
PROJECT_NAME="SisTer Atmos"

log()  { printf '\n==> %s\n' "$*"; }
pass() { printf '[PASS] %s\n' "$*"; }
die()  { printf '[ERRO] %s\n' "$*" >&2; exit 1; }

require_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "comando obrigatório não encontrado: $1"
}

require_cmd git
require_cmd cmake
require_cmd c++

log "SisTer Atmos A0 — bootstrap constitucional"
printf 'Destino: %s\n' "$ROOT"

# Proteção contra sobrescrever um diretório que pertença a outro projeto.
if [[ -d "$ROOT" ]]; then
  existing_non_git="$(find "$ROOT" -mindepth 1 -maxdepth 1 ! -name .git -print -quit 2>/dev/null || true)"
  if [[ -n "$existing_non_git" ]]; then
    if [[ ! -f "$ROOT/.hoa/project.yaml" ]] || ! grep -q '^id: sister_atmos$' "$ROOT/.hoa/project.yaml"; then
      die "o destino já contém arquivos e não foi reconhecido como SisTer Atmos: $ROOT"
    fi
  fi
fi

mkdir -p "$ROOT"
cd "$ROOT"

if [[ ! -d .git ]]; then
  git init -b main
  pass "repositório Git inicializado"
else
  pass "repositório Git já existente"
fi

log "Criando estrutura A0"
mkdir -p \
  .hoa \
  domain \
  specs/domain \
  specs/contracts \
  specs/invariants \
  docs/architecture \
  docs/adr \
  docs/lineage \
  docs/experiments/evidence/a0-baseline \
  harness/A0-constitution/scenarios \
  contracts \
  include/sister/atmos \
  src \
  tests \
  scripts

cat > .gitignore <<'EOF'
/build/
/.build/
/.run/
CMakeUserPresets.json
compile_commands.json
*.swp
*.tmp
EOF

cat > .hoa/project.yaml <<'EOF'
version: "1.0"
id: sister_atmos
kind: sister_subsystem
phase: A0
purpose: "Inteligência climática e territorial para pesquisa."
identity:
  name: "SisTer Atmos"
  repository: "sister-atmos"
  role: "subsystem"
  domain: "climate_intelligence"
  language: "C++23"
commitment:
  - "domínio antes da implementação"
  - "equivalência comportamental antes da substituição"
  - "proveniência científica explícita"
  - "Nexo-aware, não Nexo-owned"
boundaries:
  owns:
    - "produtos e análises climáticas"
    - "transformações espaciais climáticas"
    - "proveniência das análises Atmos"
  does_not_own:
    - "ciclo de vida de projetos de pesquisa"
    - "identidade institucional"
    - "governança de evidências do Nexo"
    - "infraestrutura de implantação do ecossistema"
reflexivity:
  loop:
    - purpose
    - expected_state
    - observed_state
    - difference
    - interpretation
    - recommendation
    - authorized_decision
    - action
    - new_observation
    - learning
runtime_dependency_on_praxis: false
EOF

cat > .hoa/atmosphere-a0.yaml <<'EOF'
version: "1.0"
project: sister_atmos
phase: A0
title: "Constituição do SisTer Atmos"
status: scaffolded
intent:
  - "registrar identidade e fronteiras antes do porte climático"
  - "preservar Sister-Clima 2.5 como referência comportamental temporária"
  - "registrar invariantes científicos antes de providers e infraestrutura"
  - "estabelecer a fronteira Atmos ↔ Nexo por contrato"
forbidden_in_a0:
  - "portar providers meteorológicos"
  - "implementar H3"
  - "implementar persistência PostgreSQL"
  - "reproduzir frontend"
  - "acoplar runtime ao Praxis"
gates:
  - identity
  - architecture
  - cpp23
  - cmake
  - ctest
  - scientific_contract
  - sister_clima_lineage
  - nexo_boundary
  - praxis_scaffold
EOF

cat > domain/PURPOSE.md <<'EOF'
# Propósito

O SisTer Atmos produz inteligência climática e territorial rastreável para pesquisa.

A unidade de valor do sistema não é um painel nem uma chamada a um provider:
é uma análise climática cuja transformação científica, cobertura espacial,
período efetivo, produto efetivamente usado e proveniência permaneçam explícitos.
EOF

cat > domain/CAPABILITIES.md <<'EOF'
# Capacidades

Capacidades previstas, não implementadas em A0:

- consultar produtos meteorológicos;
- construir séries temporais climáticas;
- realizar amostragem e agregação espacial;
- gerar indicadores derivados;
- produzir camadas territoriais;
- registrar proveniência;
- exportar análises para uso científico e integração com o Nexo.
EOF

cat > domain/BOUNDARIES.md <<'EOF'
# Fronteiras

O Atmos possui o domínio de inteligência climática e territorial.

Não possui:

- o ciclo de vida de projetos;
- a identidade de usuários do ecossistema;
- a governança de pesquisa do Nexo;
- a implantação do ecossistema;
- a memória/reflexão operacional do Praxis.

Praxis pode governar o processo de construção do Atmos sem tornar-se dependência
do runtime do Atmos.
EOF

cat > domain/AUTHORITY.md <<'EOF'
# Autoridade

Decisões científicas sobre interpretação, agregação, produtos meteorológicos,
períodos e proveniência pertencem ao domínio Atmos e devem ser justificadas
por especificações, invariantes, testes e evidências.

O Nexo pode solicitar e registrar análises, mas não redefine silenciosamente
as regras científicas internas do Atmos.
EOF

cat > domain/REFLEXIVITY.md <<'EOF'
# Reflexividade

A construção do Atmos seguirá o ciclo governado pelo Praxis:

propósito → estado esperado → estado observado → diferença → interpretação →
recomendação → decisão autorizada → ação → nova observação → aprendizagem.

A memória reflexiva é evidência de engenharia; não é arquitetura de runtime.
EOF

cat > domain/VOCABULARY.md <<'EOF'
# Vocabulário

**produto solicitado** — produto meteorológico pedido pelo usuário ou sistema.

**produto utilizado** — produto efetivamente empregado na análise.

**período solicitado** — janela temporal pedida.

**período efetivo** — janela efetivamente coberta pelos dados válidos.

**perfil espacial** — estratégia de amostragem, resolução e apresentação espacial.

**proveniência** — informação suficiente para explicar de onde vieram os dados e
como foram transformados.

**análise climática** — resultado científico rastreável produzido pelo Atmos.
EOF

cat > specs/domain/precipitation.md <<'EOF'
# Precipitação

Precipitação acumulada é aditiva no tempo quando os intervalos são compatíveis.

Precipitação expressa como lâmina (mm) não é uma quantidade extensiva no espaço:
não se somam espacialmente valores de mm de células distintas para representar
a precipitação média de um território.

A regra espacial deve explicitar o método de agregação adotado.
EOF

cat > specs/domain/climate-product.md <<'EOF'
# Produto climático

Um produto climático possui identidade própria.

O sistema deve manter distinguíveis:

- produto solicitado;
- produto efetivamente utilizado;
- provider;
- resolução temporal;
- resolução espacial quando conhecida;
- transformações aplicadas.

Fallbacks não podem apagar a identidade do produto efetivamente utilizado.
EOF

cat > specs/domain/spatial-profile.md <<'EOF'
# Perfil espacial

A resolução usada para amostragem meteorológica não é necessariamente a mesma
resolução usada para apresentação ou indexação territorial.

Deduplicação deve ocorrer na unidade espacial apropriada ao dado meteorológico,
evitando múltiplas consultas equivalentes para a mesma coordenada/grade.
EOF

cat > specs/domain/provenance.md <<'EOF'
# Proveniência

Toda análise climática deverá ser capaz de distinguir, no mínimo:

- solicitação original;
- produto solicitado;
- produto utilizado;
- provider;
- período solicitado;
- período efetivo;
- transformação científica;
- método espacial;
- versão/identidade do Atmos responsável pelo resultado.

A proveniência faz parte do resultado científico, não é mero log operacional.
EOF

cat > specs/invariants/precipitation-spatial-contract.yaml <<'EOF'
version: "1.0"
domain: precipitation_spatial_contract
status: baseline
invariants:
  - id: N01
    statement: "Precipitação acumulada pode ser somada no tempo quando os intervalos são compatíveis."
  - id: N02
    statement: "Precipitação em mm não pode ser somada espacialmente como se fosse quantidade extensiva."
  - id: N03
    statement: "Coordenadas meteorológicas repetidas devem ser deduplicadas antes da coleta equivalente."
  - id: N04
    statement: "ERA5 e Best Match não são produtos intercambiáveis e sua identidade deve permanecer explícita."
  - id: N05
    statement: "Resolução H3 de amostragem e resolução H3 de apresentação são conceitos distintos."
  - id: N06
    statement: "NASA POWER não produz superfície H3 enquanto a grade MERRA-2 não estiver explicitamente representada."
  - id: N07
    statement: "Produto solicitado e produto efetivamente utilizado devem permanecer distinguíveis na proveniência."
  - id: N08
    statement: "Período solicitado e período efetivamente coberto devem permanecer distinguíveis."
EOF

cat > specs/contracts/README.md <<'EOF'
# Contratos

`specs/contracts/` documenta princípios e decisões dos contratos.

Os artefatos de integração consumíveis por outras aplicações ficam em
`/contracts`, mantendo a fronteira entre especificação e interface publicada.
EOF

cat > docs/architecture/A-001-system-boundary.md <<'EOF'
# A-001 — Fronteira do sistema

## Decisão

SisTer Atmos é um subsistema independente do ecossistema SisTer.

Ele possui o domínio de inteligência climática e territorial e pode operar sem
o Nexo no mesmo processo ou banco.

## Consequência

O Atmos pode atender exploração científica própria, automação e integração,
sem transformar o Nexo em hospedeiro de regras climáticas.

O runtime do Atmos também não depende do Praxis.
EOF

cat > docs/architecture/A-002-nexo-integration-boundary.md <<'EOF'
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
EOF

cat > docs/adr/ADR-001-praxis-not-runtime.md <<'EOF'
# ADR-001 — Praxis governa construção, não runtime

**Status:** accepted em A0.

A memória em `.hoa/`, os cenários e as evidências pertencem ao processo
governado de engenharia. Nenhum executável produtivo do Atmos deve depender
desses artefatos para iniciar ou responder análises climáticas.
EOF

cat > docs/lineage/sister-clima-v2.5.md <<'EOF'
# Linhagem — Sister-Clima 2.5

- source: Sister-Clima 2.5
- status: legacy_reference
- replacement: sister-atmos
- migration_mode: behavioral_equivalence
- runtime_relation: external_reference

O Sister-Clima 2.5 será preservado durante a migração como oráculo temporário
de regressão científica e funcional.

Não serão copiados para o Atmos histórico Git, ambientes virtuais, caches ou
dependências incidentais do projeto legado.

A retirada do legado somente ocorrerá após casos dourados suficientes
demonstrarem equivalência ou diferenças cientificamente justificadas.
EOF

cat > harness/A0-constitution/scenarios/README.md <<'EOF'
# Cenários A0

A0 verifica constituição, não comportamento climático.

O primeiro cenário exige:

1. identidade inequívoca;
2. fronteiras explícitas;
3. C++23 configurado;
4. build mínimo reproduzível;
5. CTest mínimo;
6. invariantes científicos registrados;
7. linhagem do Sister-Clima 2.5 registrada;
8. fronteira Nexo registrada;
9. memória Praxis presente sem dependência de runtime.
EOF

cat > contracts/system_manifest.json <<'EOF'
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "system_id": "sister_atmos",
  "name": "SisTer Atmos",
  "repository": "sister-atmos",
  "role": "subsystem",
  "domain": "climate_intelligence",
  "language": "C++23",
  "description": "Inteligência climática e territorial para pesquisa.",
  "phase": "A0",
  "runtime_dependency_on_praxis": false,
  "integration": {
    "nexo": {
      "mode": "contract",
      "ownership": "independent_subsystem"
    }
  }
}
EOF

cat > contracts/atmosphere.openapi.yaml <<'EOF'
openapi: 3.1.0
info:
  title: SisTer Atmos API
  version: 0.0.0-a0
  description: Contrato inicial; endpoints climáticos ainda não implementados em A0.
x-sister-status: draft
paths:
  /api/health:
    get:
      summary: Estado básico do serviço Atmos
      responses:
        "200":
          description: Serviço saudável
          content:
            application/json:
              schema:
                type: object
                required: [system_id, status]
                properties:
                  system_id:
                    const: sister_atmos
                  status:
                    const: ok
EOF

cat > contracts/climate_analysis.schema.json <<'EOF'
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://sister.local/contracts/atmos/climate_analysis.schema.json",
  "title": "SisTer Atmos Climate Analysis",
  "type": "object",
  "required": [
    "analysis_id",
    "requested_product",
    "used_product",
    "requested_period",
    "effective_period",
    "provenance"
  ],
  "properties": {
    "analysis_id": { "type": "string", "minLength": 1 },
    "requested_product": { "type": "string", "minLength": 1 },
    "used_product": { "type": "string", "minLength": 1 },
    "requested_period": { "$ref": "#/$defs/period" },
    "effective_period": { "$ref": "#/$defs/period" },
    "provenance": { "type": "object" }
  },
  "$defs": {
    "period": {
      "type": "object",
      "required": ["start", "end"],
      "properties": {
        "start": { "type": "string", "format": "date" },
        "end": { "type": "string", "format": "date" }
      }
    }
  }
}
EOF

cat > contracts/nexo_research_integration.schema.json <<'EOF'
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://sister.local/contracts/atmos/nexo_research_integration.schema.json",
  "title": "Atmos to Nexo Research Evidence Reference",
  "type": "object",
  "required": ["project_id", "analysis_id", "evidence_kind"],
  "properties": {
    "project_id": { "type": "string", "minLength": 1 },
    "analysis_id": { "type": "string", "minLength": 1 },
    "evidence_kind": { "const": "climate_analysis" },
    "provenance": { "type": "object" },
    "indicators": { "type": "object" },
    "spatial_layer": { "type": ["object", "null"] }
  }
}
EOF

cat > include/sister/atmos/identity.hpp <<'EOF'
#pragma once

#include <string_view>

namespace sister::atmos {

struct Identity {
    std::string_view name;
    std::string_view system_id;
    std::string_view role;
    std::string_view domain;
    std::string_view language;
};

[[nodiscard]] constexpr Identity identity() noexcept {
    return {
        .name = "SisTer Atmos",
        .system_id = "sister_atmos",
        .role = "subsystem",
        .domain = "climate_intelligence",
        .language = "C++23",
    };
}

}  // namespace sister::atmos
EOF

cat > src/identity.cpp <<'EOF'
#include "sister/atmos/identity.hpp"

namespace sister::atmos {

// Unidade de tradução intencionalmente mínima em A0.
// O domínio climático produtivo começa apenas após o baseline científico.

}  // namespace sister::atmos
EOF

cat > src/main.cpp <<'EOF'
#include "sister/atmos/identity.hpp"

#include <iostream>

int main() {
    constexpr auto id = sister::atmos::identity();

    std::cout
        << "{"
        << "\"system_id\":\"" << id.system_id << "\","
        << "\"name\":\"" << id.name << "\","
        << "\"phase\":\"A0\","
        << "\"status\":\"constituted\""
        << "}\n";

    return 0;
}
EOF

cat > tests/identity_tests.cpp <<'EOF'
#include "sister/atmos/identity.hpp"

#include <cstdlib>
#include <iostream>

namespace {

bool expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        return false;
    }
    return true;
}

}  // namespace

int main() {
    constexpr auto id = sister::atmos::identity();

    bool ok = true;
    ok &= expect(id.name == "SisTer Atmos", "nome canônico");
    ok &= expect(id.system_id == "sister_atmos", "system_id canônico");
    ok &= expect(id.role == "subsystem", "papel arquitetural");
    ok &= expect(id.domain == "climate_intelligence", "domínio canônico");
    ok &= expect(id.language == "C++23", "linguagem declarada");

    if (ok) {
        std::cout << "[PASS] identidade SisTer Atmos\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
EOF

cat > CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.25)

project(
  sister_atmos
  VERSION 0.1.0
  DESCRIPTION "SisTer Atmos — inteligência climática e territorial para pesquisa"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(sister_atmos_core STATIC
  src/identity.cpp
)

target_include_directories(sister_atmos_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(sister_atmos_core PUBLIC cxx_std_23)

add_executable(atmos-a0
  src/main.cpp
)

target_link_libraries(atmos-a0 PRIVATE sister_atmos_core)

if(BUILD_TESTING)
  add_executable(sister_atmos_identity_tests
    tests/identity_tests.cpp
  )

  target_link_libraries(sister_atmos_identity_tests
    PRIVATE sister_atmos_core
  )

  add_test(
    NAME sister_atmos_identity_tests
    COMMAND sister_atmos_identity_tests
  )
endif()
EOF

cat > README.md <<'EOF'
# SisTer Atmos

**SisTer Atmos — inteligência climática e territorial para pesquisa.**

- `system_id`: `sister_atmos`
- Linguagem: C++23
- Papel: subsistema independente do ecossistema SisTer.

## Estado atual

**A0 — Constituição.**

A0 registra identidade, fronteiras, linhagem, invariantes científicos,
contratos iniciais e um build C++23 mínimo. Nenhuma regra climática produtiva
é portada nesta fase.

## Princípio de migração

Sister-Clima 2.5 permanece como `legacy_reference` e oráculo temporário de
regressão. O objetivo da migração é equivalência comportamental quando a regra
legada estiver correta, ou diferença explicitamente justificada quando não
estiver.

## Praxis

`.hoa/`, `domain/`, `specs/`, `harness/` e `docs/experiments/` formam a memória
governada de engenharia. O runtime do Atmos não depende do Praxis.

## Verificação A0

```bash
./scripts/verify_a0.sh
```
EOF

cat > scripts/verify_a0.sh <<'EOF'
#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

BUILD_DIR="$ROOT/.build/a0"
RUN_EVIDENCE_FILE="$BUILD_DIR/verification.txt"

mkdir -p "$BUILD_DIR"
: > "$RUN_EVIDENCE_FILE"

exec > >(tee -a "$RUN_EVIDENCE_FILE") 2>&1

on_exit() {
  rc=$?
  if (( rc != 0 )); then
    printf '\nAtmos A0: FAIL (rc=%d)\n' "$rc"
    printf 'Evidência operacional preservada em: %s\n' "$RUN_EVIDENCE_FILE"
  fi
}
trap on_exit EXIT

pass() { printf '[PASS] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

require_file() {
  [[ -f "$1" ]] || fail "arquivo ausente: $1"
}

require_dir() {
  [[ -d "$1" ]] || fail "diretório ausente: $1"
}

require_text() {
  local file="$1"
  local text="$2"
  grep -Fq -- "$text" "$file" || fail "texto esperado ausente em $file: $text"
}

printf '=== SisTer Atmos A0 ===\n'
printf 'root=%s\n' "$ROOT"
printf 'timestamp=%s\n\n' "$(date --iso-8601=seconds)"

require_file .hoa/project.yaml
require_file contracts/system_manifest.json
require_text .hoa/project.yaml "id: sister_atmos"
require_text contracts/system_manifest.json '"system_id": "sister_atmos"'
pass "identidade"

for dir in domain specs/domain specs/contracts specs/invariants docs/architecture docs/lineage contracts include src tests harness; do
  require_dir "$dir"
done
require_file docs/architecture/A-001-system-boundary.md
pass "estrutura arquitetural"

require_file CMakeLists.txt
require_text CMakeLists.txt "set(CMAKE_CXX_STANDARD 23)"
require_text CMakeLists.txt "cxx_std_23"
pass "C++23"

cmake -S . -B "$BUILD_DIR" -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build "$BUILD_DIR"
pass "CMake"

ctest --test-dir "$BUILD_DIR" --output-on-failure
pass "CTest"

require_file specs/invariants/precipitation-spatial-contract.yaml
for invariant in N01 N02 N03 N04 N05 N06 N07 N08; do
  require_text specs/invariants/precipitation-spatial-contract.yaml "id: $invariant"
done
require_text specs/domain/precipitation.md "não é uma quantidade extensiva no espaço"
pass "contrato científico registrado"

require_file docs/lineage/sister-clima-v2.5.md
require_text docs/lineage/sister-clima-v2.5.md "status: legacy_reference"
require_text docs/lineage/sister-clima-v2.5.md "replacement: sister-atmos"
require_text docs/lineage/sister-clima-v2.5.md "migration_mode: behavioral_equivalence"
pass "linhagem Sister-Clima 2.5"

require_file docs/architecture/A-002-nexo-integration-boundary.md
require_text docs/architecture/A-002-nexo-integration-boundary.md "Nexo-aware, não Nexo-owned"
require_file contracts/nexo_research_integration.schema.json
pass "fronteira Nexo"

require_file .hoa/project.yaml
require_file .hoa/atmosphere-a0.yaml
require_file domain/REFLEXIVITY.md
require_text .hoa/project.yaml "runtime_dependency_on_praxis: false"
require_text .hoa/project.yaml "authorized_decision"
pass "Praxis scaffold"

printf '\nAtmos A0: PASS\n'
printf 'Evidência operacional: %s\n' "$RUN_EVIDENCE_FILE"
trap - EXIT
EOF

chmod +x scripts/verify_a0.sh

# Mantém uma cópia do próprio bootstrap no repositório quando executado de arquivo.
if [[ -f "${BASH_SOURCE[0]}" ]]; then
  source_path="$(readlink -f "${BASH_SOURCE[0]}")"
  target_path="$(readlink -m "$ROOT/scripts/bootstrap_sister_atmos_a0.sh")"
  if [[ "$source_path" != "$target_path" ]]; then
    cp "$source_path" "$target_path"
    chmod +x "$target_path"
  fi
fi

log "Verificando A0"
./scripts/verify_a0.sh

log "Estado Git"
git status --short

cat <<'EOF'

=== Bootstrap concluído ===

O A0 foi criado e verificado.
Nenhum arquivo foi adicionado ao índice e nenhum commit foi criado automaticamente.

Próxima decisão após revisar o diff:
  A0-E001 — baseline comportamental do Sister-Clima 2.5.

EOF
