#pragma once

#include <algorithm>
#include <gsl/span>
#include <vector>

#include "recti.hpp"

namespace recti {

    /**
     * @brief Polygon
     *
     * @tparam T
     */
    template <typename T> class polygon {
      private:
        point<T> _origin;
        std::vector<vector2<T>> _vecs;

      public:
        /**
         * @brief Construct a new polygon object
         *
         * @param[in] pointset
         */
        explicit constexpr polygon(gsl::span<const point<T>> pointset) : _origin{pointset.front()} {
            auto it = pointset.begin();
            for (++it; it != pointset.end(); ++it) {
                this->_vecs.push_back(*it - this->_origin);
            }
        }

        /**
         * @brief
         *
         * @param[in] rhs
         * @return constexpr point&
         */
        constexpr auto operator+=(const vector2<T>& rhs) -> polygon& {
            this->_origin += rhs;
            return *this;
        }

        /**
         * @brief
         *
         * @return T
         */
        [[nodiscard]] constexpr auto signed_area_x2() const -> T {
            auto&& vs = this->_vecs;
            auto n = int(vs.size());
            assert(n >= 2);
            auto res = vs[0].x() * vs[1].y() - vs[n - 1].x() * vs[n - 2].y();
            for (auto i = 1; i != n - 1; ++i) {
                res += vs[i].x() * (vs[i + 1].y() - vs[i - 1].y());
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
        template <typename U> auto contains(const point<U>& rhs) const -> bool;

        /**
         * @brief
         *
         * @return point<T>
         */
        [[nodiscard]] auto lower() const -> point<T>;

        /**
         * @brief
         *
         * @return point<T>
         */
        [[nodiscard]] auto upper() const -> point<T>;
    };

    /**
     * @brief
     *
     * @tparam Stream
     * @tparam T
     * @param[out] out
     * @param[in] r
     * @return Stream&
     */
    template <class Stream, typename T> auto operator<<(Stream& out, const polygon<T>& r)
        -> Stream& {
        for (auto&& p : r) {
            out << "  \\draw " << p << ";\n";
        }
        return out;
    }

    /**
     * @brief Create a ymono polygon object
     *
     * @tparam FwIter
     * @param[in] first
     * @param[in] last
     */
    template <typename FwIter, typename Compare>
    inline void create_mono_polygon(FwIter&& first, FwIter&& last, Compare&& dir) {
        assert(first != last);

        auto max_pt = *std::max_element(first, last, dir);
        auto min_pt = *std::min_element(first, last, dir);
        auto d = max_pt - min_pt;
        auto middle
            = std::partition(first, last, [&](const auto& a) { return d.cross(a - min_pt) <= 0; });
        std::sort(first, middle, dir);
        std::sort(middle, last, dir);
        std::reverse(middle, last);
    }

    /**
     * @brief Create a xmono polygon object
     *
     * @tparam FwIter
     * @param[in] first
     * @param[in] last
     */
    template <typename FwIter> inline void create_xmono_polygon(FwIter&& first, FwIter&& last) {
        return create_mono_polygon(first, last, std::less<>());
    }

    /**
     * @brief Create a ymono polygon object
     *
     * @tparam FwIter
     * @param[in] first
     * @param[in] last
     */
    template <typename FwIter> inline void create_ymono_polygon(FwIter&& first, FwIter&& last) {
        return create_mono_polygon(first, last, [](const auto& a, const auto& b) {
            return std::tie(a.y(), a.x()) < std::tie(b.y(), b.x());
        });
    }

    /**
     * @brief determine if a point is within a polygon
     *
     * The code below is from Wm. Randolph Franklin <wrf@ecse.rpi.edu>
     * (see URL below) with some minor modifications for integer. It returns
     * true for strictly interior points, false for strictly exterior, and ub
     * for points on the boundary.  The boundary behavior is complex but
     * determined; in particular, for a partition of a region into polygons,
     * each point is "in" exactly one polygon.
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
    inline auto point_in_polygon(gsl::span<const point<T>> S, const point<T>& q) -> bool {
        auto c = false;
        auto p0 = S.back();
        for (auto&& p1 : S) {
            if ((p1.y() <= q.y() && q.y() < p0.y()) || (p0.y() <= q.y() && q.y() < p1.y())) {
                auto d = (q - p0).cross(p1 - p0);
                if (p1.y() > p0.y()) {
                    if (d < 0) {
                        c = !c;
                    }
                } else {  // v1.y() < v0.y()
                    if (d > 0) {
                        c = !c;
                    }
                }
            }
            p0 = p1;
        }
        return c;
    }

}  // namespace recti
