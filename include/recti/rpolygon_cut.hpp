#pragma once

#include <functional>
#include <span>
#include <vector>

#include "dllink.hpp"
#include "point.hpp"
#include "rdllist.hpp"

namespace recti {

    /**
     * @brief Recursive function for convex decomposition
     */
    template <typename T>
    auto rpolygon_cut_convex_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
                                   const std::function<bool(T)>& cmp, RDllist& rdll)
        -> std::vector<std::vector<size_t>>;

    /**
     * @brief Recursive function for explicit decomposition
     */
    template <typename T>
    auto rpolygon_cut_explicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
                                     const std::function<bool(T)>& cmp, RDllist& rdll)
        -> std::vector<std::vector<size_t>>;

    /**
     * @brief Recursive function for implicit decomposition
     */
    template <typename T>
    auto rpolygon_cut_implicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
                                     const std::function<bool(T)>& cmp, RDllist& rdll)
        -> std::vector<std::vector<size_t>>;

    /**
     * @brief Cut a rectilinear polygon into convex pieces
     *
     * This function takes a rectilinear polygon defined by a set of points and
     * decomposes it into convex polygons using a recursive algorithm. The algorithm
     * finds concave vertices and inserts new vertices to create convex pieces.
     *
     * @tparam T The numeric type for coordinates
     * @param[in] pointset Span of points defining the polygon vertices
     * @param[in] is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return std::vector<std::vector<Point<T>>> A vector of convex polygon pieces, each defined by
     * points
     */
    template <typename T>
    auto rpolygon_cut_convex(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>>;

    /**
     * @brief Cut a polygon into explicit pieces
     * @param[in] pointset The polygon vertices
     * @param[in] is_anticlockwise Orientation flag
     * @return A vector of polygon pieces
     */
    template <typename T>
    auto rpolygon_cut_explicit(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>>;

    /**
     * @brief Cut a convex polygon into pieces according to the implicit vertices
     */
    template <typename T>
    auto rpolygon_cut_implicit(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>>;

    /**
     * @brief Cut a convex polygon into rectangle pieces
     */
    template <typename T>
    auto rpolygon_cut_rectangle(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>>;

}  // namespace recti
