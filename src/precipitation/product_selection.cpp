#include "sister/atmos/precipitation/product_selection.hpp"

#include <expected>

namespace sister::atmos::precipitation {

std::expected<
    ProductSelection,
    ProductSelectionError
>
select_product(
    ProductSelectionRequest request
) noexcept {
    if (
        request.analysis_mode
        != AnalysisMode::consolidated_precipitation
    ) {
        return std::unexpected(
            ProductSelectionError::unsupported_analysis_mode
        );
    }

    /*
     * O produto solicitado pertence à solicitação/proveniência.
     * O contrato científico vigente governa, neste momento, somente ERA5
     * como produto solicitado para este modo analítico.
     *
     * Outras combinações não recebem semântica inventada.
     */
    if (
        request.requested_product
        != MeteorologicalProduct::era5
    ) {
        return std::unexpected(
            ProductSelectionError::
                ungoverned_requested_product
        );
    }

    /*
     * O contrato científico vigente governa somente o caso em que
     * ERA5-Land não disponibiliza a precipitação requerida.
     *
     * O caminho em que essa disponibilidade é verdadeira
     * não possui Golden Case autorizado neste marco e,
     * portanto, não recebe comportamento inventado.
     */
    if (request.era5_land_precipitation_available) {
        return std::unexpected(
            ProductSelectionError::
                ungoverned_era5_land_precipitation_path
        );
    }

    return ProductSelection{
        /*
         * O contrato caracteriza a escolha como seleção direta,
         * não como fallback. Requested e used permanecem
         * conceitos distintos; requested é preservado
         * exatamente da solicitação.
         */
        .requested_product =
            request.requested_product,
        .used_product =
            MeteorologicalProduct::era5,
        .semantics =
            ProductSelectionSemantics::
                direct_product_selection,
        .fallback_used = false,
        .reason =
            ProductSelectionReason::
                era5_land_precipitation_unavailable,
    };
}

}  // namespace sister::atmos::precipitation
