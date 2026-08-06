#include <recti/version.h>

#include <iostream>

auto main() -> int {
    const auto ok = (RECTI_VERSION_MAJOR >= 1);
    std::cout << "recti installed test: version " << RECTI_VERSION << "\n";
    return ok ? 0 : 1;
}
