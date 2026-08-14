include_guard(GLOBAL)

include(CheckCXXSourceCompiles)

function(
  sister_atmos_require_cxx_capability
  variable
  description
  source
)
  check_cxx_source_compiles(
    "${source}"
    "${variable}"
  )

  if(NOT "${${variable}}")
    message(
      FATAL_ERROR
      "Required C++ capability unavailable: ${description}"
    )
  endif()

  message(
    STATUS
    "SisTer Atmos REQUIRED capability: ${description} - available"
  )
endfunction()

function(
  sister_atmos_probe_cxx_capability
  variable
  description
  source
)
  check_cxx_source_compiles(
    "${source}"
    "${variable}"
  )

  if("${${variable}}")
    message(
      STATUS
      "SisTer Atmos TRIAL capability: ${description} - available"
    )
  else()
    message(
      STATUS
      "SisTer Atmos TRIAL capability: ${description} - unavailable"
    )
  endif()
endfunction()

# ------------------------------------------------------------
# Base C++23
# ------------------------------------------------------------

sister_atmos_require_cxx_capability(
  SISTER_ATMOS_CXX23_LANGUAGE
  "C++23 language mode (__cplusplus >= 202302L)"
  "
  #if __cplusplus < 202302L
  #error C++23 language mode unavailable
  #endif

  int main() {
    return 0;
  }
  "
)

# ------------------------------------------------------------
# REQUIRED — std::expected
# ------------------------------------------------------------

sister_atmos_require_cxx_capability(
  SISTER_ATMOS_HAS_EXPECTED
  "std::expected"
  "
  #include <expected>
  #include <version>

  #if !defined(__cpp_lib_expected)
  #error __cpp_lib_expected unavailable
  #endif

  #if __cpp_lib_expected < 202202L
  #error std::expected revision too old
  #endif

  int main() {
    std::expected<int, int> value{42};
    return value.value() == 42 ? 0 : 1;
  }
  "
)

# ------------------------------------------------------------
# REQUIRED — std::span
# ------------------------------------------------------------

sister_atmos_require_cxx_capability(
  SISTER_ATMOS_HAS_SPAN
  "std::span"
  "
  #include <span>
  #include <version>

  #if !defined(__cpp_lib_span)
  #error __cpp_lib_span unavailable
  #endif

  #if __cpp_lib_span < 202002L
  #error std::span revision too old
  #endif

  int main() {
    const int values[]{1, 2, 3};
    std::span<const int> view{values};

    return view.size() == 3 ? 0 : 1;
  }
  "
)

# ------------------------------------------------------------
# REQUIRED — std::to_underlying
# ------------------------------------------------------------

sister_atmos_require_cxx_capability(
  SISTER_ATMOS_HAS_TO_UNDERLYING
  "std::to_underlying"
  "
  #include <utility>
  #include <version>

  #if !defined(__cpp_lib_to_underlying)
  #error __cpp_lib_to_underlying unavailable
  #endif

  #if __cpp_lib_to_underlying < 202102L
  #error std::to_underlying revision too old
  #endif

  enum class Value {
    one = 1
  };

  int main() {
    return std::to_underlying(Value::one) == 1 ? 0 : 1;
  }
  "
)

# ------------------------------------------------------------
# TRIAL — std::mdspan
# ------------------------------------------------------------

sister_atmos_probe_cxx_capability(
  SISTER_ATMOS_HAS_MDSPAN
  "std::mdspan"
  "
  #include <cstddef>
  #include <mdspan>
  #include <version>

  #if !defined(__cpp_lib_mdspan)
  #error __cpp_lib_mdspan unavailable
  #endif

  #if __cpp_lib_mdspan < 202207L
  #error std::mdspan revision too old
  #endif

  int main() {
    int values[6]{};

    std::mdspan<
      int,
      std::extents<std::size_t, 2, 3>
    > view{values};

    return (
      view.extent(0) == 2
      && view.extent(1) == 3
    ) ? 0 : 1;
  }
  "
)

# ------------------------------------------------------------
# Relatório operacional da toolchain
# ------------------------------------------------------------

if(SISTER_ATMOS_HAS_MDSPAN)
  set(
    SISTER_ATMOS_MDSPAN_STATUS
    "AVAILABLE"
  )
else()
  set(
    SISTER_ATMOS_MDSPAN_STATUS
    "UNAVAILABLE"
  )
endif()

file(
  WRITE
  "${CMAKE_BINARY_DIR}/sister-atmos-cxx23-capabilities.txt"
  "compiler.id=${CMAKE_CXX_COMPILER_ID}\n"
  "compiler.version=${CMAKE_CXX_COMPILER_VERSION}\n"
  "cxx.standard=${CMAKE_CXX_STANDARD}\n"
  "required.language_cxx23=PASS\n"
  "required.expected=PASS\n"
  "required.span=PASS\n"
  "required.to_underlying=PASS\n"
  "trial.mdspan=${SISTER_ATMOS_MDSPAN_STATUS}\n"
)

unset(SISTER_ATMOS_MDSPAN_STATUS)
