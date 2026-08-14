# Golden Cases — A0-E001

Os casos abaixo foram derivados de comportamento executável e testes
existentes no Sister-Clima v2.5.

| ID | Caso | Origem no legado |
|---|---|---|
| GC-001 | deduplicação de célula nativa | test_repeated_native_cell_is_deduplicated |
| GC-002 | rejeição de valores divergentes | test_divergent_values_for_same_native_cell_are_rejected |
| GC-003 | média ponderada por área | test_precipitation_uses_intersection_weighted_mean |
| GC-004 | média de amostras operacionais únicas | test_operational_sampling_uses_mean_and_declares_no_native_surface |
| GC-005 | produto pontual não gera superfície | test_point_only_product_cannot_generate_h3_surface |
| GC-006 | seleção direta de ERA5 | test_era5_is_selected_directly_for_precipitation |
| GC-007 | cobertura temporal efetiva comum | test_temporal_accumulation_uses_only_dates_common_to_all_cells |
| GC-008 | média espacial H3, nunca soma espacial | test_h3_uses_mean_of_point_accumulations_instead_of_spatial_sum |

Esses casos constituem o primeiro baseline de equivalência
Sister-Clima → SisTer Atmos.

Nenhum caso autoriza copiar a implementação Python. O contrato é
comportamental e científico.
