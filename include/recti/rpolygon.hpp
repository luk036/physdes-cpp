#pragma once

#include <algorithm>
#include <gsl/span>
#include <vector>

#include "recti.hpp"

namespace recti {

    /**
     * @brief Rectilinear Polygon
     *
     * @tparam T
     */
    template <typename T> class RPolygon {
      private:
        Point<T> _origin;
        std::vector<Vector2<T>> _vecs;  // @todo: add custom allocator support

      public:
        /**
         * @brief Construct a new RPolygon object
         *
         * @param[in] pointset
         */
        explicit constexpr RPolygon(gsl::span<const Point<T>> pointset)
            : _origin{pointset.front()} {
            auto it = pointset.begin();
            for (++it; it != pointset.end(); ++it) {
                this->_vecs.push_back(*it - this->_origin);
            }
        }

        /**
         * @brief
         *
         * @param[in] rhs
         * @return constexpr Point&
         */
        constexpr auto operator+=(const Vector2<T>& rhs) -> RPolygon& {
            this->_origin += rhs;
            return *this;
        }

        /**
         * @brief
         *
         * @return T
         */
        [[nodiscard]] constexpr auto signed_area() const -> T {
            assert(this->_vecs.size() >= 1);

            auto it = this->_vecs.begin();
            auto res = it->x() * it->y();
            for (++it; it != this->_vecs.end(); ++it) {
                res += it->x() * (it->y() - std::prev(it)->y());
            }
            return res;
        }

        /**
         * @brief
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U> auto contains(const Point<U>& rhs) const -> bool;

        /**
         * @brief
         *
         * @return Point<T>
         */
        [[nodiscard]] auto lb() const -> Point<T>;

        /**
         * @brief
         *
         * @return Point<T>
         */
        [[nodiscard]] auto ub() const -> Point<T>;
    };

    /**
     * @brief Create a xmono RPolygon object
     *
     * @tparam FwIter
     * @param[in] first
     * @param[in] last
     * @return true
     * @return false
     */
    template <typename FwIter> inline auto create_xmono_rpolygon(FwIter&& first, FwIter&& last)
        -> bool {
        assert(first != last);

        const auto leftmost = *std::min_element(first, last);
        const auto rightmost = *std::max_element(first, last);
        const auto is_anticlockwise = rightmost.y() <= leftmost.y();
        auto r2l = [&](const auto& a) { return a.y() <= leftmost.y(); };
        auto l2r = [&](const auto& a) { return a.y() >= leftmost.y(); };
        const auto middle = is_anticlockwise ? std::partition(first, last, std::move(r2l))
                                             : std::partition(first, last, std::move(l2r));
        std::sort(first, middle);
        std::sort(middle, last, std::greater<>());
        return is_anticlockwise;
    }

    /**
     * @brief Create a ymono RPolygon object
     *
     * @tparam FwIter
     * @param[in] first
     * @param[in] last
     * @return true
     * @return false
     */
    template <typename FwIter> inline auto create_ymono_rpolygon(FwIter&& first, FwIter&& last)
        -> bool {
        assert(first != last);

        auto upward = [](const auto& a, const auto& b) {
            return std::tie(a.y(), a.x()) < std::tie(b.y(), b.x());
        };
        auto downward = [](const auto& a, const auto& b) {
            return std::tie(a.y(), a.x()) > std::tie(b.y(), b.x());
        };
        const auto botmost = *std::min_element(first, last, upward);
        const auto topmost = *std::max_element(first, last, upward);
        const auto is_anticlockwise = topmost.x() >= botmost.x();
        auto r2l = [&](const auto& a) { return a.x() >= botmost.x(); };
        auto l2r = [&](const auto& a) { return a.x() <= botmost.x(); };
        const auto middle = is_anticlockwise ? std::partition(first, last, std::move(r2l))
                                             : std::partition(first, last, std::move(l2r));
        std::sort(first, middle, std::move(upward));
        std::sort(middle, last, std::move(downward));
        return is_anticlockwise;
    }

