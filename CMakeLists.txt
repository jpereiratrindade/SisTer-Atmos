cmake_minimum_required(VERSION 3.25)

project(
  sister_atmos
  VERSION 0.2.0
  DESCRIPTION "SisTer Atmos — inteligência climática e territorial para pesquisa"
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(cmake/ProjectOptions.cmake)
find_package(CURL REQUIRED)
include(FetchContent)
set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
set(BUILD_FILTERS OFF CACHE BOOL "" FORCE)
set(BUILD_GENERATORS OFF CACHE BOOL "" FORCE)
set(ENABLE_DOCS OFF CACHE BOOL "" FORCE)
set(ENABLE_LINTING OFF CACHE BOOL "" FORCE)
set(ENABLE_FORMAT OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  h3
  URL https://github.com/uber/h3/archive/refs/tags/v4.5.0.tar.gz
  URL_HASH SHA256=0da8a392a6ff77e76b60e6a331a49497d0935b6b7b6899da7a3e2786139b0441
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(h3)
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
  URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(nlohmann_json)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/web/index.html" ATMOS_INDEX_HTML)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/web/atmos.css" ATMOS_STYLESHEET)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/web/atmos-overrides.css" ATMOS_STYLE_OVERRIDES)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/web/atmos.js" ATMOS_JAVASCRIPT)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/data/rs_territorial_units.csv" ATMOS_RS_TERRITORIES)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/web/web_assets.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/sister/atmos/web_assets.hpp"
  @ONLY
)

option(
  SISTER_ATMOS_ENABLE_PRECIPITATION_CORE
  "Enable the A1 native precipitation scientific core"
  ON
)

if(SISTER_ATMOS_ENABLE_PRECIPITATION_CORE)
  include(cmake/CheckCxx23Capabilities.cmake)
endif()

add_library(sister_atmos_core STATIC
  src/identity.cpp
)

