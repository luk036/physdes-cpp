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
#include <fmt/core.h>

namespace recti {

    // /**
    //  * @brief Check if a polygon is monotone with respect to a given direction function
    //  *
    //  * A polygon is monotone with respect to a direction if it can be divided into two chains
    //  * that are both monotone (either entirely non-decreasing or non-increasing) in that direction.
    //  *
    //  * @tparam T The type of the coordinates
    //  * @tparam DirFunc The type of the direction function
    //  * @param pointset The polygon vertices as points
    //  * @param dir The direction function that returns a key for comparison
    //  * @return true if the polygon is monotone, false otherwise
    //  */
    // template <typename T, typename DirFunc>
    // inline auto rpolygon_is_monotone(std::span<const Point<T>> pointset, DirFunc&& dir) -> bool {
    //     if (pointset.size() <= 3) {
    //         return true;
    //     }

    //     // Find min and max indices
    //     size_t min_index = 0;
    //     size_t max_index = 0;
    //     auto min_val = dir(pointset[0]);
    //     auto max_val = dir(pointset[0]);

    //     for (size_t i = 1; i < pointset.size(); ++i) {
    //         auto current_val = dir(pointset[i]);
    //         if (current_val < min_val) {
    //             min_val = current_val;
    //             min_index = i;
    //         }
    //         if (current_val > max_val) {
    //             max_val = current_val;
    //             max_index = i;
    //         }
    //     }

    //     RDllist rdll(pointset.size());
    //     auto& v_min = rdll[min_index];
    //     auto& v_max = rdll[max_index];

    //     auto violate = [&pointset, &dir](Dllink<size_t>* vi, Dllink<size_t>* v_stop, 
    //                                     std::function<bool(T, T)> cmp) -> bool {
    //         auto current = vi;
    //         while (current != v_stop) {
    //             auto vnext = current->next;
    //             auto current_key = dir(pointset[current->data]);
    //             auto next_key = dir(pointset[vnext->data]);
    //             if (cmp(std::get<0>(current_key), std::get<0>(next_key))) {
    //                 return true;
    //             }
    //             current = vnext;
    //         }
    //         return false;
    //     };

    //     // Chain from min to max
    //     if (violate(&v_min, &v_max, [](T a, T b) { return a > b; })) {
    //         return false;
    //     }

    //     // Chain from max to min
    //     return !violate(&v_max, &v_min, [](T a, T b) { return a < b; });
    // }

    // /**
    //  * @brief Check if a polygon is x-monotone
    //  *
    //  * A polygon is x-monotone if it can be divided into two chains that are both
    //  * monotone with respect to the x-axis.
    //  *
    //  * @tparam T The type of the coordinates
    //  * @param pointset The polygon vertices as points
    //  * @return true if the polygon is x-monotone, false otherwise
    //  */
    // template <typename T>
    // inline auto rpolygon_is_xmonotone(std::span<const Point<T>> pointset) -> bool {
    //     auto x_key = [](const Point<T>& pt) -> std::pair<T, T> {
    //         return {pt.xcoord(), pt.ycoord()};
    //     };
    //     return rpolygon_is_monotone(pointset, x_key);
    // }

    // /**
    //  * @brief Check if a polygon is y-monotone
    //  *
    //  * A polygon is y-monotone if it can be divided into two chains that are both
    //  * monotone with respect to the y-axis.
    //  *
    //  * @tparam T The type of the coordinates
    //  * @param pointset The polygon vertices as points
    //  * @return true if the polygon is y-monotone, false otherwise
    //  */
    // template <typename T>
    // inline auto rpolygon_is_ymonotone(std::span<const Point<T>> pointset) -> bool {
    //     auto y_key = [](const Point<T>& pt) -> std::pair<T, T> {
    //         return {pt.ycoord(), pt.xcoord()};
    //     };
    //     return rpolygon_is_monotone(pointset, y_key);
    // }

