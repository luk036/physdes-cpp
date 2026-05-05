/**
 * @file demo_overlap.cpp
 * @brief Demo: Rectangle Overlap Detection using Line Sweep Algorithm
 */

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "recti/interval.hpp"
#include "recti/point.hpp"
#include "recti/recti.hpp"

using namespace recti;

/**
 * @brief Visualize rectangles and overlap detection as SVG
 */
std::string visualize_overlap_svg(
    const std::vector<Rectangle<int>>& rectangles,
    const std::optional<std::pair<Rectangle<int>, Rectangle<int>>>& overlapping_pair,
    int width = 800,
    int height = 600,
    int margin = 50) {

    if (rectangles.empty()) {
        return "<svg></svg>";
    }

    int min_x = rectangles[0].xcoord().lb();
    int max_x = rectangles[0].xcoord().ub();
    int min_y = rectangles[0].ycoord().lb();
    int max_y = rectangles[0].ycoord().ub();

    for (const auto& r : rectangles) {
        min_x = std::min(min_x, r.xcoord().lb());
        max_x = std::max(max_x, r.xcoord().ub());
        min_y = std::min(min_y, r.ycoord().lb());
        max_y = std::max(max_y, r.ycoord().ub());
    }

    int content_w = max_x - min_x;
    int content_h = max_y - min_y;
    if (content_w == 0) content_w = 1;
    if (content_h == 0) content_h = 1;

    double scale_x = (width - 2.0 * margin) / content_w;
    double scale_y = (height - 2.0 * margin) / content_h;
    double scale = std::min(scale_x, scale_y);

    auto to_svg_x = [margin, min_x, scale](int x) -> double {
        return margin + (x - min_x) * scale;
    };
    auto to_svg_y = [height, margin, min_y, scale](int y) -> double {
        return height - margin - (y - min_y) * scale;
    };

    std::vector<size_t> overlap_indices;
    if (overlapping_pair) {
        const auto& r1 = overlapping_pair->first;
        const auto& r2 = overlapping_pair->second;
        for (size_t i = 0; i < rectangles.size(); ++i) {
            const auto& r = rectangles[i];
            if ((r.xcoord() == r1.xcoord() && r.ycoord() == r1.ycoord()) ||
                (r.xcoord() == r2.xcoord() && r.ycoord() == r2.ycoord())) {
                overlap_indices.push_back(i);
            }
        }
    }

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\"" << height
        << "\" style=\"background-color:#f8f9fa;\">\n";

    int legend_y = 30;
    svg << "<text x=\"" << margin << "\" y=\"" << legend_y << "\" font-size=\"14\" fill=\"#333\">Rectangles</text>\n";
    legend_y += 20;
    svg << "<rect x=\"" << margin << "\" y=\"" << (legend_y - 12)
        << "\" width=\"16\" height=\"12\" fill=\"#4a90d9\" stroke=\"#333\"/>\n";
    svg << "<text x=\"" << (margin + 20) << "\" y=\"" << legend_y
        << "\" font-size=\"12\" fill=\"#333\">Non-overlapping</text>\n";
    legend_y += 20;
    svg << "<rect x=\"" << margin << "\" y=\"" << (legend_y - 12)
        << "\" width=\"16\" height=\"12\" fill=\"#e74c3c\" stroke=\"#c0392b\"/>\n";
    svg << "<text x=\"" << (margin + 20) << "\" y=\"" << legend_y
        << "\" font-size=\"12\" fill=\"#333\">Overlapping</text>\n";

    if (overlapping_pair) {
        legend_y += 20;
        svg << "<text x=\"" << (margin + 20) << "\" y=\"" << legend_y
            << "\" font-size=\"12\" fill=\"#e74c3c\">Overlap detected!</text>\n";
        legend_y += 20;
        svg << "<text x=\"" << (margin + 20) << "\" y=\"" << legend_y
            << "\" font-size=\"11\" fill=\"#666\">(" << overlapping_pair->first.xcoord() << ", "
            << overlapping_pair->first.ycoord() << ")</text>\n";
        legend_y += 15;
        svg << "<text x=\"" << (margin + 20) << "\" y=\"" << legend_y
            << "\" font-size=\"11\" fill=\"#666\">(" << overlapping_pair->second.xcoord() << ", "
            << overlapping_pair->second.ycoord() << ")</text>\n";
    }

    for (size_t idx = 0; idx < rectangles.size(); ++idx) {
        const auto& r = rectangles[idx];
        double x1 = to_svg_x(r.xcoord().lb());
        double y1 = to_svg_y(r.ycoord().ub());
        double w = (r.xcoord().ub() - r.xcoord().lb()) * scale;
        double h = (r.ycoord().ub() - r.ycoord().lb()) * scale;

        bool is_overlap = false;
        for (size_t oi : overlap_indices) {
            if (oi == idx) {
                is_overlap = true;
                break;
            }
        }

        std::string fill = is_overlap ? "#e74c3c" : "#4a90d9";
        std::string stroke = is_overlap ? "#c0392b" : "#357abd";
        std::string opacity = is_overlap ? "0.7" : "0.5";

        svg << "<rect x=\"" << x1 << "\" y=\"" << y1 << "\" width=\"" << w << "\" height=\"" << h
            << "\" fill=\"" << fill << "\" stroke=\"" << stroke << "\" stroke-width=\"2\" opacity=\""
            << opacity << "\"/>\n";

        double cx = x1 + w / 2;
        double cy = y1 + h / 2;
        svg << "<text x=\"" << cx << "\" y=\"" << (cy + 4)
            << "\" text-anchor=\"middle\" font-size=\"11\" fill=\"white\" font-weight=\"bold\">"
            << (idx + 1) << "</text>\n";
    }

    svg << "</svg>";
    return svg.str();
}