target_include_directories(sister_atmos_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(sister_atmos_core PUBLIC cxx_std_23)
sister_atmos_apply_project_options(sister_atmos_core)

if(SISTER_ATMOS_ENABLE_PRECIPITATION_CORE)

add_library(sister_atmos_precipitation_core STATIC
  src/precipitation/domain_types.cpp
  src/precipitation/native_samples.cpp
  src/precipitation/area_weighting.cpp
  src/precipitation/product_selection.cpp
  src/precipitation/temporal_coverage.cpp
  src/precipitation/operational_sampling.cpp
  src/precipitation/surface_eligibility.cpp
  src/precipitation/presentation_aggregation.cpp
)

target_include_directories(sister_atmos_precipitation_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
  sister_atmos_precipitation_core
  PUBLIC cxx_std_23
)

sister_atmos_apply_project_options(
  sister_atmos_precipitation_core
)

endif()

add_executable(atmos-a0
  src/main.cpp
)

target_link_libraries(atmos-a0 PRIVATE sister_atmos_core)
sister_atmos_apply_project_options(atmos-a0)

add_library(sister_atmos_http STATIC
  src/climate_service.cpp
  src/http.cpp
  src/http_server.cpp
)

target_include_directories(sister_atmos_http
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(sister_atmos_http PRIVATE CURL::libcurl h3 nlohmann_json::nlohmann_json)

target_compile_features(sister_atmos_http PUBLIC cxx_std_23)
sister_atmos_apply_project_options(sister_atmos_http)

add_executable(sister-atmos-http
  src/main_http.cpp
)

target_link_libraries(sister-atmos-http
  PRIVATE
    sister_atmos_http
    sister_atmos_core
)

if(SISTER_ATMOS_ENABLE_PRECIPITATION_CORE)
  target_link_libraries(sister-atmos-http
    PRIVATE
      sister_atmos_precipitation_core
  )
endif()

sister_atmos_apply_project_options(sister-atmos-http)

if(BUILD_TESTING)
  find_program(BASH_EXECUTABLE bash REQUIRED)
  find_program(PYTHON3_EXECUTABLE python3 REQUIRED)

  add_test(
    NAME sister_atmos_frontend_contract_test
    COMMAND "${PYTHON3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/frontend_contract_test.py"
  )

  add_test(
    NAME sister_atmos_component_runtime_contract_test
    COMMAND "${BASH_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/component_runtime_contract_test.sh"
  )

  add_test(
    NAME sister_atmos_runtime_e2e_test
    COMMAND
      "${CMAKE_COMMAND}" -E env
      "ATMOS_BINARY=$<TARGET_FILE:sister-atmos-http>"
      "${BASH_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/runtime_e2e_test.sh"
  )

  add_executable(sister_atmos_identity_tests
    tests/identity_tests.cpp
  )

  target_link_libraries(sister_atmos_identity_tests
    PRIVATE sister_atmos_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_identity_tests
  )

  add_test(
    NAME sister_atmos_identity_tests
    COMMAND sister_atmos_identity_tests
  )

  add_executable(sister_atmos_http_tests
    tests/http_tests.cpp
  )

  target_link_libraries(sister_atmos_http_tests
    PRIVATE sister_atmos_http
  )

  sister_atmos_apply_project_options(
    sister_atmos_http_tests
  )

  add_test(
    NAME sister_atmos_http_tests
    COMMAND sister_atmos_http_tests
  )

  if(SISTER_ATMOS_ENABLE_PRECIPITATION_CORE)

  add_executable(
    sister_atmos_precipitation_domain_types_tests
    tests/precipitation_domain_types_tests.cpp
  )

  target_link_libraries(
    sister_atmos_precipitation_domain_types_tests
    PRIVATE sister_atmos_precipitation_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_precipitation_domain_types_tests
  )

  add_test(
    NAME sister_atmos_precipitation_domain_types_tests
    COMMAND sister_atmos_precipitation_domain_types_tests
  )

  add_executable(
    sister_atmos_precipitation_native_samples_tests
    tests/precipitation_native_samples_tests.cpp
  )

  target_link_libraries(
    sister_atmos_precipitation_native_samples_tests
    PRIVATE sister_atmos_precipitation_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_precipitation_native_samples_tests
  )

  add_test(
    NAME sister_atmos_precipitation_native_samples_tests
    COMMAND sister_atmos_precipitation_native_samples_tests
  )

  add_executable(
    sister_atmos_precipitation_area_weighting_tests
    tests/precipitation_area_weighting_tests.cpp
  )

  target_link_libraries(
    sister_atmos_precipitation_area_weighting_tests
    PRIVATE sister_atmos_precipitation_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_precipitation_area_weighting_tests
  )

  add_test(
    NAME sister_atmos_precipitation_area_weighting_tests
    COMMAND sister_atmos_precipitation_area_weighting_tests
  )

  add_executable(
    sister_atmos_product_selection_tests
    tests/product_selection_tests.cpp
  )

  target_link_libraries(
    sister_atmos_product_selection_tests
    PRIVATE sister_atmos_precipitation_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_product_selection_tests
  )

  add_test(
    NAME sister_atmos_product_selection_tests
    COMMAND sister_atmos_product_selection_tests
  )

  add_executable(
    sister_atmos_temporal_coverage_tests
    tests/temporal_coverage_tests.cpp
  )

  target_link_libraries(
    sister_atmos_temporal_coverage_tests
    PRIVATE sister_atmos_precipitation_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_temporal_coverage_tests
  )

  add_test(
    NAME sister_atmos_temporal_coverage_tests
    COMMAND sister_atmos_temporal_coverage_tests
  )

  add_executable(
    sister_atmos_operational_sampling_tests
    tests/operational_sampling_tests.cpp
  )

  target_link_libraries(
    sister_atmos_operational_sampling_tests
    PRIVATE sister_atmos_precipitation_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_operational_sampling_tests
  )

  add_test(
    NAME sister_atmos_operational_sampling_tests
    COMMAND sister_atmos_operational_sampling_tests
  )

  add_executable(
    sister_atmos_surface_eligibility_tests
    tests/surface_eligibility_tests.cpp
  )

  target_link_libraries(
    sister_atmos_surface_eligibility_tests
    PRIVATE sister_atmos_precipitation_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_surface_eligibility_tests
  )

  add_test(
    NAME sister_atmos_surface_eligibility_tests
    COMMAND sister_atmos_surface_eligibility_tests
  )

  add_executable(
    sister_atmos_presentation_aggregation_tests
    tests/presentation_aggregation_tests.cpp
  )

  target_link_libraries(
    sister_atmos_presentation_aggregation_tests
    PRIVATE sister_atmos_precipitation_core
  )

  sister_atmos_apply_project_options(
    sister_atmos_presentation_aggregation_tests
  )

  add_test(
    NAME sister_atmos_presentation_aggregation_tests
    COMMAND sister_atmos_presentation_aggregation_tests
  )

  endif()
endif()