    // /**
    //  * @brief Check if a polygon is convex
    //  *
    //  * A rectilinear polygon is convex precisely when it is both x-monotone and y-monotone.
    //  *
    //  * @tparam T The type of the coordinates
    //  * @param pointset The polygon vertices as points
    //  * @return true if the polygon is convex, false otherwise
    //  */
    // template <typename T>
    // inline auto rpolygon_is_convex(std::span<const Point<T>> pointset) -> bool {
    //     return rpolygon_is_xmonotone(pointset) && rpolygon_is_ymonotone(pointset);
    // }

    /**
     * @brief Create an x-monotone hull from a rectilinear polygon
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of points representing the x-monotone hull
     */
    template <typename T>
    inline auto rpolygon_make_xmonotone_hull(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<Point<T>> {
        if (pointset.size() <= 3) {
            return std::vector<Point<T>>(pointset.begin(), pointset.end());
        }

        auto min_it = std::min_element(pointset.begin(), pointset.end(),
            [](const auto& a, const auto& b) {
                return std::make_pair(a.xcoord(), a.ycoord()) <
                       std::make_pair(b.xcoord(), b.ycoord());
            });
        auto max_it = std::max_element(pointset.begin(), pointset.end(),
            [](const auto& a, const auto& b) {
                return std::make_pair(a.xcoord(), a.ycoord()) <
                       std::make_pair(b.xcoord(), b.ycoord());
            });
        size_t min_index = static_cast<size_t>(std::distance(pointset.begin(), min_it));
        size_t max_index = static_cast<size_t>(std::distance(pointset.begin(), max_it));
        Point<T> min_point = *min_it;

        RDllist rdll(pointset.size());
        auto& v_min = rdll[min_index];
        auto& v_max = rdll[max_index];

        auto process = [&pointset](Dllink<size_t>* vcurr, Dllink<size_t>* vstop,
                                   std::function<bool(T, T)> cmp, std::function<bool(T)> cmp2) {
            while (vcurr != vstop) {
                auto vnext = vcurr->next;
                auto vprev = vcurr->prev;
                const auto& p0 = pointset[vprev->data];
                const auto& p1 = pointset[vcurr->data];
                const auto& p2 = pointset[vnext->data];
                if (cmp(p1.xcoord(), p2.xcoord()) || cmp(p0.xcoord(), p1.xcoord())) {
                    T area_diff = (p1.ycoord() - p0.ycoord()) * (p2.xcoord() - p1.xcoord());
                    if (cmp2(area_diff)) {
                        vcurr->detach();
                        vcurr = vprev;
                    } else {
                        vcurr = vnext;
                    }
                } else {
                    vcurr = vnext;
                }
            }
        };

        if (is_anticlockwise) {
            process(&v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a >= 0; });
            process(&v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a >= 0; });
        } else {
            process(&v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a <= 0; });
            process(&v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a <= 0; });
        }

        std::vector<Point<T>> result = {min_point};
        for (const auto& v : rdll.from_node(min_index)) {
            result.push_back(pointset[v.data]);
        }
        return result;
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
    inline auto rpolygon_make_ymonotone_hull(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<Point<T>> {
        if (pointset.size() <= 3) {
            return std::vector<Point<T>>(pointset.begin(), pointset.end());
        }

        auto min_it = std::min_element(pointset.begin(), pointset.end(),
            [](const auto& a, const auto& b) {
                return std::make_pair(a.ycoord(), a.xcoord()) <
                       std::make_pair(b.ycoord(), b.xcoord());
            });
        auto max_it = std::max_element(pointset.begin(), pointset.end(),
            [](const auto& a, const auto& b) {
                return std::make_pair(a.ycoord(), a.xcoord()) <
                       std::make_pair(b.ycoord(), b.xcoord());
            });
        size_t min_index = static_cast<size_t>(std::distance(pointset.begin(), min_it));
        size_t max_index = static_cast<size_t>(std::distance(pointset.begin(), max_it));
        Point<T> min_point = *min_it;

        RDllist rdll(pointset.size());
        auto& v_min = rdll[min_index];
        auto& v_max = rdll[max_index];

        auto process = [&pointset](Dllink<size_t>* vcurr, Dllink<size_t>* vstop,
                                   std::function<bool(T, T)> cmp, std::function<bool(T)> cmp2) {
            while (vcurr != vstop) {
                auto vnext = vcurr->next;
                auto vprev = vcurr->prev;
                const auto& p0 = pointset[vprev->data];
                const auto& p1 = pointset[vcurr->data];
                const auto& p2 = pointset[vnext->data];
                if (cmp(p1.ycoord(), p2.ycoord()) || cmp(p0.ycoord(), p1.ycoord())) {
                    T area_diff = (p1.ycoord() - p0.ycoord()) * (p2.xcoord() - p1.xcoord());
                    if (cmp2(area_diff)) {
                        vcurr->detach();
                        vcurr = vprev;
                    } else {
                        vcurr = vnext;
                    }
                } else {
                    vcurr = vnext;
                }
            }
        };

        if (is_anticlockwise) {
            process(&v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a >= 0; });
            process(&v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a >= 0; });
        } else {
            process(&v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a <= 0; });
            process(&v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a <= 0; });
        }

        std::vector<Point<T>> result = {min_point};
        for (const auto& v : rdll.from_node(min_index)) {
            result.push_back(pointset[v.data]);
        }
        return result;
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

    /**
     * @brief Recursively cut a rectilinear polygon into convex polygons
     *
     * @tparam T The type of the coordinates
     * @param v1 The starting node in the doubly-linked list
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @param rdll The doubly-linked list representing the polygon
     * @return A vector of vectors of indices representing convex polygons
     */
    template <typename T>
    inline auto rpolygon_cut_convex_recur(Dllink<size_t>* v1, std::vector<Point<T>>& pointset,
                                         bool is_anticlockwise, RDllist& rdll)
        -> std::vector<std::vector<size_t>> {
        auto v2 = v1->next;
        auto v3 = v2->next;
        if (v3 == v1) {  // Rectangle
            return {{v1->data, v2->data}};
        }
        if (v3->next == v1) {  // Monotone
            return {{v1->data, v2->data, v3->data}};
        }

        auto find_concave_point = [&pointset](Dllink<size_t>* vcurr, std::function<bool(T)> cmp2)
            -> Dllink<size_t>* {
            auto vstop = vcurr;
            do {
                auto vnext = vcurr->next;
                auto vprev = vcurr->prev;
                const auto& p0 = pointset[vprev->data];
                const auto& p1 = pointset[vcurr->data];
                const auto& p2 = pointset[vnext->data];
                T area_diff = (p1.ycoord() - p0.ycoord()) * (p2.xcoord() - p1.xcoord());
                Vector2<T> v1 = p1 - p0;
                Vector2<T> v2 = p2 - p1;
                if (v1.x() * v2.x() < 0 || v1.y() * v2.y() < 0) {
                    if (cmp2(area_diff)) {
                        return vcurr;
                    }
                }
                vcurr = vnext;
            } while (vcurr != vstop);
            return nullptr;  // Convex
        };

        auto vcurr = is_anticlockwise ? find_concave_point(v1, [](T a) { return a > 0; })
                                      : find_concave_point(v1, [](T a) { return a < 0; });

        if (vcurr == nullptr) {  // Convex
            std::vector<size_t> L = {v1->data};
            for (const auto& vi : rdll.from_node(v1->data)) {
                L.push_back(vi.data);
            }
            return {L};
        }

        auto find_min_dist_point = [&pointset](Dllink<size_t>* vcurr)
            -> std::pair<Dllink<size_t>*, bool> {
            auto vnext = vcurr->next;
            auto vprev = vcurr->prev;
            auto vi = vnext->next;
            T min_value = std::numeric_limits<T>::max();
            bool vertical = true;
            Dllink<size_t>* v_min = vcurr;
            const auto& pcurr = pointset[vcurr->data];
            while (vi != vprev) {
                const auto& p0 = pointset[vi->prev->data];
                const auto& p1 = pointset[vi->data];
                const auto& p2 = pointset[vi->next->data];
                Vector2<T> vec_i = p1 - pcurr;
                if ((p0.ycoord() <= pcurr.ycoord() && pcurr.ycoord() <= p1.ycoord()) ||
                    (p1.ycoord() <= pcurr.ycoord() && pcurr.ycoord() <= p0.ycoord())) {
                    if (std::abs(vec_i.x()) < min_value) {
                        min_value = std::abs(vec_i.x());
                        v_min = vi;
                        vertical = true;
                    }
                }
                if ((p2.xcoord() <= pcurr.xcoord() && pcurr.xcoord() <= p1.xcoord()) ||
                    (p1.xcoord() <= pcurr.xcoord() && pcurr.xcoord() <= p2.xcoord())) {
                    if (std::abs(vec_i.y()) < min_value) {
                        min_value = std::abs(vec_i.y());
                        v_min = vi;
                        vertical = false;
                    }
                }
                vi = vi->next;
            }
            return {v_min, vertical};
        };

        auto [v_min, vertical] = find_min_dist_point(vcurr);
        size_t n = pointset.size();

        pointset.emplace_back(Point<T, T>{});
        rdll.cycle.emplace_back(Dllink<size_t>(n));
        auto& new_node = rdll[n];
        const auto& p_min = pointset[v_min->data];
        const auto& p1 = pointset[vcurr->data];

        if (vertical) {
            new_node.next = vcurr->next;
            new_node.prev = v_min->prev;
            v_min->prev->next = &new_node;
            vcurr->next->prev = &new_node;
            vcurr->next = v_min;
            v_min->prev = vcurr;
            pointset[n] = Point<T>(p_min.xcoord(), p1.ycoord());
        } else {
            new_node.prev = vcurr->prev;
            new_node.next = v_min->next;
            v_min->next->prev = &new_node;
            vcurr->prev->next = &new_node;
            vcurr->prev = v_min;
            v_min->next = vcurr;
            pointset[n] = Point<T>(p1.xcoord(), p_min.ycoord());
        }

        auto L1 = rpolygon_cut_convex_recur(vcurr, pointset, is_anticlockwise, rdll);
        auto L2 = rpolygon_cut_convex_recur(&new_node, pointset, is_anticlockwise, rdll);
        L1.insert(L1.end(), L2.begin(), L2.end());
        return L1;
    }

    /**
     * @brief Cut a rectilinear polygon into convex polygons
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of vectors of points representing convex polygons
     */
    template <typename T>
    inline auto rpolygon_cut_convex(std::vector<std::vector<Point<T>>>& result, std::span<const Point<T>> pointset, bool is_anticlockwise) {
        std::vector<Point<T>> mutable_pointset(pointset.begin(), pointset.end());
        RDllist rdll(pointset.size());
        auto L = rpolygon_cut_convex_recur(&rdll[0], mutable_pointset, is_anticlockwise, rdll);
        fmt::print("after recursion\n");

        // std::vector<std::vector<Point<T>>> result;
        for (const auto& item : L) {
            fmt::print("    item begin\n");
            std::vector<Point<T>> P{};
            for (const auto& i : item) {
                Point<T> pi = mutable_pointset[i];
                fmt::print("    ({}, {}) ", pi.xcoord(), pi.ycoord());
                P.push_back(pi);
            }
            fmt::print("\n");
            for (const auto& pi : P) {
                fmt::print("    ({}, {}) ", pi.xcoord(), pi.ycoord());
            }
            fmt::print("    item end\n");
            result.push_back(P);
            fmt::print("    after push_back\n");
        }

        for (const auto& C: result) {
            fmt::print("check begin\n");
            for (const auto& pi : C) {
                fmt::print("    ({}, {}) ", pi.xcoord(), pi.ycoord());
            }
            fmt::print("check end\n");
        }
        fmt::print("after loop\n");

        // return result;
    }

}  // namespace recti