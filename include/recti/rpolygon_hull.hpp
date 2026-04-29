#pragma once

#include <functional>
#include <span>
#include <vector>

#include "point.hpp"

namespace recti {
    /**
     * @brief Create a monotone hull from a rectilinear polygon with custom direction
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @param dir Function to extract comparison coordinates
     * @return A vector of points representing the monotone hull
     */
    template <typename T>
    auto rpolygon_make_monotone_hull(std::span<const Point<T>> pointset, bool is_anticlockwise,
        const std::function<std::pair<T, T>(const Point<T>&)>& dir) -> std::vector<Point<T>>;

    /**
     * @brief Create an x-monotone hull from a rectilinear polygon
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of points representing the x-monotone hull
     */
    template <typename T>
    inline auto rpolygon_make_xmonotone_hull(std::span<const Point<T>> pointset,
                                             bool is_anticlockwise) -> std::vector<Point<T>> {
        return rpolygon_make_monotone_hull<T>(pointset, is_anticlockwise, [](const Point<T>& p) {
            return std::make_pair(p.xcoord(), p.ycoord());
        });
    }

    /**
     * @brief Create a y-monotone hull from a rectilinear polygon
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of points representing the y-monotone hull
     */
    template <typename T>
    inline auto rpolygon_make_ymonotone_hull(std::span<const Point<T>> pointset,
                                             bool is_anticlockwise) -> std::vector<Point<T>> {
        return rpolygon_make_monotone_hull<T>(pointset, is_anticlockwise, [](const Point<T>& p) {
            return std::make_pair(p.ycoord(), p.xcoord());
        });
    }

    /**
     * @brief Create a convex hull from a rectilinear polygon
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of points representing the convex hull
     */
    template <typename T>
    inline auto rpolygon_make_convex_hull(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<Point<T>> {
        auto xmono_hull = rpolygon_make_xmonotone_hull<T>(pointset, is_anticlockwise);
        return rpolygon_make_ymonotone_hull<T>(xmono_hull, is_anticlockwise);
    }
}  // namespace recti