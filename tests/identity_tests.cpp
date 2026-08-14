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