    /**
     * @brief
     *
     * @tparam T
     * @tparam FwIter
     * @param[in] first
     * @param[in] last
     */
    template <typename FwIter> inline void create_test_rpolygon(FwIter&& first, FwIter&& last) {
        assert(first != last);

        auto up = [](const auto& a, const auto& b) {
            return std::tie(a.y(), a.x()) < std::tie(b.y(), b.x());
        };
        auto down = [](const auto& a, const auto& b) {
            return std::tie(a.y(), a.x()) > std::tie(b.y(), b.x());
        };
        auto left = [](const auto& a, const auto& b) {
            return std::tie(a.x(), a.y()) < std::tie(b.x(), b.y());
        };
        auto right = [](const auto& a, const auto& b) {
            return std::tie(a.x(), a.y()) > std::tie(b.x(), b.y());
        };

        auto min_pt = *std::min_element(first, last, up);
        auto max_pt = *std::max_element(first, last, up);
        auto dx = max_pt.x() - min_pt.x();
        auto dy = max_pt.y() - min_pt.y();
        auto middle = std::partition(first, last, [&](const auto& a) {
            return dx * (a.y() - min_pt.y()) < (a.x() - min_pt.x()) * dy;
        });

        auto max_pt1 = *std::max_element(first, middle, left);
        auto middle2
            = std::partition(first, middle, [&](const auto& a) { return a.y() < max_pt1.y(); });

        auto min_pt2 = *std::min_element(middle, last, left);
        auto middle3
            = std::partition(middle, last, [&](const auto& a) { return a.y() > min_pt2.y(); });

        if (dx < 0)  // clockwise
        {
            std::sort(first, middle2, down);
            std::sort(middle2, middle, left);
            std::sort(middle, middle3, up);
            std::sort(middle3, last, right);
        } else  // anti-clockwise
        {
            std::sort(first, middle2, left);
            std::sort(middle2, middle, up);
            std::sort(middle, middle3, right);
            std::sort(middle3, last, down);
        }
    }

    /**
     * @brief determine if a Point is within a Polygon
     *
     * The code below is from Wm. Randolph Franklin <wrf@ecse.rpi.edu>
     * (see URL below) with some minor modifications for rectilinear. It returns
     * true for strictly interior points, false for strictly exterior, and ub
     * for points on the boundary.  The boundary behavior is complex but
     * determined; in particular, for a partition of a region into polygons,
     * each Point is "in" exactly one Polygon.
     * (See p.243 of [O'Rourke (C)] for a discussion of boundary behavior.)
     *
     * See http://www.faqs.org/faqs/graphics/algorithms-faq/ Subject 2.03
     *
     * @tparam T
     * @param[in] S
     * @param[in] q
     * @return true
     * @return false
     */
    template <typename T>
    inline auto point_in_rpolygon(gsl::span<const Point<T>> S, const Point<T>& q) -> bool {
        auto c = false;
        auto p0 = S.back();
        for (auto&& p1 : S) {
            if ((p1.y() <= q.y() && q.y() < p0.y()) || (p0.y() <= q.y() && q.y() < p1.y())) {
                if (p1.x() > q.x()) {
                    c = !c;
                }
            }
            p0 = p1;
        }
        return c;
    }

    /**
     * @brief Polygon is clockwise
     *
     * @tparam T
     * @param[in] S
     * @return true
     * @return false
     */
    template <typename T> inline auto rpolygon_is_clockwise(gsl::span<const Point<T>> S) -> bool {
        auto it1 = std::min_element(S.begin(), S.end());
        auto it0 = it1 != S.begin() ? std::prev(it1) : S.end() - 1;
        if (it1->y() < it0->y()) return false;
        if (it1->y() > it0->y()) return true;
        // it1->y() == it0->y()
        auto it2 = std::next(it1) != S.end() ? std::next(it1) : S.begin();
        return it2->y() > it1->y();
    }

}  // namespace recti
