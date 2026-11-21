#include <algorithm>
#include <functional>
#include <limits>
#include <optional>
#include <recti/dllink.hpp>
#include <recti/point.hpp>
#include <recti/rdllist.hpp>
#include <recti/rpolygon_cut.hpp>
#include <span>
#include <vector>

namespace recti {

    /**
     * @brief Find the nearest edge to a concave vertex.
     *
     * Searches the polygon for the edge connected to `vcurr` (via vertical or
     * horizontal projection) that yields the minimum distance, used to decide
     * where to insert a new cut vertex during decomposition.
     *
     * @tparam T Coordinate type.
     * @param[in] lst All polygon points (including newly inserted ones).
     * @param[in] vcurr The current concave vertex to cut from.
     * @param[in] vi First node to scan (variant-dependent).
     * @param[in] pcurr The reference point to project onto (variant-dependent).
     * @return Pair containing the nearest edge node and whether the cut is vertical.
     */
    template <typename T>
    static auto _find_min_dist_point(const std::vector<Point<T>>& lst, Dllink<size_t>* vcurr,
                                     Dllink<size_t>* vi, const Point<T>& pcurr)
        -> std::pair<Dllink<size_t>*, bool> {
        auto* vstop = vcurr;

        T min_value = std::numeric_limits<T>::max();
        bool vertical = true;
        Dllink<size_t>* v_min = vcurr;

        while (vi != vstop) {
            const auto& p0 = lst[vi->prev->data];
            const auto& p1 = lst[vi->data];
            const auto& p2 = lst[vi->next->data];
            auto vec_i = p1 - pcurr;

            if ((p0.ycoord() < pcurr.ycoord() && pcurr.ycoord() <= p1.ycoord())
                || (p1.ycoord() <= pcurr.ycoord() && pcurr.ycoord() < p0.ycoord())) {
                auto dist = std::abs(vec_i.x());
                if (min_value > dist) {
                    min_value = dist;
                    v_min = vi;
                    vertical = true;
                }
            }
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

    namespace {

        template <typename T, typename Cmp>
        static auto find_concave_point_convex(const std::vector<Point<T>>& lst,
                                              Dllink<size_t>* vstart, const Cmp& cmp)
            -> Dllink<size_t>* {
            auto* vcurr = vstart;
            do {
                auto* vnext = vcurr->next;
                auto* vprev = vcurr->prev;

                const auto& prev_point = lst[vprev->data];
                const auto& curr_point = lst[vcurr->data];
                const auto& next_point = lst[vnext->data];

                auto vec1 = curr_point - prev_point;
                auto vec2 = next_point - curr_point;

                if (vec1.x() * vec2.x() < 0 || vec1.y() * vec2.y() < 0) {
                    auto area_diff = (curr_point.ycoord() - prev_point.ycoord())
                                     * (next_point.xcoord() - curr_point.xcoord());
                    if (cmp(area_diff)) {
                        return vcurr;
                    }
                }
                vcurr = vnext;
            } while (vcurr != vstart);

            return nullptr;
        }

        template <typename T> static auto find_min_dist_point_convex(
            const std::vector<Point<T>>& lst,
            Dllink<size_t>* vcurr) -> std::pair<Dllink<size_t>*, bool> {
            return _find_min_dist_point<T>(lst, vcurr, vcurr->next, lst[vcurr->data]);
        }

        template <typename T>
        static auto insert_cut_convex(Dllink<size_t>* new_node, Dllink<size_t>* vcurr,
                                      Dllink<size_t>* v_min, bool vertical,
                                      const std::vector<Point<T>>& lst) -> Point<T> {
            const auto& p_min = lst[v_min->data];
            const auto& p_curr = lst[vcurr->data];

            if (vertical) {
                new_node->next = vcurr->next;
                new_node->prev = v_min->prev;
                v_min->prev->next = new_node;
                vcurr->next->prev = new_node;
                vcurr->next = v_min;
                v_min->prev = vcurr;
                return Point<T>(p_min.xcoord(), p_curr.ycoord());
            }
            new_node->prev = vcurr->prev;
            new_node->next = v_min->next;
            v_min->next->prev = new_node;
            vcurr->prev->next = new_node;
            vcurr->prev = v_min;
            v_min->next = vcurr;
            return Point<T>(p_curr.xcoord(), p_min.ycoord());
        }

        template <typename T, typename RecurFn>
        static auto try_degenerate_two_node_split(Dllink<size_t>* vcurr, Dllink<size_t>* v_min,
                                                  bool vertical,
                                                  const std::vector<Point<T>>& lst,
                                                  const RecurFn& recur)
            -> std::optional<std::vector<std::vector<size_t>>> {
            const auto& p_min = lst[v_min->data];
            const auto& p_curr = lst[vcurr->data];

            Dllink<size_t>* first = nullptr;
            Dllink<size_t>* rest = nullptr;
            if (vertical && v_min->prev == vcurr->next && p_min.ycoord() == p_curr.ycoord()) {
                first = vcurr->next;
                rest = v_min->next;
            } else if (!vertical && v_min->next == vcurr->prev
                       && p_min.xcoord() == p_curr.xcoord()) {
                first = v_min->next;
                rest = v_min->prev;
            } else {
                return std::nullopt;
            }

            const auto& first_point = lst[first->data];
            if (first_point.xcoord() == p_min.xcoord()
                || first_point.ycoord() == p_min.ycoord()) {
                return std::nullopt;
            }

            first->next = v_min;
            first->prev = v_min;
            v_min->next = first;
            v_min->prev = first;
            if (vertical) {
                vcurr->next = rest;
                rest->prev = vcurr;
            } else {
                vcurr->prev = rest;
                rest->next = vcurr;
            }

            auto L1 = recur(first);
            auto L2 = recur(vcurr);
            L1.insert(L1.end(), L2.begin(), L2.end());
            return L1;
        }

        // ---------------------------------------------------------------------
        // Template Method: the explicit / implicit variants share one recursive
        // skeleton.  Each Policy supplies the hooks that differ: the
        // concave-vertex test, the min-distance scan, the list re-wiring, and
        // the first recursion target.
        // ---------------------------------------------------------------------

        template <typename T> struct ConvexCutPolicy {
            static constexpr bool has_triangle_base_case = true;

            template <typename Cmp>
            static auto find_concave_point(const std::vector<Point<T>>& lst, Dllink<size_t>* vstart,
                                           const Cmp& cmp) -> Dllink<size_t>* {
                auto* vcurr = vstart;
                do {
                    auto* vnext = vcurr->next;
                    auto* vprev = vcurr->prev;

                    auto p0 = lst[vprev->data];
                    auto p1 = lst[vcurr->data];
                    auto p2 = lst[vnext->data];

                    auto area_diff = (p1.ycoord() - p0.ycoord()) * (p2.xcoord() - p1.xcoord());
                    auto v1_vec = p1 - p0;
                    auto v2_vec = p2 - p1;

                    if (v1_vec.x() * v2_vec.x() < 0 || v1_vec.y() * v2_vec.y() < 0) {
                        if (cmp(area_diff)) {
                            return vcurr;
                        }
                    }
                    vcurr = vnext;
                } while (vcurr != vstart);

                return nullptr;
            }

            static auto find_min_dist_point(const std::vector<Point<T>>& lst, Dllink<size_t>* vcurr)
                -> std::pair<Dllink<size_t>*, bool> {
                return _find_min_dist_point<T>(lst, vcurr, vcurr->next, lst[vcurr->data]);
            }

            static auto insert_cut(Dllink<size_t>* new_node, Dllink<size_t>* vcurr,
                                   Dllink<size_t>* v_min, bool vertical,
                                   const std::vector<Point<T>>& lst) -> Point<T> {
                auto p_min = lst[v_min->data];
                auto p1 = lst[vcurr->data];

                if (vertical) {
                    new_node->next = vcurr->next;
                    new_node->prev = v_min->prev;
                    v_min->prev->next = new_node;
                    vcurr->next->prev = new_node;
                    vcurr->next = v_min;
                    v_min->prev = vcurr;
                    return Point<T>(p_min.xcoord(), p1.ycoord());
                }
                new_node->prev = vcurr->prev;
                new_node->next = v_min->next;
                v_min->next->prev = new_node;
                vcurr->prev->next = new_node;
                vcurr->prev = v_min;
                v_min->next = vcurr;
                return Point<T>(p1.xcoord(), p_min.ycoord());
            }

            static auto first_target(Dllink<size_t>* vcurr, Dllink<size_t>* /* v_min */)
                -> Dllink<size_t>* {
                return vcurr;
            }
        };

        template <typename T> struct ExplicitCutPolicy : ConvexCutPolicy<T> {
            static constexpr bool has_triangle_base_case = false;

            template <typename Cmp>
            static auto find_concave_point(const std::vector<Point<T>>& lst, Dllink<size_t>* vstart,
                                           const Cmp& cmp) -> Dllink<size_t>* {
                auto* vcurr = vstart;
                do {
                    auto* vnext = vcurr->next;
                    auto* vprev = vcurr->prev;

                    auto& p0 = lst[vprev->data];
                    auto& p1 = lst[vcurr->data];
                    auto& p2 = lst[vnext->data];
                    auto area_diff = (p1.ycoord() - p0.ycoord()) * (p2.xcoord() - p1.xcoord());
                    auto v1_vec = p1 - p0;
                    auto v2_vec = p2 - p1;

                    if (v1_vec.x() * v2_vec.x() < 0 || v1_vec.y() * v2_vec.y() < 0) {
                        if (cmp(area_diff)) {
                            return vcurr;
                        }
                    }
                    vcurr = vnext;
                } while (vcurr != vstart);

                return nullptr;
            }
        };

        template <typename T> struct ImplicitCutPolicy {
            static constexpr bool has_triangle_base_case = false;

            template <typename Cmp>
            static auto find_concave_point(const std::vector<Point<T>>& lst, Dllink<size_t>* vstart,
                                           const Cmp& cmp) -> Dllink<size_t>* {
                auto* vcurr = vstart;
                do {
                    auto* vnext = vcurr->next;

                    auto p1 = lst[vcurr->data];
                    auto p2 = lst[vnext->data];
                    auto area_diff = -(p2.ycoord() - p1.ycoord()) * (p2.xcoord() - p1.xcoord());

                    if (cmp(area_diff)) {
                        return vcurr;
                    }
                    vcurr = vnext;
                } while (vcurr != vstart);

                return nullptr;
            }

            static auto find_min_dist_point(const std::vector<Point<T>>& lst, Dllink<size_t>* vcurr)
                -> std::pair<Dllink<size_t>*, bool> {
                auto* vnext = vcurr->next;
                auto pcurr = Point<T>(lst[vnext->data].xcoord(), lst[vcurr->data].ycoord());
                return _find_min_dist_point<T>(lst, vcurr, vnext->next, pcurr);
            }

            static auto insert_cut(Dllink<size_t>* new_node, Dllink<size_t>* vcurr,
                                   Dllink<size_t>* v_min, bool vertical,
                                   const std::vector<Point<T>>& lst) -> Point<T> {
                const auto& p_min = lst[v_min->data];
                const auto& pc1 = lst[vcurr->data];
                const auto& pc2 = lst[vcurr->next->data];
                const auto p1 = Point<T>(pc2.xcoord(), pc1.ycoord());
                auto* vnext = vcurr->next;

                if (vertical) {
                    new_node->next = vnext;
                    new_node->prev = v_min->prev;
                    v_min->prev->next = new_node;
                    vnext->prev = new_node;
                    vcurr->next = v_min;
                    v_min->prev = vcurr;
                    return Point<T>(p_min.xcoord(), p1.ycoord());
                }
                new_node->prev = vcurr;
                new_node->next = v_min->next;
                v_min->next->prev = new_node;
                vcurr->next = new_node;
                vnext->prev = v_min;
                v_min->next = vnext;
                return Point<T>(p1.xcoord(), p_min.ycoord());
            }

            static auto first_target(Dllink<size_t>* /* vcurr */, Dllink<size_t>* v_min)
                -> Dllink<size_t>* {
                return v_min;
            }
        };

        template <typename T, typename Cmp, typename Policy>
        auto rpolygon_cut_recur_impl(Dllink<size_t>* v1, std::vector<Point<T>>& lst, const Cmp& cmp,
                                     RDllist& rdll) -> std::vector<std::vector<size_t>> {
            auto* v2 = v1->next;
            if (v2->next == v1) {
                return {{v1->data, v2->data}};
            }
            if constexpr (Policy::has_triangle_base_case) {
                if (v2->next->next == v1) {
                    return {{v1->data, v2->data, v2->next->data}};
                }
            }

            auto* vcurr = Policy::find_concave_point(lst, v1, cmp);
            if (vcurr == nullptr) {
                std::vector<size_t> indices;
                auto* current = v1;
                do {
                    indices.emplace_back(current->data);
                    current = current->next;
                } while (current != v1);
                return {std::move(indices)};
            }

            auto [v_min, vertical] = Policy::find_min_dist_point(lst, vcurr);
            auto recur = [&](Dllink<size_t>* node) {
                return rpolygon_cut_recur_impl<T, Cmp, Policy>(node, lst, cmp, rdll);
            };
            auto split = try_degenerate_two_node_split(vcurr, v_min, vertical, lst, recur);
            if (split) {
                return *split;
            }
            size_t n = lst.size();
            rdll.cycle.emplace_back(n);
            auto* new_node = &rdll.cycle[n];

            auto p_new = Policy::insert_cut(new_node, vcurr, v_min, vertical, lst);
            lst.emplace_back(p_new);

            auto L1 = rpolygon_cut_recur_impl<T, Cmp, Policy>(Policy::first_target(vcurr, v_min),
                                                              lst, cmp, rdll);
            auto L2 = rpolygon_cut_recur_impl<T, Cmp, Policy>(new_node, lst, cmp, rdll);
            L1.insert(L1.end(), L2.begin(), L2.end());
            return L1;
        }

        template <typename T, typename Policy>
        auto rpolygon_cut_impl(std::span<const Point<T>> pointset, bool is_anticlockwise)
            -> std::vector<std::vector<Point<T>>> {
            std::vector<Point<T>> lst(pointset.begin(), pointset.end());
            RDllist rdll(lst.size());

            auto cmp = is_anticlockwise ? std::function<bool(T)>([](T a) { return a > 0; })
                                        : std::function<bool(T)>([](T a) { return a < 0; });
            auto index_lists = rpolygon_cut_recur_impl<T, std::function<bool(T)>, Policy>(
                &rdll[0], lst, cmp, rdll);

            std::vector<std::vector<Point<T>>> result;
            for (const auto& indices : index_lists) {
                std::vector<Point<T>> polygon;
                for (auto index : indices) {
                    polygon.emplace_back(lst[index]);
                }
                result.emplace_back(polygon);
            }
            return result;
        }

        // A convex rectilinear piece is x-monotone, so the intersection with a
        // vertical slab between consecutive vertex x-coordinates is one rectangle.
        template <typename T>
        auto rpolygon_cut_convex_into_rectangles(std::span<const Point<T>> pointset,
                                                 bool is_anticlockwise)
            -> std::vector<std::vector<Point<T>>> {
            std::vector<std::vector<Point<T>>> result{};
            if (pointset.size() < 2) {
                return result;
            }

            std::vector<T> xcoords;
            xcoords.reserve(pointset.size());
            for (const auto& point : pointset) {
                xcoords.push_back(point.xcoord());
            }
            std::sort(xcoords.begin(), xcoords.end());
            xcoords.erase(std::unique(xcoords.begin(), xcoords.end()), xcoords.end());

            for (size_t index = 0; index + 1 < xcoords.size(); ++index) {
                const T x_left = xcoords[index];
                const T x_right = xcoords[index + 1];

                std::vector<T> ycoords;
                for (size_t i = 0; i < pointset.size(); ++i) {
                    const auto& current = pointset[i];
                    const auto& next = pointset[(i + 1) % pointset.size()];
                    const T x_min = std::min(current.xcoord(), next.xcoord());
                    const T x_max = std::max(current.xcoord(), next.xcoord());
                    if (x_min <= x_left && x_right <= x_max) {
                        ycoords.push_back(current.ycoord());
                    }
                }
                std::sort(ycoords.begin(), ycoords.end());

                for (size_t i = 0; i + 1 < ycoords.size(); i += 2) {
                    if (ycoords[i] == ycoords[i + 1]) {
                        continue;
                    }
                    std::vector<Point<T>> rectangle;
                    rectangle.reserve(2);
                    if (is_anticlockwise) {
                        rectangle.emplace_back(x_left, ycoords[i]);
                        rectangle.emplace_back(x_right, ycoords[i + 1]);
                    } else {
                        rectangle.emplace_back(x_left, ycoords[i + 1]);
                        rectangle.emplace_back(x_right, ycoords[i]);
                    }
                    result.emplace_back(std::move(rectangle));
                }
            }
            return result;
        }

    }  // namespace

    template <typename T, typename Cmp>
    auto rpolygon_cut_convex_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst, const Cmp& cmp,
                                   RDllist& rdll) -> std::vector<std::vector<size_t>> {
        auto* v2 = v1->next;
        if (v2->next == v1) {
            return {{v1->data, v2->data}};
        }
        auto* v3 = v2->next;
        if (v3->next == v1) {
            return {{v1->data, v2->data, v3->data}};
        }

        auto* vcurr = find_concave_point_convex(lst, v1, cmp);
        if (vcurr == nullptr) {
            std::vector<size_t> indices;
            auto* current = v1;
            do {
                indices.emplace_back(current->data);
                current = current->next;
            } while (current != v1);
            return {std::move(indices)};
        }

        auto [v_min, vertical] = find_min_dist_point_convex(lst, vcurr);
        auto recur = [&](Dllink<size_t>* node) {
            return rpolygon_cut_convex_recur<T>(node, lst, cmp, rdll);
        };
        auto split = try_degenerate_two_node_split(vcurr, v_min, vertical, lst, recur);
        if (split) {
            return *split;
        }
        size_t n = lst.size();
        rdll.cycle.emplace_back(n);
        auto* new_node = &rdll.cycle[n];

        auto p_new = insert_cut_convex(new_node, vcurr, v_min, vertical, lst);
        lst.emplace_back(p_new);

        auto L1 = rpolygon_cut_convex_recur<T>(vcurr, lst, cmp, rdll);
        auto L2 = rpolygon_cut_convex_recur<T>(new_node, lst, cmp, rdll);
        L1.insert(L1.end(), L2.begin(), L2.end());
        return L1;
    }

    template <typename T, typename Cmp>
    auto rpolygon_cut_explicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst, const Cmp& cmp,
                                     RDllist& rdll) -> std::vector<std::vector<size_t>> {
        return rpolygon_cut_recur_impl<T, Cmp, ExplicitCutPolicy<T>>(v1, lst, cmp, rdll);
    }

