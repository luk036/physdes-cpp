#include <recti/greeter.h>
#include <recti/version.h>

#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

auto main(int argc, char **argv) -> int {
    const std::unordered_map<std::string, recti::LanguageCode> languages{
        {"en", recti::LanguageCode::EN},
        {"de", recti::LanguageCode::DE},
        {"es", recti::LanguageCode::ES},
        {"fr", recti::LanguageCode::FR},
    };

    const cxxopts::Options options(*argv, "A program to welcome the world!");

    std::string language;
    std::string name;

    // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("v,version", "Print the current version number")
    ("n,name", "Name to greet", cxxopts::value(name)->default_value("World"))
    ("l,lang", "Language code to use", cxxopts::value(language)->default_value("en"))
  ;
    // clang-format on

    const auto result = options.parse(argc, argv);

    if (result["help"].as<bool>()) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result["version"].as<bool>()) {
        std::cout << "Recti, version " << RECTI_VERSION << std::endl;
        return 0;
    }

    const auto langIt = languages.find(language);
    if (langIt == languages.end()) {
        std::cerr << "unknown language code: " << language << std::endl;
        return 1;
    }

    // recti::Recti recti(name);
    // std::cout << recti.greet(langIt->second) << std::endl;

    return 0;
}
