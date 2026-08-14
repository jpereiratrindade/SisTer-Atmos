#include "sister/atmos/precipitation/product_selection.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace p = sister::atmos::precipitation;

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
    bool ok = true;

    // GC-006 — ERA5 é selecionado diretamente para
    // precipitação consolidada quando ERA5-Land não
    // disponibiliza a variável requerida.
    {
        const p::ProductSelectionRequest request{
            .requested_product =
                p::MeteorologicalProduct::era5,
            .analysis_mode =
                p::AnalysisMode::
                    consolidated_precipitation,
            .era5_land_precipitation_available = false,
        };

        const auto result = p::select_product(request);

        ok &= expect(
            result.has_value(),
            "GC-006 aceito"
        );

        if (result) {
            ok &= expect(
                result->requested_product
                    == p::MeteorologicalProduct::era5,
                "GC-006 preserva produto solicitado"
            );

            ok &= expect(
                result->used_product
                    == p::MeteorologicalProduct::era5,
                "GC-006 utiliza ERA5"
            );

            ok &= expect(
                !result->fallback_used,
                "GC-006 não é fallback"
            );

            ok &= expect(
                result->semantics
                    == p::ProductSelectionSemantics::
                        direct_product_selection,
                "GC-006 é seleção direta"
            );

            const std::string_view reason =
                p::selection_reason_text(
                    result->reason
                );

            ok &= expect(
                reason.find(
                    "não disponibiliza precipitação"
                ) != std::string_view::npos,
                "GC-006 preserva razão científica"
            );
        }
    }

    // Não inventamos o caminho positivo de ERA5-Land:
    // ele ainda não possui Golden Case governado.
    {
        const p::ProductSelectionRequest request{
            .requested_product =
                p::MeteorologicalProduct::era5,
            .analysis_mode =
                p::AnalysisMode::
                    consolidated_precipitation,
            .era5_land_precipitation_available = true,
        };

        const auto result = p::select_product(request);

        ok &= expect(
            !result
                && result.error()
                    == p::ProductSelectionError::
                        ungoverned_era5_land_precipitation_path,
            "caminho não governado é rejeitado"
        );
    }

    // Produto solicitado diferente do caso governado não
    // recebe comportamento científico inventado.
    {
        const p::ProductSelectionRequest request{
            .requested_product =
                p::MeteorologicalProduct::
                    best_match_dynamic,
            .analysis_mode =
                p::AnalysisMode::
                    consolidated_precipitation,
            .era5_land_precipitation_available = false,
        };

        const auto result = p::select_product(request);

        ok &= expect(
            !result
                && result.error()
                    == p::ProductSelectionError::
                        ungoverned_requested_product,
            "produto solicitado não governado é rejeitado"
        );
    }

    ok &= expect(
        p::to_string(
            p::ProductSelectionSemantics::
                direct_product_selection
        ) == "direct_product_selection",
        "semântica possui identidade textual estável"
    );

    ok &= expect(
        p::to_string(
            p::ProductSelectionSemantics::fallback
        ) == "fallback",
        "fallback permanece representável"
    );

    ok &= expect(
        p::to_string(
            p::ProductSelectionError::
                ungoverned_requested_product
        ) == "ungoverned_requested_product",
        "produto não governado possui erro estável"
    );

    ok &= expect(
        p::to_string(
            p::ProductSelectionError::
                ungoverned_era5_land_precipitation_path
        ) == "ungoverned_era5_land_precipitation_path",
        "erro não governado possui identidade estável"
    );

    if (ok) {
        std::cout
            << "[PASS] seleção científica de produto\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
