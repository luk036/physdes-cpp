/** @file rpolygon_cut.hpp
 *  @brief Functions for decomposing rectilinear polygons into convex/rectangular pieces.
 */

#pragma once

#include <span>
#include <vector>

#include "point.hpp"

// Forward declaration for recursive functions
class RDllist;
template <typename T> class Dllink;

namespace recti {

    /**
     * @brief Recursive function for convex decomposition
     */
    template <typename T, typename Cmp>
    auto rpolygon_cut_convex_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst, const Cmp& cmp,
                                   RDllist& rdll) -> std::vector<std::vector<size_t>>;

    /**
     * @brief Recursive function for explicit decomposition
     */
    template <typename T, typename Cmp>
    auto rpolygon_cut_explicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst, const Cmp& cmp,
                                     RDllist& rdll) -> std::vector<std::vector<size_t>>;

    /**
     * @brief Recursive function for implicit decomposition
     */
    template <typename T, typename Cmp>
    auto rpolygon_cut_implicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst, const Cmp& cmp,
                                     RDllist& rdll) -> std::vector<std::vector<size_t>>;

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
     * @brief Cut a rectilinear polygon into convex pieces using the full vertex list
     *
     * This is the full-vertex baseline of `rpolygon_cut_convex`.  It stores and
     * traverses every boundary vertex, tests orthogonal convexity on the full
     * cycle, and cuts at the first reflex vertex when the cycle is not convex.
     * The output pieces are orthogonally convex and use full vertex lists.
     *
     * @tparam T The numeric type for coordinates
     * @param[in] pointset The full vertex list of the polygon
     * @param[in] is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of orthogonally convex pieces, each as a full vertex list
     */
    template <typename T>
    auto rpolygon_cut_convex_full(std::span<const Point<T>> pointset, bool is_anticlockwise)
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
     * @brief Cut a rectilinear polygon into rectangle pieces
     *
     * The polygon is first decomposed into convex pieces; each convex piece is
     * then sliced into rectangles by vertical slabs.
     */
    template <typename T>
    auto rpolygon_cut_rectangle(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>>;

}  // namespace recti
