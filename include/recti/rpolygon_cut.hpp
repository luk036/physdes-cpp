#pragma once

#include <algorithm>
#include <functional>
#include <span>
#include <vector>

#include "point.hpp"
#include "polygon.hpp"
#include "rdllist.hpp"
#include "rpolygon.hpp"
#include "vector2.hpp"

namespace recti {

    // Find minimum distance point for cutting
    template <typename T>
    inline auto _find_min_dist_point(const std::vector<Point<T>>& lst, Dllink<size_t>* vcurr)
        -> std::pair<Dllink<size_t>*, bool> {
        auto vnext = vcurr->next;
        auto vstop = vcurr;
        auto vi = vnext;

        T min_value = std::numeric_limits<T>::max();
        bool vertical = true;
        Dllink<size_t>* v_min = vcurr;
        const auto& pcurr = lst[vcurr->data];

        while (vi != vstop) {
            const auto& p0 = lst[vi->prev->data];
            const auto& p1 = lst[vi->data];
            const auto& p2 = lst[vi->next->data];
            auto vec_i = p1 - pcurr;

            // Check vertical alignment
            if ((p0.ycoord() < pcurr.ycoord() && pcurr.ycoord() <= p1.ycoord())
                || (p1.ycoord() <= pcurr.ycoord() && pcurr.ycoord() < p0.ycoord())) {
                auto dist = std::abs(vec_i.x());
                if (min_value > dist) {
                    min_value = dist;
                    v_min = vi;
                    vertical = true;
                }
            }
            // Check horizontal alignment
            if ((p2.xcoord() < pcurr.xcoord() && pcurr.xcoord() <= p1.xcoord())
                || (p1.xcoord() <= pcurr.xcoord() && pcurr.xcoord() < p2.xcoord())) {
                auto dist = std::abs(vec_i.y());
                if (min_value > dist) {
                    min_value = dist;
                    v_min = vi;
                    vertical = false;
                }
            }
            vi = vi->next;
        }
        return {v_min, vertical};
    }

    /**
     * @brief Recursive function for convex decomposition
     */
    template <typename T>
    auto rpolygon_cut_convex_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
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
    inline auto rpolygon_cut_convex(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        std::vector<Point<T>> lst(pointset.begin(), pointset.end());
        RDllist rdll(lst.size());

        auto cmp = is_anticlockwise ? std::function<bool(T)>([](T a) { return a > 0; })
                                    : std::function<bool(T)>([](T a) { return a < 0; });
        auto index_lists = rpolygon_cut_convex_recur(&rdll[0], lst, cmp, rdll);

        std::vector<std::vector<Point<T>>> result;
        for (const auto& indices : index_lists) {
            std::vector<Point<T>> polygon;
            for (auto index : indices) {
                polygon.push_back(lst[index]);
            }
            result.push_back(polygon);
        }

        return result;
    }

    /**
     * @brief Recursive function for explicit decomposition
     */
    template <typename T>
    auto rpolygon_cut_explicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
                                    const std::function<bool(T)>& cmp, RDllist& rdll)
        -> std::vector<std::vector<size_t>>;

    /**
     * @brief Cut a polygon into explicit pieces
     * @param[in] pointset The polygon vertices
     * @param[in] is_anticlockwise Orientation flag
     * @return A vector of polygon pieces
     */
    template <typename T>
    inline auto rpolygon_cut_explicit(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        std::vector<Point<T>> lst(pointset.begin(), pointset.end());
        RDllist rdll(lst.size());

        auto cmp = is_anticlockwise ? std::function<bool(T)>([](T a) { return a > 0; })
                                    : std::function<bool(T)>([](T a) { return a < 0; });
        auto index_lists = rpolygon_cut_explicit_recur(&rdll[0], lst, cmp, rdll);

        std::vector<std::vector<Point<T>>> result;
        for (const auto& indices : index_lists) {
            std::vector<Point<T>> polygon;
            for (auto index : indices) {
                polygon.push_back(lst[index]);
            }
            result.push_back(polygon);
        }

        return result;
    }

    /**
     * @brief Recursive function for implicit decomposition
     */
    template <typename T>
    auto rpolygon_cut_implicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
                                    const std::function<bool(T)>& cmp, RDllist& rdll)
        -> std::vector<std::vector<size_t>>;

    /**
     * @brief Cut a convex polygon into pieces according to the implicit vertices
     */
    template <typename T>
    inline auto rpolygon_cut_implicit(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        std::vector<Point<T>> lst(pointset.begin(), pointset.end());
        RDllist rdll(lst.size());

        auto cmp = is_anticlockwise ? std::function<bool(T)>([](T a) { return a > 0; })
                                    : std::function<bool(T)>([](T a) { return a < 0; });
        auto index_lists = rpolygon_cut_implicit_recur(&rdll[0], lst, cmp, rdll);

        std::vector<std::vector<Point<T>>> result;
        for (const auto& indices : index_lists) {
            std::vector<Point<T>> polygon;
            for (auto index : indices) {
                polygon.push_back(lst[index]);
            }
            result.push_back(polygon);
        }

        return result;
    }

    /**
     * @brief Cut a convex polygon into rectangle pieces
     */
    template <typename T>
    inline auto rpolygon_cut_rectangle(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        std::vector<std::vector<Point<T>>> res{};

        std::vector<Point<T>> lst(pointset.begin(), pointset.end());
        auto L1 = rpolygon_cut_implicit<T>(lst, is_anticlockwise);
        for (auto& lst1 : L1) {
            auto L2 = rpolygon_cut_explicit<T>(lst1, is_anticlockwise);
            res.insert(res.end(), L2.begin(), L2.end());
        }
        return res;
    }

}  // namespace recti