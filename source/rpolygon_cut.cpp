#include <functional>
#include <limits>
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

        // ---------------------------------------------------------------------
        // Template Method: the three decomposition variants (convex / explicit /
        // implicit) share one recursive skeleton.  Each Policy supplies the
        // hooks that differ: the concave-vertex test, the min-distance scan, the
        // list re-wiring, and the first recursion target.
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

                    if (cmp(area_diff)) {
                        return vcurr;
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

    }  // namespace

    template <typename T, typename Cmp>
    auto rpolygon_cut_convex_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst, const Cmp& cmp,
                                   RDllist& rdll) -> std::vector<std::vector<size_t>> {
        return rpolygon_cut_recur_impl<T, Cmp, ConvexCutPolicy<T>>(v1, lst, cmp, rdll);
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
        return rpolygon_cut_impl<T, ConvexCutPolicy<T>>(pointset, is_anticlockwise);
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

    template <typename T>
    auto rpolygon_cut_rectangle(std::span<const Point<T>> pointset, bool is_anticlockwise)
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

    template auto rpolygon_cut_convex<int>(std::span<const Point<int>>, bool)
        -> std::vector<std::vector<Point<int>>>;

    template auto rpolygon_cut_explicit<int>(std::span<const Point<int>>, bool)
        -> std::vector<std::vector<Point<int>>>;

    template auto rpolygon_cut_implicit<int>(std::span<const Point<int>>, bool)
        -> std::vector<std::vector<Point<int>>>;

    template auto rpolygon_cut_rectangle<int>(std::span<const Point<int>>, bool)
        -> std::vector<std::vector<Point<int>>>;

}  // namespace recti