void demo() {
    std::cout << "=== Rectangle Overlap Detection Demo ===\n\n";

    std::vector<Rectangle<int>> rects = {
        Rectangle<int>{Interval{0, 4}, Interval{0, 4}},
        Rectangle<int>{Interval{2, 6}, Interval{2, 6}},
        Rectangle<int>{Interval{5, 9}, Interval{5, 9}},
        Rectangle<int>{Interval{8, 12}, Interval{8, 12}},
        Rectangle<int>{Interval{11, 15}, Interval{11, 15}},
        Rectangle<int>{Interval{3, 7}, Interval{10, 14}},
        Rectangle<int>{Interval{14, 18}, Interval{14, 18}},
        Rectangle<int>{Interval{16, 20}, Interval{4, 8}},
        Rectangle<int>{Interval{6, 10}, Interval{6, 10}},
        Rectangle<int>{Interval{9, 13}, Interval{12, 16}},
        Rectangle<int>{Interval{0, 3}, Interval{8, 11}},
    };

    std::cout << "Testing with " << rects.size() << " rectangles:\n";
    for (size_t i = 0; i < rects.size(); ++i) {
        std::cout << "  " << (i + 1) << ": (" << rects[i].xcoord() << ", " << rects[i].ycoord() << ")\n";
    }

    auto result = detect_overlap(rects);

    std::cout << "\n"
              << "==================================================\n";
    std::cout << "Overlap Detection Result\n";
    std::cout << "==================================================\n";
    if (result) {
        const auto& [r1, r2] = *result;
        std::cout << "OVERLAP DETECTED between:\n";
        std::cout << "  Rect A: (" << r1.xcoord() << ", " << r1.ycoord() << ")\n";
        std::cout << "  Rect B: (" << r2.xcoord() << ", " << r2.ycoord() << ")\n";
    } else {
        std::cout << "No overlap found\n";
    }

    std::string svg = visualize_overlap_svg(rects, result);

    std::ofstream out("demo_overlap.svg");
    out << svg;
    out.close();

    std::cout << "\nGenerated demo_overlap.svg\n";
    std::cout << "Open the SVG file in a browser to see the visualization.\n";
}

int demo_main() {
    demo();
    return 0;
}