#include <doctest/doctest.h>
#include <recti/greeter.h>
#include <recti/version.h>

#include <string>

TEST_CASE("PhysDesCpp") {
    using namespace physdes;

    PhysDesCpp physdes("Tests");

    CHECK(physdes.greet(LanguageCode::EN) == "Hello, Tests!");
    CHECK(physdes.greet(LanguageCode::DE) == "Hallo Tests!");
    CHECK(physdes.greet(LanguageCode::ES) == "¡Hola Tests!");
    CHECK(physdes.greet(LanguageCode::FR) == "Bonjour Tests!");
}

TEST_CASE("PhysDesCpp version") {
    static_assert(std::string_view(physdes_VERSION) == std::string_view("1.0"));
    CHECK(std::string(physdes_VERSION) == std::string("1.0"));
}
