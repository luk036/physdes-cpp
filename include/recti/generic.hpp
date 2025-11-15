#pragma once

// #include <algorithm> // import std::min, std::max
#include <cassert>
#include <cmath>  // for abs

namespace recti {
    /**
     * @brief Checks if two objects overlap.
     *
     * This function checks if the two input objects `lhs` and `rhs` overlap with each other. It
     * first checks if the `lhs` object has an `overlaps` member function that can be called with
     * `rhs` as an argument. If so, it calls that function and returns the result. If not, it checks
     * if the `rhs` object has an `overlaps` member function that can be called with `lhs` as an
     * argument, and returns the result of that call. If neither object has an `overlaps` member
     * function, the function simply checks if the two objects are equal and returns the result.
     *
     * @verbatim
     *     .---------------------.
     *     |                     |
     *     |            .--------+-------.
     *     |            |        |       |
     *     `------------+--------'       |
     *                  |                |
     *                  `----------------'
     * @endverbatim
     *
     * @tparam U1 The type of the first object.
     * @tparam U2 The type of the second object.
     * @param lhs The first object to check for overlap.
     * @param rhs The second object to check for overlap.
     * @return `true` if the two objects overlap, `false` otherwise.
     */
    template <typename U1, typename U2>  //
    constexpr auto overlap(const U1& lhs, const U2& rhs) -> bool {
        if constexpr (requires { lhs.overlaps(rhs); }) {
            return lhs.overlaps(rhs);
        } else if constexpr (requires { rhs.overlaps(lhs); }) {
            return rhs.overlaps(lhs);
        } else /* constexpr */ {
            return lhs == rhs;
        }
    }

    /**
     * @brief Check if one object contains another object.
     *
     * This function checks if the first object `lhs` contains the second object `rhs`. It first
     * checks if `lhs` has a `contains` member function that can be called with `rhs` as an
     * argument, and returns the result of that call. If `lhs` does not have a `contains` member
     * function, it checks if `rhs` has a `contains` member function that can be called with `lhs`
     * as an argument, and returns `false` if that is the case (since `rhs` contains `lhs` implies
     * that `lhs` does not contain `rhs`). If neither object has a `contains` member function, the
     * function simply checks if the two objects are equal and returns the result.
     *
     * @verbatim
     *     .---------------------.
     *     |            .--.     |
     *     | .  | ----  |  |     |
     *     |            `--'     |
     *     `---------------------'
     * @endverbatim
     *
     * @tparam U1 The type of the first object.
     * @tparam U2 The type of the second object.
     * @param lhs The first object to check for containment.
     * @param rhs The second object to check for containment.
     * @return `true` if the first object contains the second object, `false` otherwise.
     */
    template <typename U1, typename U2>  //
    constexpr auto contain(const U1& lhs, const U2& rhs) -> bool {
        if constexpr (requires { lhs.contains(rhs); }) {
            return lhs.contains(rhs);
        } else if constexpr (requires { rhs.contains(lhs); }) {
            return false;
        } else /* constexpr */ {
            return lhs == rhs;
        }
    }

    /**
     * @brief Computes the intersection of two objects.
     *
     * This function computes the intersection of the two input objects `lhs` and `rhs`. It first
     * checks if `lhs` has an `intersect_with` member function that can be called with `rhs` as an
     * argument, and returns the result of that call. If `lhs` does not have an `intersect_with`
     * member function, it checks if `rhs` has an `intersect_with` member function that can be
     * called with `lhs` as an argument, and returns the result of that call. If neither object has
     * an `intersect_with` member function, the function asserts that `lhs` and `rhs` are equal and
     * returns `lhs`.
     *
     * @verbatim
     *     .---------------------.
     *     |                     |
     *     |            .--------+-------.
     *     |            |        |       |
     *     `------------+--------'       |
     *                  |                |
     *                  `----------------'
     * @endverbatim
     *
     * @tparam U1 The type of the first object.
     * @tparam U2 The type of the second object.
     * @param[in] lhs The first object.
     * @param[in] rhs The second object.
     * @return The intersection of `lhs` and `rhs`.
     */
    template <typename U1, typename U2>  //
    constexpr auto intersection(const U1& lhs, const U2& rhs) {
        if constexpr (requires { lhs.intersect_with(rhs); }) {
            return lhs.intersect_with(rhs);
        } else if constexpr (requires { rhs.intersect_with(lhs); }) {
            return rhs.intersect_with(lhs);
        } else /* constexpr */ {
            assert(lhs == rhs);
            return lhs;
        }
    }

    /**
     * @brief Calculates the minimum distance between two objects `lhs` and `rhs`.
     *
     * @verbatim
     *     .-----------.
     *     |           |
     *     |           |~~~~~.-------------.
     *     |           |     |             |
     *     `-----------'     |             |
     *                       |             |
     *                       `-------------'
     * @endverbatim
     *
     * The function first checks if `lhs` has a `min_dist_with` member function that can be
     * called with `rhs` as an argument, and returns the result of that call. If `lhs` does not
     * have a `min_dist_with` member function, it checks if `rhs` has a `min_dist_with` member
     * function that can be called with `lhs` as an argument, and returns the result of that
     * call. If neither object has a `min_dist_with` member function, the function returns the
     * absolute difference between `lhs` and `rhs`.
     *
     * @tparam U1 The type of the first object.
     * @tparam U2 The type of the second object.
     * @param[in] lhs The first object.
     * @param[in] rhs The second object.
     * @return The minimum distance between `lhs` and `rhs`.
     */
    template <typename U1, typename U2>  //
    constexpr auto min_dist(const U1& lhs, const U2& rhs) {
        if constexpr (requires { lhs.min_dist_with(rhs); }) {
            return lhs.min_dist_with(rhs);
        } else if constexpr (requires { rhs.min_dist_with(lhs); }) {
            return rhs.min_dist_with(lhs);
        } else /* constexpr */ {
            return std::abs(lhs - rhs);
        }
    }

