#pragma once

#include <algorithm>
#include <gsl/span>
#include <utility> // for std::pair
#include <vector>

#include "recti.hpp"

namespace recti {

/**
 * @brief Polygon
 *
 * @tparam T
 */
template <typename T> class Polygon {
private:
  Point<T> _origin{};
  std::vector<Vector2<T>> _vecs{};

public:
  /**
   * @brief Construct a new Polygon object
   *
   * @param[in] pointset
   */
  explicit constexpr Polygon(gsl::span<const Point<T>> pointset)
      : _origin{pointset.front()} {
    auto itr = pointset.begin();
    for (++itr; itr != pointset.end(); ++itr) {
      this->_vecs.push_back(*itr - this->_origin);
    }
  }

  /**
   * @brief
   *
   * @param[in] rhs
   * @return constexpr Point&
   */
  constexpr auto operator+=(const Vector2<T> &rhs) -> Polygon & {
    this->_origin += rhs;
    return *this;
  }

  /**
   * @brief
   *
   * @return T
   */
  [[nodiscard]] constexpr auto signed_area_x2() const -> T {
    auto first = this->_vecs.begin();
    auto second = std::next(first);
    auto end = this->_vecs.end();
    auto last = std::prev(end);
    auto res = first->x() * second->y() - last->x() * std::prev(last)->y();
    for (auto itr = second; itr != last; ++itr) {
      res += itr->x() * (std::next(itr)->y() - std::prev(itr)->y());
    }
    return res;
  }

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
 * @brief
 *
 * @tparam Stream
 * @tparam T
 * @param[out] out
 * @param[in] r
 * @return Stream&
 */
template <class Stream, typename T>
auto operator<<(Stream &out, const Polygon<T> &poly) -> Stream & {
  for (auto &&vtx : poly) {
    out << "  \\draw " << vtx << ";\n";
  }
  return out;
}

/**
 * @brief Create a mono Polygon object
 *
 * @tparam FwIter
 * @tparam Compare
 * @param[in] first
 * @param[in] last
 * @param dir
 */
template <typename FwIter, typename Compare>
inline void create_mono_polygon(FwIter &&first, FwIter &&last, Compare &&dir) {
  assert(first != last);

  auto [min, max] = std::minmax_element(first, last, dir);
  auto min_pt = *min;
  auto max_pt = *max;
  auto displace = max_pt - min_pt;
  auto middle = std::partition(first, last,
                               [&displace, &min_pt](const auto &elem) -> bool {
                                 return displace.cross(elem - min_pt) <= 0;
                               });
  std::sort(first, middle, dir);
  std::sort(middle, last, dir);
  std::reverse(middle, last);
}

/**
 * @brief Create a xmono Polygon object
 *
 * @tparam FwIter
 * @param[in] first
 * @param[in] last
 */
template <typename FwIter>
inline auto create_xmono_polygon(FwIter &&first, FwIter &&last) -> void {
  return create_mono_polygon(first, last,
                             [](const auto &lhs, const auto &rhs) -> bool {
                               return std::pair(lhs.xcoord(), lhs.ycoord()) <
                                      std::pair(rhs.xcoord(), rhs.ycoord());
                             });
}

/**
 * @brief Create a ymono Polygon object
 *
 * @tparam FwIter
 * @param[in] first
 * @param[in] last
 */
template <typename FwIter>
inline auto create_ymono_polygon(FwIter &&first, FwIter &&last) -> void {
  return create_mono_polygon(first, last,
                             [](const auto &lhs, const auto &rhs) -> bool {
                               return std::pair(lhs.ycoord(), lhs.xcoord()) <
                                      std::pair(rhs.ycoord(), rhs.xcoord());
                             });
}

/**
 * @brief determine if a Point is within a Polygon
 *
 * The code below is from Wm. Randolph Franklin <wrf@ecse.rpi.edu>
 * (see URL below) with some minor modifications for integer. It returns
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
inline auto point_in_polygon(gsl::span<const Point<T>> pointset,
                             const Point<T> &ptq) -> bool {
  auto res = false;
  auto pt0 = pointset.back();
  for (auto &&pt1 : pointset) {
    if ((pt1.ycoord() <= ptq.ycoord() && ptq.ycoord() < pt0.ycoord()) ||
        (pt0.ycoord() <= ptq.ycoord() && ptq.ycoord() < pt1.ycoord())) {
      auto det = (ptq - pt0).cross(pt1 - pt0);
      if (pt1.ycoord() > pt0.ycoord()) {
        if (det < 0) {
          res = !res;
        }
      } else { // v1.ycoord() < v0.ycoord()
        if (det > 0) {
          res = !res;
        }
      }
    }
    pt0 = pt1;
  }
  return res;
}

/**
 * @brief Polygon is clockwise
 *
 * @tparam T
 * @param[in] S
 * @return true
 * @return false
 */
template <typename T>
inline auto polygon_is_clockwise(gsl::span<const Point<T>> pointset) -> bool {
  auto it1 = std::min_element(pointset.begin(), pointset.end());
  auto it0 =
      it1 != pointset.begin() ? std::prev(it1) : std::prev(pointset.end());
  auto it2 =
      std::next(it1) != pointset.end() ? std::next(it1) : pointset.begin();
  return (*it1 - *it0).cross(*it2 - *it1) < 0;
}

} // namespace recti
