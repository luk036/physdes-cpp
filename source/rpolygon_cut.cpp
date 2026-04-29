#include <recti/rpolygon_cut.hpp>

namespace recti {
    template <typename T>
    auto rpolygon_cut_convex_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
                                  const std::function<bool(T)>& cmp, RDllist& rdll)
        -> std::vector<std::vector<size_t>> {
        auto v2 = v1->next;
        auto v3 = v2->next;

        if (v3 == v1) {
            return {{v1->data, v2->data}};
        }
        if (v3->next == v1) {
            return {{v1->data, v2->data, v3->data}};
        }

        auto find_concave_point = [&lst](Dllink<size_t>* vstart,
                                         const std::function<bool(T)>& cmp2) -> Dllink<size_t>* {
            auto vcurr = vstart;
            do {
                auto vnext = vcurr->next;
                auto vprev = vcurr->prev;

                auto p0 = lst[vprev->data];
                auto p1 = lst[vcurr->data];
                auto p2 = lst[vnext->data];

                auto area_diff = (p1.ycoord() - p0.ycoord()) * (p2.xcoord() - p1.xcoord());
                auto v1_vec = p1 - p0;
                auto v2_vec = p2 - p1;

                if (v1_vec.x() * v2_vec.x() < 0 || v1_vec.y() * v2_vec.y() < 0) {
                    if (cmp2(area_diff)) {
                        return vcurr;
                    }
                }
                vcurr = vnext;
            } while (vcurr != vstart);

            return nullptr;
        };

        auto vcurr = find_concave_point(v1, cmp);

        if (vcurr == nullptr) {
            std::vector<size_t> indices;
            auto current = v1;
            do {
                indices.push_back(current->data);
                current = current->next;
            } while (current != v1);
            return {std::move(indices)};
        }

        auto [v_min, vertical] = _find_min_dist_point<T>(lst, vcurr);
        size_t n = lst.size();
        rdll.cycle.emplace_back(Dllink<size_t>(n));
        auto new_node = &rdll.cycle[n];
        auto p_min = lst[v_min->data];
        auto p1 = lst[vcurr->data];

        Point<T> p_new;
        if (vertical) {
            new_node->next = vcurr->next;
            new_node->prev = v_min->prev;
            v_min->prev->next = new_node;
            vcurr->next->prev = new_node;
            vcurr->next = v_min;
            v_min->prev = vcurr;
            p_new = Point<T>(p_min.xcoord(), p1.ycoord());
        } else {
            new_node->prev = vcurr->prev;
            new_node->next = v_min->next;
            v_min->next->prev = new_node;
            vcurr->prev->next = new_node;
            vcurr->prev = v_min;
            v_min->next = vcurr;
            p_new = Point<T>(p1.xcoord(), p_min.ycoord());
        }
        lst.push_back(p_new);

        auto L1 = rpolygon_cut_convex_recur(vcurr, lst, cmp, rdll);
        auto L2 = rpolygon_cut_convex_recur(new_node, lst, cmp, rdll);
        L1.insert(L1.end(), L2.begin(), L2.end());
        return L1;
    }

    template <typename T>
    auto rpolygon_cut_explicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
                                    const std::function<bool(T)>& cmp, RDllist& rdll)
        -> std::vector<std::vector<size_t>> {
        const auto v2 = v1->next;

        if (v2->next == v1) {
            return {{v1->data, v2->data}};
        }

        auto find_explicit_concave_point =
            [&lst](Dllink<size_t>* vstart, const std::function<bool(T)>& cmp2) -> Dllink<size_t>* {
            auto vcurr = vstart;
            do {
                auto vnext = vcurr->next;
                auto vprev = vcurr->prev;

                auto& p0 = lst[vprev->data];
                auto& p1 = lst[vcurr->data];
                auto& p2 = lst[vnext->data];
                auto area_diff = (p1.ycoord() - p0.ycoord()) * (p2.xcoord() - p1.xcoord());

                if (cmp2(area_diff)) {
                    return vcurr;
                }
                vcurr = vnext;
            } while (vcurr != vstart);

            return nullptr;
        };

        auto vcurr = find_explicit_concave_point(v1, cmp);

        if (vcurr == nullptr) {
            std::vector<size_t> indices;
            auto current = v1;
            do {
                indices.push_back(current->data);
                current = current->next;
            } while (current != v1);
            return {std::move(indices)};
        }

        auto [v_min, vertical] = _find_min_dist_point<T>(lst, vcurr);
        size_t n = lst.size();
        rdll.cycle.emplace_back(Dllink<size_t>(n));
        auto new_node = &rdll.cycle[n];
        auto p_min = lst[v_min->data];
        auto p1 = lst[vcurr->data];

        Point<T> p_new;
        if (vertical) {
            new_node->next = vcurr->next;
            new_node->prev = v_min->prev;
            v_min->prev->next = new_node;
            vcurr->next->prev = new_node;
            vcurr->next = v_min;
            v_min->prev = vcurr;
            p_new = Point<T>(p_min.xcoord(), p1.ycoord());
        } else {
            new_node->prev = vcurr->prev;
            new_node->next = v_min->next;
            v_min->next->prev = new_node;
            vcurr->prev->next = new_node;
            vcurr->prev = v_min;
            v_min->next = vcurr;
            p_new = Point<T>(p1.xcoord(), p_min.ycoord());
        }

        lst.push_back(p_new);

        auto L1 = rpolygon_cut_explicit_recur(vcurr, lst, cmp, rdll);
        auto L2 = rpolygon_cut_explicit_recur(new_node, lst, cmp, rdll);
        L1.insert(L1.end(), L2.begin(), L2.end());
        return L1;
    }

    template <typename T>
    auto rpolygon_cut_implicit_recur(Dllink<size_t>* v1, std::vector<Point<T>>& lst,
                                    const std::function<bool(T)>& cmp, RDllist& rdll)
        -> std::vector<std::vector<size_t>> {
        auto v2 = v1->next;

        if (v2->next == v1) {
            return {{v1->data, v2->data}};
        }

        auto find_implicit_concave_point =
            [&lst](Dllink<size_t>* vstart, const std::function<bool(T)>& cmp2) -> Dllink<size_t>* {
            auto vcurr = vstart;
            do {
                auto vnext = vcurr->next;

                auto p1 = lst[vcurr->data];
                auto p2 = lst[vnext->data];
                auto area_diff = -(p2.ycoord() - p1.ycoord()) * (p2.xcoord() - p1.xcoord());

                if (cmp2(area_diff)) {
                    return vcurr;
                }
                vcurr = vnext;
            } while (vcurr != vstart);

            return nullptr;
        };

        auto vcurr = find_implicit_concave_point(v1, cmp);

        if (vcurr == nullptr) {
            std::vector<size_t> indices;
            auto current = v1;
            do {
                indices.push_back(current->data);
                current = current->next;
            } while (current != v1);
            return {std::move(indices)};
        }

        auto find_min_dist_point
            = [&lst](Dllink<size_t>* vcurr) -> std::pair<Dllink<size_t>*, bool> {
            auto vnext = vcurr->next;
            auto vstop = vcurr;
            auto vi = vnext->next;

            T min_value = std::numeric_limits<T>::max();
            bool vertical = true;
            Dllink<size_t>* v_min = vcurr;
            const auto& pc1 = lst[vcurr->data];
            const auto& pc2 = lst[vnext->data];
            const auto pcurr = Point<T>(pc2.xcoord(), pc1.ycoord());

            while (vi != vstop) {
                const auto& p0 = lst[vi->prev->data];
                const auto& p1 = lst[vi->data];
                const auto& p2 = lst[vi->next->data];

                auto vec_i = p1 - pcurr;

                if ((p0.ycoord() < pcurr.ycoord() && pcurr.ycoord() <= p1.ycoord())
                    || (p1.ycoord() <= pcurr.ycoord() && pcurr.ycoord() < p0.ycoord())) {
                    if (min_value > std::abs(vec_i.x())) {
                        min_value = std::abs(vec_i.x());
                        v_min = vi;
                        vertical = true;
                    }
                }

                if ((p2.xcoord() < pcurr.xcoord() && pcurr.xcoord() <= p1.xcoord())
                    || (p1.xcoord() <= pcurr.xcoord() && pcurr.xcoord() < p2.xcoord())) {
                    if (min_value > std::abs(vec_i.y())) {
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
        size_t n = lst.size();
        rdll.cycle.emplace_back(Dllink<size_t>(n));
        auto new_node = &rdll.cycle[n];
        const auto& p_min = lst[v_min->data];
        const auto& pc1 = lst[vcurr->data];
        const auto& pc2 = lst[vcurr->next->data];
        const auto p1 = Point<T>(pc2.xcoord(), pc1.ycoord());

        Point<T> p_new;
        auto vnext = vcurr->next;
        if (vertical) {
            new_node->next = vnext;
            new_node->prev = v_min->prev;
            v_min->prev->next = new_node;
            vnext->prev = new_node;
            vcurr->next = v_min;
            v_min->prev = vcurr;
            p_new = Point<T>(p_min.xcoord(), p1.ycoord());
        } else {
            new_node->prev = vcurr;
            new_node->next = v_min->next;
            v_min->next->prev = new_node;
            vcurr->next = new_node;
            vnext->prev = v_min;
            v_min->next = vnext;
            p_new = Point<T>(p1.xcoord(), p_min.ycoord());
        }

        lst.push_back(p_new);

        auto L1 = rpolygon_cut_implicit_recur(v_min, lst, cmp, rdll);
        auto L2 = rpolygon_cut_implicit_recur(new_node, lst, cmp, rdll);
        L1.insert(L1.end(), L2.begin(), L2.end());
        return L1;
    }

    template std::vector<std::vector<size_t>> rpolygon_cut_convex_recur<int>(
        Dllink<size_t>*, std::vector<Point<int>>&,
        const std::function<bool(int)>&, RDllist&);

    template std::vector<std::vector<size_t>> rpolygon_cut_explicit_recur<int>(
        Dllink<size_t>*, std::vector<Point<int>>&,
        const std::function<bool(int)>&, RDllist&);

    template std::vector<std::vector<size_t>> rpolygon_cut_implicit_recur<int>(
        Dllink<size_t>*, std::vector<Point<int>>&,
        const std::function<bool(int)>&, RDllist&);

}