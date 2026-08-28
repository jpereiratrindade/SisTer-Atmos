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
include(cmake/ProjectOptions.cmake)

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
  src/http.cpp
  src/http_server.cpp
)

target_include_directories(sister_atmos_http
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

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
