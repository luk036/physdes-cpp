#include <algorithm>
#include <cstdlib>
#include <limits>
#include <recti/dllink.hpp>
#include <recti/point.hpp>
#include <recti/rpolygon_cut.hpp>
#include <span>
#include <utility>
#include <vector>

namespace recti {
    namespace {

        template <typename T>
        auto canonicalize_full(std::vector<Point<T>> pts) -> std::vector<Point<T>> {
            bool changed = true;
            while (changed && pts.size() >= 2) {
                changed = false;
                for (std::size_t i = 0; i < pts.size(); ++i) {
                    const std::size_t j = (i + 1) % pts.size();
                    if (pts[i] == pts[j]) {
                        pts.erase(pts.begin() + static_cast<std::ptrdiff_t>(j));
                        changed = true;
                        break;
                    }
                }
            }
            if (pts.size() < 4) {
                return pts;
            }

            changed = true;
            while (changed && pts.size() >= 4) {
                changed = false;
                for (std::size_t i = 0; i < pts.size(); ++i) {
                    const auto& a = pts[(i + pts.size() - 1) % pts.size()];
                    const auto& b = pts[i];
                    const auto& c = pts[(i + 1) % pts.size()];
                    if ((a.xcoord() == b.xcoord() && b.xcoord() == c.xcoord())
                        || (a.ycoord() == b.ycoord() && b.ycoord() == c.ycoord())) {
                        pts.erase(pts.begin() + static_cast<std::ptrdiff_t>(i));
                        changed = true;
                        break;
                    }
                }
            }
            return pts;
        }

        template <typename T, typename Key>
        auto cycle_is_monotone(Dllink<size_t>* start, const std::vector<Point<T>>& pts,
                               const Key& key) -> bool {
            auto* min_node = start;
            auto* max_node = start;
            for (auto* cur = start->next; cur != start; cur = cur->next) {
                if (key(pts[cur->data]) < key(pts[min_node->data])) min_node = cur;
                if (key(pts[cur->data]) > key(pts[max_node->data])) max_node = cur;
            }
            for (auto* cur = min_node; cur != max_node; cur = cur->next) {
                if (key(pts[cur->data]) > key(pts[cur->next->data])) return false;
            }
            for (auto* cur = max_node; cur != min_node; cur = cur->next) {
                if (key(pts[cur->data]) < key(pts[cur->next->data])) return false;
            }
            return true;
        }

        template <typename T>
        auto cycle_is_convex(Dllink<size_t>* start, const std::vector<Point<T>>& pts) -> bool {
            const auto x_key = [](const Point<T>& p) { return p.xcoord(); };
            const auto y_key = [](const Point<T>& p) { return p.ycoord(); };
            return cycle_is_monotone(start, pts, x_key) && cycle_is_monotone(start, pts, y_key);
        }

        template <typename T>
        auto reflex_node(Dllink<size_t>* start, const std::vector<Point<T>>& pts, bool is_ccw)
            -> Dllink<size_t>* {
            for (auto* cur = start;; cur = cur->next) {
                const auto& a = pts[cur->prev->data];
                const auto& b = pts[cur->data];
                const auto& c = pts[cur->next->data];
                const T cross = (b.xcoord() - a.xcoord()) * (c.ycoord() - b.ycoord())
                                - (b.ycoord() - a.ycoord()) * (c.xcoord() - b.xcoord());
                if ((is_ccw && cross < T{0}) || (!is_ccw && cross > T{0})) {
                    return cur;
                }
                if (cur->next == start) break;
            }
            return nullptr;
        }

        struct CutTarget {
            Dllink<size_t>* edge_start = nullptr;
            bool vertical = true;
            long long distance = 0;
        };

        template <typename T>
        auto find_cut_target(Dllink<size_t>* start, Dllink<size_t>* reflex,
                             const std::vector<Point<T>>& pts) -> CutTarget {
            const auto& v = pts[reflex->data];

            int horizontal_dir = 0;
            int vertical_dir = 0;
            for (const auto* neighbor : {reflex->prev, reflex->next}) {
                const auto& q = pts[neighbor->data];
                if (q.ycoord() == v.ycoord() && q.xcoord() != v.xcoord()) {
                    horizontal_dir = (q.xcoord() > v.xcoord()) ? 1 : -1;
                }
                if (q.xcoord() == v.xcoord() && q.ycoord() != v.ycoord()) {
                    vertical_dir = (q.ycoord() > v.ycoord()) ? 1 : -1;
                }
            }

            CutTarget best;
            best.distance = std::numeric_limits<long long>::max();
            bool found = false;

            auto* a = start;
            do {
                auto* b = a->next;
                if (a != reflex && b != reflex) {
                    const auto& pa = pts[a->data];
                    const auto& pb = pts[b->data];
                    if (pa.xcoord() == pb.xcoord()) {
                        const long long lo = std::min(pa.ycoord(), pb.ycoord());
                        const long long hi = std::max(pa.ycoord(), pb.ycoord());
                        if (lo <= v.ycoord() && v.ycoord() <= hi) {
                            const long long delta = static_cast<long long>(pa.xcoord()) - v.xcoord();
                            if (horizontal_dir == 0 || delta * horizontal_dir < 0) {
                                const long long dist = std::abs(delta);
                                if (!found || dist < best.distance) {
                                    best = {a, true, dist};
                                    found = true;
                                }
                            }
                        }
                    } else if (pa.ycoord() == pb.ycoord()) {
                        const long long lo = std::min(pa.xcoord(), pb.xcoord());
                        const long long hi = std::max(pa.xcoord(), pb.xcoord());
                        if (lo <= v.xcoord() && v.xcoord() <= hi) {
                            const long long delta = static_cast<long long>(pa.ycoord()) - v.ycoord();
                            if (vertical_dir == 0 || delta * vertical_dir < 0) {
                                const long long dist = std::abs(delta);
                                if (!found || dist < best.distance) {
                                    best = {a, false, dist};
                                    found = true;
                                }
                            }
                        }
                    }
                }
                a = b;
            } while (a != start);
            return best;
        }

