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
