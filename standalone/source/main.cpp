#include <recti/version.h>

#include <cxxopts.hpp>
#include <iostream>
#include <recti/recti.hpp>
#include <string>
#include <vector>

auto main(int argc, char** argv) -> int {
    cxxopts::Options options("Recti", "Rectilinear shape demo");
    options.add_options()("h,help", "Print usage")("v,version", "Print version");

    const auto result = options.parse(argc, argv);
    if (result.count("help") > 0) {
        std::cout << options.help() << '\n';
        return 0;
    }
    if (result.count("version") > 0) {
        std::cout << "Recti, version " << RECTI_VERSION << '\n';
        return 0;
    }

    const std::vector<recti::Rectangle<int>> rectangles{
        recti::Rectangle<int>{{0, 4}, {0, 4}},
        recti::Rectangle<int>{{2, 6}, {2, 6}},
        recti::Rectangle<int>{{10, 12}, {10, 12}},
    };
    const auto overlap = recti::detect_overlap(rectangles);
    std::cout << "Recti: " << rectangles.size() << " rectangles";
    if (overlap.has_value()) {
        std::cout << ", first overlapping pair detected";
    } else {
        std::cout << ", no overlaps";
    }
    std::cout << '\n';

    return 0;
}