        template <typename T>
        auto collect_indices(Dllink<size_t>* start) -> std::vector<std::size_t> {
            std::vector<std::size_t> indices;
            auto* cur = start;
            do {
                indices.push_back(cur->data);
                cur = cur->next;
            } while (cur != start);
            return indices;
        }

        template <typename T>
        auto cut_convex_full_recur(Dllink<size_t>* start, std::vector<Point<T>>& pts,
                                   std::vector<Dllink<size_t>>& nodes, bool is_ccw)
            -> std::vector<std::vector<std::size_t>> {
            if (cycle_is_convex(start, pts)) {
                return {collect_indices<T>(start)};
            }

            auto* reflex = reflex_node(start, pts, is_ccw);
            if (reflex == nullptr) {
                return {collect_indices<T>(start)};
            }

            const auto target = find_cut_target(start, reflex, pts);
            if (target.edge_start == nullptr) {
                return {collect_indices<T>(start)};
            }

            auto* a = target.edge_start;
            auto* b = a->next;
            auto* before_reflex = reflex->prev;
            auto* after_reflex = reflex->next;
            const auto& pa = pts[a->data];
            const auto& v = pts[reflex->data];

            const Point<T> cut_point = target.vertical
                                           ? Point<T>(pa.xcoord(), v.ycoord())
                                           : Point<T>(v.xcoord(), pa.ycoord());
            const std::size_t cut_index = pts.size();
            pts.push_back(cut_point);

            nodes.emplace_back(reflex->data);
            auto* v1 = &nodes.back();
            nodes.emplace_back(cut_index);
            auto* p1 = &nodes.back();
            nodes.emplace_back(cut_index);
            auto* p2 = &nodes.back();
            nodes.emplace_back(reflex->data);
            auto* v2 = &nodes.back();

            v1->next = after_reflex;
            after_reflex->prev = v1;
            a->next = p1;
            p1->prev = a;
            p1->next = v1;
            v1->prev = p1;

            b->prev = p2;
            p2->next = b;
            before_reflex->next = v2;
            v2->prev = before_reflex;
            v2->next = p2;
            p2->prev = v2;

            auto left = cut_convex_full_recur<T>(v1, pts, nodes, is_ccw);
            auto right = cut_convex_full_recur<T>(p2, pts, nodes, is_ccw);
            left.insert(left.end(), right.begin(), right.end());
            return left;
        }

    }  // namespace

    template <typename T>
    auto rpolygon_cut_convex_full(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<std::vector<Point<T>>> {
        if (pointset.size() < 4) {
            return {};
        }

        std::vector<Point<T>> pts(pointset.begin(), pointset.end());
        std::vector<Dllink<size_t>> nodes;
        nodes.reserve(5 * pts.size());
        for (std::size_t i = 0; i < pts.size(); ++i) {
            nodes.emplace_back(i);
        }
        for (std::size_t i = 0; i < pts.size(); ++i) {
            nodes[i].next = &nodes[(i + 1) % nodes.size()];
            nodes[i].prev = &nodes[(i + nodes.size() - 1) % nodes.size()];
        }

        auto index_lists = cut_convex_full_recur<T>(&nodes[0], pts, nodes, is_anticlockwise);

        std::vector<std::vector<Point<T>>> result;
        result.reserve(index_lists.size());
        for (const auto& indices : index_lists) {
            std::vector<Point<T>> polygon;
            polygon.reserve(indices.size());
            for (const auto index : indices) {
                polygon.push_back(pts[index]);
            }
            polygon = canonicalize_full(std::move(polygon));
            if (polygon.size() >= 4) {
                result.push_back(std::move(polygon));
            }
        }
        return result;
    }

    template auto rpolygon_cut_convex_full<int>(std::span<const Point<int>>, bool)
        -> std::vector<std::vector<Point<int>>>;

}  // namespace recti
