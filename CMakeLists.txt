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

add_library(sister_atmos_core STATIC
  src/identity.cpp
)

target_include_directories(sister_atmos_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(sister_atmos_core PUBLIC cxx_std_23)
sister_atmos_apply_project_options(sister_atmos_core)

add_library(sister_atmos_precipitation_core STATIC
  src/precipitation/domain_types.cpp
  src/precipitation/native_samples.cpp
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

add_executable(atmos-a0
  src/main.cpp
)

target_link_libraries(atmos-a0 PRIVATE sister_atmos_core)
sister_atmos_apply_project_options(atmos-a0)

if(BUILD_TESTING)
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
endif()
