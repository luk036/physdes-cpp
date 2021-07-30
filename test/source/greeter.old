#include <doctest/doctest.h>
#include <recti/greeter.h>
#include <recti/version.h>

#include <string>

TEST_CASE("Recti") {
    using namespace recti;

    Recti recti("Tests");

    CHECK(recti.greet(LanguageCode::EN) == "Hello, Tests!");
    CHECK(recti.greet(LanguageCode::DE) == "Hallo Tests!");
    CHECK(recti.greet(LanguageCode::ES) == "¡Hola Tests!");
    CHECK(recti.greet(LanguageCode::FR) == "Bonjour Tests!");
}

TEST_CASE("Recti version") {
    static_assert(std::string_view(RECTI_VERSION) == std::string_view("1.0"));
    CHECK(std::string(RECTI_VERSION) == std::string("1.0"));
}
