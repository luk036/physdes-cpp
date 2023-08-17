#include <fmt/format.h>     // for format
#include <recti/greeter.h>  // for Recti, LanguageCode, LanguageCode::DE

#include <iosfwd>       // for string
#include <string>       // for basic_string
#include <type_traits>  // for move

using namespace recti;

Recti::Recti(std::string _name) : name(std::move(_name)) {}

auto Recti::greet(LanguageCode lang) const -> std::string {
    switch (lang) {
        default:
        case LanguageCode::EN:
            return fmt::format("Hello, {}!", name);
        case LanguageCode::DE:
            return fmt::format("Hallo {}!", name);
        case LanguageCode::ES:
            return fmt::format("¡Hola {}!", name);
        case LanguageCode::FR:
            return fmt::format("Bonjour {}!", name);
    }
}
