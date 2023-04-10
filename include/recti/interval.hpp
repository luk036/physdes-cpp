#pragma once

// #include <algorithm> // import std::min, std::max
#include <cassert>
#include <cmath>
#include <utility>     // import std::move

namespace recti {

// /**
//  * @brief overlap 1
//  *
//  * @tparam U1
//  * @tparam U2
//  * @param lhs
//  * @param rhs
//  * @return std::enable_if<!std::is_scalar<U1>::value, bool>::type
//  */
// template <typename U1, typename U2> //
// inline constexpr auto overlap(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<!std::is_scalar<U1>::value, bool>::type {
//   return lhs.overlaps(rhs);
// }

// /**
//  * @brief overlap 2
//  *
//  * @tparam U1
//  * @tparam U2
//  * @param lhs
//  * @param rhs
//  * @return std::enable_if<
//  * std::is_scalar<U1>::value && !std::is_scalar<U2>::value, bool>::type
//  */
// template <typename U1, typename U2> //
// inline constexpr auto overlap(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && !std::is_scalar<U2>::value, bool>::type
//         {
//   return rhs.overlaps(lhs);
// }

// /**
//  * @brief overlap 3
//  *
//  * @tparam U1
//  * @tparam U2
//  * @param lhs
//  * @param rhs
//  * @return std::enable_if<
//  * std::is_scalar<U1>::value && std::is_scalar<U2>::value, bool>::type
//  */
// template <typename U1, typename U2> //
// inline constexpr auto overlap(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && std::is_scalar<U2>::value, bool>::type {
//   return lhs == rhs;
// }

/**
 * @brief overlap
 *
 * @tparam U1
 * @tparam U2
 * @param lhs
 * @param rhs
 * @return true
 * @return false
 */
template <typename U1, typename U2> //
inline constexpr auto overlap(const U1 &lhs, const U2 &rhs) -> bool {
  if constexpr (requires { lhs.overlaps(rhs); }) {
    return lhs.overlaps(rhs);
  } else if constexpr (requires { rhs.overlaps(lhs); }) {
    return rhs.overlaps(lhs);
  } else /* constexpr */ {
    return lhs == rhs;
  }
}

// template <typename U1, typename U2> //
// inline constexpr auto contain(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<!std::is_scalar<U1>::value, bool>::type {
//   return lhs.contains(rhs);
// }

// template <typename U1, typename U2> //
// inline constexpr auto contain(const U1 &, const U2 &) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && !std::is_scalar<U2>::value, bool>::type
//         {
//   return false;
// }

// template <typename U1, typename U2> //
// inline constexpr auto contain(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && std::is_scalar<U2>::value, bool>::type {
//   return lhs == rhs;
// }

/**
 * @brief contain
 *
 * @tparam U1
 * @tparam U2
 * @param lhs
 * @param rhs
 * @return true
 * @return false
 */
template <typename U1, typename U2> //
inline constexpr auto contain(const U1 &lhs, const U2 &rhs) -> bool {
  if constexpr (requires { lhs.contains(rhs); }) {
    return lhs.contains(rhs);
  } else if constexpr (requires { rhs.contains(lhs); }) {
    return false;
  } else /* constexpr */ {
    return lhs == rhs;
  }
}

// template <typename U1, typename U2> //
// constexpr auto intersection(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<!std::is_scalar<U1>::value,
//                             decltype(lhs.intersection_with(rhs))>::type {
//   return lhs.intersection_with(rhs);
// }

// template <typename U1, typename U2> //
// constexpr auto intersection(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && !std::is_scalar<U2>::value, U1>::type {
//   return rhs.intersection_with(lhs);
// }

// template <typename U1, typename U2> //
// constexpr auto intersection(const U1 &lhs, const U2 &) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && std::is_scalar<U2>::value, U1>::type {
//   // assert(lhs == rhs);
//   return lhs;
// }

/**
 * @brief intersection
 *
 * @tparam U1
 * @tparam U2
 * @param lhs
 * @param rhs
 * @return constexpr auto
 */
template <typename U1, typename U2> //
constexpr auto intersection(const U1 &lhs, const U2 &rhs) {
  if constexpr (requires { lhs.intersection_with(rhs); }) {
    return lhs.intersection_with(rhs);
  } else if constexpr (requires { rhs.intersection_with(lhs); }) {
    return rhs.intersection_with(lhs);
  } else /* constexpr */ {
    assert(lhs == rhs);
    return lhs;
  }
}

// template <typename U1, typename U2> //
// constexpr auto min_dist(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<!std::is_scalar<U1>::value,
//                             decltype(lhs.min_dist_with(rhs))>::type {
//   return lhs.min_dist_with(rhs);
// }

// template <typename U1, typename U2> //
// constexpr auto min_dist(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && !std::is_scalar<U2>::value, U1>::type {
//   return rhs.min_dist_with(lhs);
// }

// template <typename U1, typename U2> //
// constexpr auto min_dist(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && std::is_scalar<U2>::value, U1>::type {
//   return std::abs(lhs - rhs);
// }

/**
 * @brief min_dist
 *
 * @tparam U1
 * @tparam U2
 * @param lhs
 * @param rhs
 * @return constexpr auto
 */
template <typename U1, typename U2> //
inline constexpr auto min_dist(const U1 &lhs, const U2 &rhs) {
  if constexpr (requires { lhs.min_dist_with(rhs); }) {
    return lhs.min_dist_with(rhs);
  } else if constexpr (requires { rhs.min_dist_with(lhs); }) {
    return rhs.min_dist_with(lhs);
  } else /* constexpr */ {
    return std::abs(lhs - rhs);
  }
}

// template <typename U1, typename U2> //
// constexpr auto min_dist_change(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<!std::is_scalar<U1>::value,
//                             decltype(lhs.min_dist_change_with(rhs))>::type {
//   return lhs.min_dist_with(rhs);
// }

// template <typename U1, typename U2> //
// constexpr auto min_dist_change(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && !std::is_scalar<U2>::value, U1>::type {
//   return rhs.min_dist_change_with(lhs);
// }

// template <typename U1, typename U2> //
// constexpr auto min_dist_change(const U1 &lhs, const U2 &rhs) ->
//     typename std::enable_if<
//         std::is_scalar<U1>::value && std::is_scalar<U2>::value, U1>::type {
//   return std::abs(lhs - rhs);
// }

/**
 * @brief min_dist with change
 *
 * @tparam U1
 * @tparam U2
 * @param lhs
 * @param rhs
 * @return constexpr auto
 */
template <typename U1, typename U2> //
inline constexpr auto min_dist_change(U1 &lhs, U2 &rhs) {
  if constexpr (requires { lhs.min_dist_change_with(rhs); }) {
    return lhs.min_dist_change_with(rhs);
  } else if constexpr (requires { rhs.min_dist_change_with(lhs); }) {
    return rhs.min_dist_change_with(lhs);
  } else /* constexpr */ {
    return std::abs(lhs - rhs);
  }
}

#pragma pack(push, 1)
/**
 * @brief Interval
 *
 * @tparam T
 */
template <typename T = int> class Interval {
private:
  T _lb; //> lower bound
  T _ub; //> upper bound

public:
  using value_type = T;

  /**
   * @brief Construct a new Interval object
   *
   * @param[in] lower
   * @param[in] upper
   */
  constexpr Interval(T &&lower, T &&upper) noexcept
      : _lb{std::move(lower)}, _ub{std::move(upper)} {}

  /**
   * @brief Construct a new Interval object
   *
   * @param[in] lower
   * @param[in] upper
   */
  constexpr Interval(const T &lower, const T &upper) : _lb{lower}, _ub{upper} {}

  /**
   * @brief Construct a new Interval object
   *
   * @param[in] c
   */
  explicit constexpr Interval(const T &alpha) : _lb{alpha}, _ub{alpha} {}

  /**
   * @brief Assignment operator
   *
   * @param c
   * @return Interval&
   */
  constexpr auto operator=(const T &alpha) -> Interval & {
    this->_lb = this->_ub = alpha;
    return *this;
  }

  /**
   * @brief
   *
   * @return const T&
   */
  constexpr auto lb() const -> const T & { return this->_lb; }

  /**
   * @brief
   *
   * @return const T&
   */
  constexpr auto ub() const -> const T & { return this->_ub; }

  /**
   * @brief
   *
   * @return constexpr T
   */
  constexpr auto len() const -> T { return this->ub() - this->lb(); }

  /** @name Comparison operators
   *  definie ==, !=, <, >, <=, >=.
   */
  ///@{

  /**
   * @brief Equal to
   *
   * @tparam U
   * @param[in] rhs
   * @return true
   * @return false
   */
  template <typename U> //
  constexpr auto operator==(const Interval<U> &rhs) const -> bool {
    return this->lb() == rhs.lb() && this->ub() == rhs.ub();
  }

  /**
   * @brief Not equal to
   *
   * @tparam U
   * @param[in] rhs
   * @return true
   * @return false
   */
  template <typename U> //
  constexpr auto operator!=(const Interval<U> &rhs) const -> bool {
    return !(*this == rhs);
  }

  /**
   * @brief Spaceship operator
   *
   * @tparam U
   * @param[in] rhs
   * @return std::weak_ordering
   */
  template <typename U>  //
  constexpr auto operator<=>(const U& rhs) const -> std::weak_ordering {
      if (this->ub() < rhs) return std::weak_ordering::less;
      if (this->lb() > rhs) return std::weak_ordering::greater;
      return std::weak_ordering::equivalent;
  }

  /**
   * @brief Spaceship operator
   *
   * @param[in] lhs
   * @param[in] rhs
   * @return std::weak_ordering
   */
  friend constexpr auto operator<=>(const T& lhs, const Interval& rhs) ->
  std::weak_ordering {
      if (lhs < rhs.lb()) return std::weak_ordering::less;
      if (lhs > rhs.ub()) return std::weak_ordering::greater;
      return std::weak_ordering::equivalent;
  }

  // /**
  //  * @brief Less than other
  //  *
  //  * @tparam U
  //  * @param[in] rhs
  //  * @return true
  //  * @return false
  //  */
  // template <typename U> //
  // constexpr auto operator<(const U &rhs) const -> bool {
  //   return this->ub() < rhs;
  // }

  // /**
  //  * @brief Left less than
  //  *
  //  * @param[in] lhs
  //  * @param[in] rhs
  //  * @return true
  //  * @return false
  //  */
  // friend constexpr auto operator<(const T &lhs, const Interval &rhs) -> bool {
  //   return lhs < rhs.lb();
  // }

  // /**
  //  * @brief Greater than
  //  *
  //  * @tparam U
  //  * @param[in] rhs
  //  * @return true
  //  * @return false
  //  */
  // template <typename U> //
  // constexpr auto operator>(const U &rhs) const -> bool {
  //   return this->lb() > rhs;
  // }

  // /**
  //  * @brief
  //  *
  //  * @param lhs
  //  * @param rhs
  //  * @return true
  //  * @return false
  //  */
  // friend constexpr auto operator>(const T &lhs, const Interval &rhs) -> bool {
  //   return rhs < lhs;
  // }

  // /**
  //  * @brief Less than or euqual to
  //  *
  //  * @tparam U
  //  * @param[in] rhs
  //  * @return true
  //  * @return false
  //  */
  // template <typename U> //
  // constexpr auto operator<=(const U &rhs) const -> bool {
  //   return !(rhs < *this);
  // }

  // /**
  //  * @brief
  //  *
  //  * @param lhs
  //  * @param rhs
  //  * @return true
  //  * @return false
  //  */
  // friend constexpr auto operator<=(const T &lhs, const Interval &rhs) -> bool {
  //   return !(rhs < lhs);
  //   ;
  // }

  // /**
  //  * @brief Greater than or equal to
  //  *
  //  * @tparam U
  //  * @param[in] rhs
  //  * @return true
  //  * @return false
  //  */
  // template <typename U> //
  // constexpr auto operator>=(const U &rhs) const -> bool {
  //   return !(*this < rhs);
  // }

  // /**
  //  * @brief
  //  *
  //  * @param lhs
  //  * @param rhs
  //  * @return true
  //  * @return false
  //  */
  // friend constexpr auto operator>=(const T &lhs, const Interval &rhs) -> bool {
  //   return !(lhs < rhs);
  // }

  ///@}

  /** @name Arithmetic operators
   *  definie +, -, *, /, +=, -=, +=, /=, etc.
   */
  ///@{

  /**
   * @brief Negate
   *
   * @return interval
   */
  constexpr auto operator-() const -> Interval {
    return {-this->_ub, -this->_lb};
  }

  /**
   * @brief Add
   *
   * @param[in] alpha
   * @return Interval&
   */
  template <typename U>
  constexpr auto operator+=(const U &alpha) -> Interval & {
    this->_lb += alpha;
    this->_ub += alpha;
    return *this;
  }

  /**
   * @brief Add by a scalar
   *
   * @param[in] rhs
   * @param[in] alpha
   * @return interval
   */
  template <typename U>
  friend constexpr auto operator+(Interval rhs, const U &alpha) -> Interval {
    return rhs += alpha;
  }

  /**
   * @brief Add (by a scalar)
   *
   * @param[in] alpha
   * @param[in] rhs
   * @return interval
   */
  friend constexpr auto operator+(const T &alpha, Interval rhs) -> Interval {
    return rhs += alpha;
  }

  /**
   * @brief Substract
   *
   * @param[in] alpha
   * @return Interval&
   */
  template <typename U>
  constexpr auto operator-=(const U &alpha) -> Interval & {
    this->_lb -= alpha;
    this->_ub -= alpha;
    return *this;
  }

  /**
   * @brief Subtract by a scalar
   *
   * @param[in] rhs
   * @param[in] alpha
   * @return interval
   */
  template <typename U>
  friend constexpr auto operator-(const Interval &rhs, const U &alpha)
      -> Interval {
    auto lower = rhs.lb() - alpha;
    auto upper = rhs.ub() - alpha;
    return Interval<decltype(lower)>{std::move(lower), std::move(upper)};
  }

  /**
   * @brief Enlarge with
   *
   * @param[in] alpha
   * @return Interval&
   */
  constexpr auto enlarge_with(const T &alpha) -> Interval & {
    this->_lb -= alpha;
    this->_ub += alpha;
    return *this;
  }

  ///@}

  /**
   * @brief
   *
   * @tparam U
   * @param[in] a
   * @return true
   * @return false
   */
  template <typename U> // cppcheck-suppress internalAstError
  constexpr auto overlaps(const U &other) const -> bool {
    return !(*this < other || other < *this);
  }

  // template <typename U> //
  // constexpr auto contains(const U &other) const ->
  //     typename std::enable_if<std::is_scalar<U>::value, bool>::type {
  //   return this->lb() <= other && other <= this->ub();
  // }

  // template <typename U> //
  // constexpr auto contains(const U &other) const ->
  //     typename std::enable_if<!std::is_scalar<U>::value, bool>::type {
  //   return this->lb() <= other.lb() && other.ub() <= this->ub();
  // }

  /**
   * @brief contains
   *
   * @tparam U
   * @param[in] a
   * @return true
   * @return false
   */
  template <typename U> // cppcheck-suppress internalAstError
  constexpr auto contains(const U &other) const -> bool {
    if constexpr (requires { other.lb(); }) {
      return this->lb() <= other.lb() && other.ub() <= this->ub();
    } else /* constexpr */ { // assume scalar
      return this->lb() <= other && other <= this->ub();
    }
  }

  // template <typename U> //
  // constexpr auto intersection_with(const U &other) const ->
  //     typename std::enable_if<std::is_scalar<U>::value, Interval<U>>::type {
  //   return Interval<U>{other, other};
  // }

  // template <typename U> //
  // constexpr auto intersection_with(const U &other) const ->
  //     typename std::enable_if<!std::is_scalar<U>::value, Interval<T>>::type {
  //   return Interval<T>{this->lb() > other.lb() ? this->lb() : T(other.lb()),
  //                      this->ub() < other.ub() ? this->ub() : T(other.ub())};
  // }

  /**
   * @brief minimum distance with
   *
   * @tparam U
   * @param[in] other
   * @return constexpr auto
   */
  template <typename U> //
  constexpr auto intersection_with(const U &other) const {
    if constexpr (requires { other.lb(); }) {
      return Interval<T>{this->lb() > other.lb() ? this->lb() : T(other.lb()),
                         this->ub() < other.ub() ? this->ub() : T(other.ub())};
    } else /* constexpr */ { // assume scalar
      return Interval<U>{other, other};
    }
  }

  /**
   * @brief minimum distance with
   *
   * @tparam U
   * @param[in] other
   * @return constexpr auto
   */
  template <typename U>
  constexpr auto min_dist_with(const U &other) const -> T {
    if (*this < other) {
      return min_dist(this->_ub, other);
    }
    if (other < *this) {
      return min_dist(this->_lb, other);
    }
    return T(0);
  }


  // /**
  //  * @brief minimum distance with
  //  *
  //  * @tparam U
  //  * @param[in] other
  //  * @return constexpr auto
  //  */
  // template <typename U> constexpr auto min_dist_change_with(U &other) {
  //   if (*this < other) {
  //     this->_lb = this->_ub;
  //     return min_dist_change(this->_ub, other);
  //   }
  //   if (other < *this) {
  //     this->_ub = this->_lb;
  //     return min_dist_change(this->_lb, other);
  //   }
  //   if constexpr (std::is_scalar<U>::value) {
  //     this->_ub = this->_lb = other;
  //   } else {
  //     *this = other = this->intersection_with(other);
  //   }
  //   return T(0);
  // }

  /**
   * @brief minimum distance with
   *
   * @tparam U
   * @param[in] other
   * @return constexpr auto
   */
  template <typename U>
  constexpr auto min_dist_change_with(U &other) const -> T {
    if (*this < other) {
      this->_lb = this->_ub;
      return min_dist_change(this->_ub, other);
    }
    if (other < *this) {
      this->_ub = this->_lb;
      return min_dist_change(this->_lb, other);
    }

    if constexpr (requires { other.lb(); }) {
      *this = other = this->intersection_with(other);
    } else /* constexpr */ { // assume scalar
      this->_ub = this->_lb = other;
    }
    return T(0);
  }

  /**
   * @brief
   *
   * @tparam Stream
   * @param[out] out
   * @param[in] I
   * @return Stream&
   */
  template <class Stream>
  friend auto operator<<(Stream &out, const Interval &intvl) -> Stream & {
    out << "[" << intvl.lb() << ", " << intvl.ub() << "]";
    return out;
  }
};
#pragma pack(pop)

// template <typename U1, typename U2> //
// inline constexpr auto enlarge(U1 lhs, const U2 &rhs) ->
//     typename std::enable_if<std::is_scalar<U1>::value, Interval<U1>>::type {
//   return Interval<U1>{lhs - rhs, lhs + rhs};
// }

// template <typename U1, typename U2> //
// inline constexpr auto enlarge(U1 lhs, const U2 &rhs) ->
//     typename std::enable_if<!std::is_scalar<U1>::value, U1>::type {
//   lhs.enlarge_with(rhs);
//   return lhs;
// }

template <typename U1, typename U2> //
inline constexpr auto enlarge(const U1& lhs, const U2 &rhs) {
  if constexpr (requires { lhs.enlarge_with(rhs); }) {
    auto res{lhs};
    res.enlarge_with(rhs);
    return res;
  } else /* constexpr */ {
    return Interval<U1>{lhs - rhs, lhs + rhs};
  }
}

// template <typename U1, typename U2>  //
// inline constexpr auto min_dist_change_merge(U1& lhs, U2& rhs) {
//     if constexpr (std::is_scalar<U1>::value) {
//         if constexpr (std::is_scalar<U2>::value) {
//             return std::abs(lhs - rhs);
//         } else /* constexpr */ {
//             return rhs.min_dist_change_merge_with(lhs);
//         }
//     } else /* constexpr */ {
//         return lhs.min_dist_change_with(rhs);
//     }
// }

} // namespace recti