    /**
     * @brief Calculates the minimum distance between two objects `lhs` and `rhs`, with the ability
     * to handle a change in the objects.
     *
     * The function first checks if `lhs` has a `min_dist_change_with` member function that can be
     * called with `rhs` as an argument, and returns the result of that call. If `lhs` does not have
     * a `min_dist_change_with` member function, it checks if `rhs` has a `min_dist_change_with`
     * member function that can be called with `lhs` as an argument, and returns the result of that
     * call. If neither object has a `min_dist_change_with` member function, the function returns
     * the absolute difference between `lhs` and `rhs`.
     *
     * @tparam U1 The type of the first object.
     * @tparam U2 The type of the second object.
     * @param[in] lhs The first object.
     * @param[in] rhs The second object.
     * @return The minimum distance between `lhs` and `rhs`, with the ability to handle a change in
     * the objects.
     */
    template <typename U1, typename U2>  //
    constexpr auto min_dist_change(U1& lhs, U2& rhs) {
        if constexpr (requires { lhs.min_dist_change_with(rhs); }) {
            return lhs.min_dist_change_with(rhs);
        } else if constexpr (requires { rhs.min_dist_change_with(lhs); }) {
            return rhs.min_dist_change_with(lhs);
        } else /* constexpr */ {
            return std::abs(lhs - rhs);
        }
    }

    /**
     * @brief Returns the nearest point on `lhs` to `rhs`.
     *
     * @verbatim
     *     .---------------------.
     *     |                     *~~~~~~~~~~~o rhs
     *     |                     |
     *     |                     |
     *     `---------------------'
     *         lhs
     * @endverbatim
     *
     * This function returns the point on `lhs` that is nearest to `rhs`. If `lhs` has a
     * `nearest_to` member function, it is used. Otherwise, it assumes `lhs` is a scalar and
     * returns it directly.
     *
     * @tparam U1 Type of the first object.
     * @tparam U2 Type of the second object.
     * @param[in] lhs The object to find the nearest point on.
     * @param[in] rhs The reference point.
     * @return The nearest point on `lhs` to `rhs`.
     */
    template <typename U1, typename U2>
    constexpr auto nearest(const U1& lhs, const U2& rhs) {
        if constexpr (requires { lhs.nearest_to(rhs); }) {
            return lhs.nearest_to(rhs);
        } else {
            return lhs;
        }
    }

    /**
     * @brief Calculates the measure (length, area, etc.) of an object.
     *
     * This function returns the measure of `obj`. If `obj` has a `measure` member function,
     * it is used. Otherwise, it returns 1.
     *
     * @tparam U Type of the object.
     * @param[in] obj The object to measure.
     * @return The measure of the object.
     */
    template <typename U>
    constexpr auto measure_of(const U& obj) {
        if constexpr (requires { obj.measure(); }) {
            return obj.measure();
        } else {
            return 1;
        }
    }

    /**
     * @brief Calculates the center of an object.
     *
     * This function returns the center of `obj`. If `obj` has a `get_center` member function,
     * it is used. Otherwise, it assumes `obj` is a scalar and returns it directly.
     *
     * @tparam U Type of the object.
     * @param[in] obj The object to find the center of.
     * @return The center of the object.
     */
    template <typename U>
    constexpr auto center(const U& obj) {
        if constexpr (requires { obj.get_center(); }) {
            return obj.get_center();
        } else {
            return obj;
        }
    }

    /**
     * @brief Calculates the lower corner of an object.
     *
     * This function returns the lower corner of `obj`. If `obj` has a `lower_corner` member
     * function, it is used. Otherwise, it assumes `obj` is a scalar and returns it directly.
     *
     * @tparam U Type of the object.
     * @param[in] obj The object to find the lower corner of.
     * @return The lower corner of the object.
     */
    template <typename U>
    constexpr auto lower(const U& obj) {
        if constexpr (requires { obj.lower_corner(); }) {
            return obj.lower_corner();
        } else {
            return obj;
        }
    }

    /**
     * @brief Calculates the upper corner of an object.
     *
     * This function returns the upper corner of `obj`. If `obj` has a `upper_corner` member
     * function, it is used. Otherwise, it assumes `obj` is a scalar and returns it directly.
     *
     * @tparam U Type of the object.
     * @param[in] obj The object to find the upper corner of.
     * @return The upper corner of the object.
     */
    template <typename U>
    constexpr auto upper(const U& obj) {
        if constexpr (requires { obj.upper_corner(); }) {
            return obj.upper_corner();
        } else {
            return obj;
        }
    }
}  // namespace recti
