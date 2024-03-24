#pragma once

// #include <algorithm> // import std::min, std::max
#include <cassert>
#include <cmath>  // for abs

namespace recti {

    /**
     * @brief overlap
     *
     * The above code defines a template function called `overlap` that takes two
     * arguments `lhs` and `rhs`. The function checks if the two arguments overlap
     * with each other.
     *
     *     .---------------------.
     *     |                     |
     *     |            .--------+-------.
     *     |            |        |       |
     *     `------------+--------'       |
     *                  |                |
     *                  `----------------'
     *
     * @tparam U1
     * @tparam U2
     * @param[in] lhs
     * @param[in] rhs
     * @return true
     * @return false
     */
    template <typename U1, typename U2>  //
    constexpr auto overlap(const U1 &lhs, const U2 &rhs) -> bool {
        if constexpr (requires { lhs.overlaps(rhs); }) {
            return lhs.overlaps(rhs);
        } else if constexpr (requires { rhs.overlaps(lhs); }) {
            return rhs.overlaps(lhs);
        } else /* constexpr */ {
            return lhs == rhs;
        }
    }

    /**
     * @brief contain
     *
     * The above code is defining a template function called `contain` that checks if one object
     * contains another object.
     *
     *
     *     .---------------------.
     *     |                     |
     *     | .  | ----  ▯        |
     *     |                     |
     *     `---------------------'
     *
     * @tparam U1
     * @tparam U2
     * @param[in] lhs
     * @param[in] rhs
     * @return true
     * @return false
     */
    template <typename U1, typename U2>  //
    constexpr auto contain(const U1 &lhs, const U2 &rhs) -> bool {
        if constexpr (requires { lhs.contains(rhs); }) {
            return lhs.contains(rhs);
        } else if constexpr (requires { rhs.contains(lhs); }) {
            return false;
        } else /* constexpr */ {
            return lhs == rhs;
        }
    }

    /* The above code is defining a template function called `intersection` that takes two
    parameters `lhs` and `rhs`. The function returns the intersection of `lhs` and `rhs`. */
    /**
     * @brief intersection
     *
     *     .---------------------.
     *     |                     |
     *     |            .--------+-------.
     *     |            |        |       |
     *     `------------+--------'       |
     *                  |                |
     *                  `----------------'
     *
     * @tparam U1
     * @tparam U2
     * @param[in] lhs
     * @param[in] rhs
     * @return constexpr auto
     */
    template <typename U1, typename U2>  //
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

    /* The above code is defining a template function called `min_dist` that calculates the minimum
    distance between two objects `lhs` and `rhs`. */
    /**
     * @brief min_dist
     *
     *     .-----------.
     *     |           |
     *     |           |~~~~~.-------------.
     *     |           |     |             |
     *     `-----------'     |             |
     *                       |             |
     *                       `-------------'
     * @tparam U1
     * @tparam U2
     * @param[in] lhs
     * @param[in] rhs
     * @return constexpr auto
     */
    template <typename U1, typename U2>  //
    constexpr auto min_dist(const U1 &lhs, const U2 &rhs) {
        if constexpr (requires { lhs.min_dist_with(rhs); }) {
            return lhs.min_dist_with(rhs);
        } else if constexpr (requires { rhs.min_dist_with(lhs); }) {
            return rhs.min_dist_with(lhs);
        } else /* constexpr */ {
            return std::abs(lhs - rhs);
        }
    }

    /**
     * @brief min_dist with change
     *
     * @tparam U1
     * @tparam U2
     * @param[in] lhs
     * @param[in] rhs
     * @return constexpr auto
     */
    template <typename U1, typename U2>  //
    constexpr auto min_dist_change(U1 &lhs, U2 &rhs) {
        if constexpr (requires { lhs.min_dist_change_with(rhs); }) {
            return lhs.min_dist_change_with(rhs);
        } else if constexpr (requires { rhs.min_dist_change_with(lhs); }) {
            return rhs.min_dist_change_with(lhs);
        } else /* constexpr */ {
            return std::abs(lhs - rhs);
        }
    }

}  // namespace recti