    template <typename T, typename Cmp>
    auto rpolygon_cut_implicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst, const Cmp& cmp,
                                     RDllist& rdll) -> std::vector<std::vector<size_t>> {
        return rpolygon_cut_recur_impl<T, Cmp, ImplicitCutPolicy<T>>(v1, lst, cmp, rdll);
    }

    template <typename T>
    auto rpolygon_cut_convex(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        std::vector<Point<T>> lst(pointset.begin(), pointset.end());
        RDllist rdll(lst.size());

        auto cmp = is_anticlockwise ? std::function<bool(T)>([](T a) { return a > 0; })
                                    : std::function<bool(T)>([](T a) { return a < 0; });
        auto index_lists = rpolygon_cut_convex_recur<T>(&rdll[0], lst, cmp, rdll);

        std::vector<std::vector<Point<T>>> result;
        for (const auto& indices : index_lists) {
            std::vector<Point<T>> polygon;
            polygon.reserve(indices.size());
            for (auto index : indices) {
                polygon.emplace_back(lst[index]);
            }
            result.emplace_back(std::move(polygon));
        }
        return result;
    }

    template <typename T>
    auto rpolygon_cut_explicit(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        return rpolygon_cut_impl<T, ExplicitCutPolicy<T>>(pointset, is_anticlockwise);
    }

    template <typename T>
    auto rpolygon_cut_implicit(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        return rpolygon_cut_impl<T, ImplicitCutPolicy<T>>(pointset, is_anticlockwise);
    }

    /**
     * @brief Cut a rectilinear polygon into rectangle pieces
     *
     * The polygon is first decomposed into convex pieces; each convex piece is
     * then sliced into rectangles by vertical slabs.
     */
    template <typename T>
    auto rpolygon_cut_rectangle(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        std::vector<std::vector<Point<T>>> result{};

        const auto convex_pieces = rpolygon_cut_convex<T>(pointset, is_anticlockwise);
        for (const auto& piece : convex_pieces) {
            auto rectangles = rpolygon_cut_convex_into_rectangles<T>(piece, is_anticlockwise);
            result.insert(result.end(), rectangles.begin(), rectangles.end());
        }
        return result;
    }

    template std::vector<std::vector<size_t>>
    rpolygon_cut_convex_recur<int, std::function<bool(int)>>(Dllink<size_t>*,
                                                             std::vector<Point<int>>&,
                                                             const std::function<bool(int)>&,
                                                             RDllist&);

    template std::vector<std::vector<size_t>>
    rpolygon_cut_explicit_recur<int, std::function<bool(int)>>(Dllink<size_t>*,
                                                               std::vector<Point<int>>&,
                                                               const std::function<bool(int)>&,
                                                               RDllist&);

    template std::vector<std::vector<size_t>>
    rpolygon_cut_implicit_recur<int, std::function<bool(int)>>(Dllink<size_t>*,
                                                               std::vector<Point<int>>&,
                                                               const std::function<bool(int)>&,
                                                               RDllist&);

    template auto rpolygon_cut_convex<int>(std::span<const Point<int>>,
                                           bool) -> std::vector<std::vector<Point<int>>>;

    template auto rpolygon_cut_explicit<int>(std::span<const Point<int>>,
                                             bool) -> std::vector<std::vector<Point<int>>>;

    template auto rpolygon_cut_implicit<int>(std::span<const Point<int>>,
                                             bool) -> std::vector<std::vector<Point<int>>>;

    template auto rpolygon_cut_rectangle<int>(std::span<const Point<int>>,
                                              bool) -> std::vector<std::vector<Point<int>>>;

}  // namespace recti
