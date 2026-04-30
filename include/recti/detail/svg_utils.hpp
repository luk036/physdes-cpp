// include/recti/detail/svg_utils.hpp
#pragma once

#include <recti/global_router.hpp>
#include <iosfwd>
#include <vector>

namespace recti::detail {

    /**
     * @brief SVG rendering parameters
     *
     * This structure contains all the parameters needed for generating
     * SVG visualizations of routing trees and geometric objects.
     */
    struct SvgParams {
        int width;     ///< Width of the SVG canvas
        int height;    ///< Height of the SVG canvas
        int margin;    ///< Margin around the drawing area
        double scale;  ///< Scaling factor for coordinates
        int min_x;     ///< Minimum x-coordinate in the data
        int min_y;     ///< Minimum y-coordinate in the data
    };

    // Declarations for functions moved to global_router.cpp
    template <typename IntPoint>
    SvgParams calculate_svg_params(const std::vector<RoutingNode<IntPoint>*>& nodes, int width,
                                   int height, int margin);

    template <> SvgParams calculate_svg_params<Point<int, int>>(
        const std::vector<RoutingNode<Point<int, int>>*>& nodes, int width, int height,
        int margin);

    template <> SvgParams calculate_svg_params<Point<Point<int, int>, int>>(
        const std::vector<RoutingNode<Point<Point<int, int>, int>>*>& nodes, int width, int height,
        int margin);

    /**
     * @brief Scale coordinates from data space to SVG canvas space
     * @param[in] x The x-coordinate in data space
     * @param[in] y The y-coordinate in data space
     * @param[in] params The SVG rendering parameters
     * @return A pair of scaled coordinates (x, y) in SVG canvas space
     */
    inline std::pair<double, double> scale_coords(int x, int y, const SvgParams& params) {
        double sx = params.margin + (x - params.min_x) * params.scale;
        double sy = params.margin + (y - params.min_y) * params.scale;
        return {sx, sy};
    }

    template <typename IntPoint> void draw_node(std::ostringstream& svg,
                                                 const RoutingNode<IntPoint>* node,
                                                 const SvgParams& params);

    template <> void draw_node<Point<int, int>>(std::ostringstream& svg,
                                                       const RoutingNode<Point<int, int>>* node,
                                                       const SvgParams& params);

    template <> void draw_node<Point<Point<int, int>, int>>(
        std::ostringstream& svg, const RoutingNode<Point<Point<int, int>, int>>* node,
        const SvgParams& params);

    void draw_legend(std::ostringstream& svg);

    template <typename IntPoint>
    void draw_stats(std::ostringstream& svg, const GlobalRoutingTree<IntPoint>& tree);

    template <>
    void draw_stats<Point<int, int>>(
        std::ostringstream& svg, const GlobalRoutingTree<Point<int, int>>& tree);

    template <>
    void draw_stats<Point<Point<int, int>, int>>(
        std::ostringstream& svg, const GlobalRoutingTree<Point<Point<int, int>, int>>& tree);

}  // namespace recti::detail
