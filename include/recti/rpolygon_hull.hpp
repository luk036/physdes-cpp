#pragma once

#include <fmt/core.h>

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

    // /**
    //  * @brief Check if a polygon is monotone with respect to a given direction function
    //  *
    //  * A polygon is monotone with respect to a direction if it can be divided into two chains
    //  * that are both monotone (either entirely non-decreasing or non-increasing) in that
    //  direction.
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
    inline auto rpolygon_make_xmonotone_hull(std::span<const Point<T>> pointset,
                                             bool is_anticlockwise) -> std::vector<Point<T>> {
        if (pointset.size() <= 3) {
            return std::vector<Point<T>>(pointset.begin(), pointset.end());
        }

        auto min_it
            = std::min_element(pointset.begin(), pointset.end(), [](const auto& a, const auto& b) {
                  return std::make_pair(a.xcoord(), a.ycoord())
                         < std::make_pair(b.xcoord(), b.ycoord());
              });
        auto max_it
            = std::max_element(pointset.begin(), pointset.end(), [](const auto& a, const auto& b) {
                  return std::make_pair(a.xcoord(), a.ycoord())
                         < std::make_pair(b.xcoord(), b.ycoord());
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
            process(
                &v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a >= 0; });
            process(
                &v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a >= 0; });
        } else {
            process(
                &v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a <= 0; });
            process(
                &v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a <= 0; });
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
    inline auto rpolygon_make_ymonotone_hull(std::span<const Point<T>> pointset,
                                             bool is_anticlockwise) -> std::vector<Point<T>> {
        if (pointset.size() <= 3) {
            return std::vector<Point<T>>(pointset.begin(), pointset.end());
        }

        auto min_it
            = std::min_element(pointset.begin(), pointset.end(), [](const auto& a, const auto& b) {
                  return std::make_pair(a.ycoord(), a.xcoord())
                         < std::make_pair(b.ycoord(), b.xcoord());
              });
        auto max_it
            = std::max_element(pointset.begin(), pointset.end(), [](const auto& a, const auto& b) {
                  return std::make_pair(a.ycoord(), a.xcoord())
                         < std::make_pair(b.ycoord(), b.xcoord());
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
            process(
                &v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a >= 0; });
            process(
                &v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a >= 0; });
        } else {
            process(
                &v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a <= 0; });
            process(
                &v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a <= 0; });
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
}  // namespace recti