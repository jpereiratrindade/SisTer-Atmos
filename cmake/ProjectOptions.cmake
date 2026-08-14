include_guard(GLOBAL)

option(
  SISTER_ATMOS_WARNINGS_AS_ERRORS
  "Treat SisTer Atmos warnings as errors"
  ON
)

option(
  SISTER_ATMOS_ENABLE_SANITIZERS
  "Enable AddressSanitizer and UndefinedBehaviorSanitizer"
  OFF
)

add_library(sister_atmos_project_options INTERFACE)
target_compile_features(
  sister_atmos_project_options
  INTERFACE cxx_std_23
)

if(MSVC)
  target_compile_options(
    sister_atmos_project_options
    INTERFACE
      /W4
      /permissive-
  )

  if(SISTER_ATMOS_WARNINGS_AS_ERRORS)
    target_compile_options(
      sister_atmos_project_options
      INTERFACE /WX
    )
  endif()

elseif(
  CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
  OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"
)
  target_compile_options(
    sister_atmos_project_options
    INTERFACE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wshadow
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Woverloaded-virtual
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
      -Wimplicit-fallthrough
  )

  if(SISTER_ATMOS_WARNINGS_AS_ERRORS)
    target_compile_options(
      sister_atmos_project_options
      INTERFACE -Werror
    )
  endif()
endif()

add_library(sister_atmos_sanitizers INTERFACE)

if(SISTER_ATMOS_ENABLE_SANITIZERS)
  if(
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"
  )
    include(CheckCXXSourceCompiles)

    set(
      _SISTER_ATMOS_SAVED_REQUIRED_FLAGS
      "${CMAKE_REQUIRED_FLAGS}"
    )
    set(
      _SISTER_ATMOS_SAVED_REQUIRED_LINK_OPTIONS
      "${CMAKE_REQUIRED_LINK_OPTIONS}"
    )

    set(
      CMAKE_REQUIRED_FLAGS
      "${CMAKE_REQUIRED_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all"
    )

    list(
      APPEND CMAKE_REQUIRED_LINK_OPTIONS
      -fsanitize=address,undefined
      -fno-omit-frame-pointer
      -fno-sanitize-recover=all
    )

    check_cxx_source_compiles(
      "
      int main() {
        return 0;
      }
      "
      SISTER_ATMOS_SANITIZER_RUNTIME_AVAILABLE
    )

    set(
      CMAKE_REQUIRED_FLAGS
      "${_SISTER_ATMOS_SAVED_REQUIRED_FLAGS}"
    )
    set(
      CMAKE_REQUIRED_LINK_OPTIONS
      "${_SISTER_ATMOS_SAVED_REQUIRED_LINK_OPTIONS}"
    )

    unset(_SISTER_ATMOS_SAVED_REQUIRED_FLAGS)
    unset(_SISTER_ATMOS_SAVED_REQUIRED_LINK_OPTIONS)

    if(NOT SISTER_ATMOS_SANITIZER_RUNTIME_AVAILABLE)
      message(
        FATAL_ERROR
        "AddressSanitizer/UndefinedBehaviorSanitizer runtime is unavailable. "
        "Install the compiler-matched ASan/UBSan runtime before enabling "
        "SISTER_ATMOS_ENABLE_SANITIZERS."
      )
    endif()

    target_compile_options(
      sister_atmos_sanitizers
      INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )

    target_link_options(
      sister_atmos_sanitizers
      INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )
  else()
    message(
      FATAL_ERROR
      "SISTER_ATMOS_ENABLE_SANITIZERS is unsupported by this compiler"
    )
  endif()
endif()

function(sister_atmos_apply_project_options target)
  if(NOT TARGET "${target}")
    message(
      FATAL_ERROR
      "Unknown SisTer Atmos target: ${target}"
    )
  endif()

  target_link_libraries(
    "${target}"
    PRIVATE
      sister_atmos_project_options
      sister_atmos_sanitizers
  )
endfunction()
