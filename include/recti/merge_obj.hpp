#pragma once

#include <algorithm>  // import std::min, std::max
#include <cassert>
#include <utility>  // import std::move

#include "interval.hpp"
#include "point.hpp"

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief merge_obj
     *
     * object includes a single point, segment, or a region
     *
     * @tparam T
     */
    template <typename T1, typename T2> class merge_obj : public point<T1, T2> {
      public:
        /**
         * @brief Construct a new point object
         *
         * @param[in] x
         * @param[in] y
         */
        constexpr merge_obj(T1&& x, T2&& y) noexcept : point<T1, T2>{std::move(x), std::move(y)} {}

        // /**
        //  * @brief Construct a new point object
        //  *
        //  * @param[in] x
        //  * @param[in] y
        //  */
        // constexpr merge_obj(const T1& x, const T2& y) : point<T1, T2>{x + y, x - y} {}

        /**
         * @brief Add a vector (translation)
         *
         * @tparam U
         * @param[in] rhs
         * @return merge_obj&
         */
        template <typename U> constexpr auto operator+=(const vector2<U>& rhs) -> merge_obj& {
            this->_x += rhs.x() + rhs.y();
            this->_y += rhs.x() - rhs.y();
            return *this;
        }

        /**
         * @brief Substract a vector (translation)
         *
         * @tparam U
         * @param[in] rhs
         * @return merge_obj&
         */
        template <typename U> constexpr auto operator-=(const vector2<U>& rhs) -> merge_obj& {
            this->_x -= rhs.x() + rhs.y();
            this->_y -= rhs.x() - rhs.y();
            return *this;
        }

        /**
         * @brief Add
         *
         * @tparam U
         * @param[in] x
         * @param[in] y
         * @return vector2<T>
         */
        template <typename U>  //
        friend constexpr auto operator+(merge_obj x, const vector2<U>& y) -> merge_obj {
            return x += y;
        }

        /**
         * @brief Substract
         *
         * @tparam U
         * @param[in] x
         * @param[in] y
         * @return vector2<T>
         */
        template <typename U>  //
        friend constexpr auto operator-(merge_obj x, const vector2<U>& y) -> merge_obj {
            return x -= y;
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto min_dist_with(const merge_obj<U1, U2>& other) const {
            return std::max(min_dist(this->_x, other._x), min_dist(this->_y, other._y));
        }

        template <typename R>  //
        friend constexpr auto enlarge(const merge_obj& lhs, const R& alpha) {
            auto x = enlarge(lhs.x(), alpha);
            auto y = enlarge(lhs.y(), alpha);
            return merge_obj<decltype(x), decltype(y)>{std::move(x), std::move(y)};
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto merge_with(const merge_obj<U1, U2>& other) const {
            auto alpha = this->min_dist_with(other);
            auto half = alpha / 2;
            auto trr1 = enlarge(*this, half);
            auto trr2 = enlarge(other, alpha - half);
            return intersection(trr1, trr2);
        }

        // /**
        //  * @brief minimum distance with
        //  *
        //  * @param[in] other
        //  * @return constexpr auto
        //  */
        // [[nodiscard]] constexpr auto min_dist_change_with(merge_obj& other) {
        //     auto minDist = this->min_dist_with(other);
        //     auto mobj1 = this->enlarge_with(minDist);
        //     auto mobj2 = other.enlarge_with(minDist);
        //     other = mobj1.intersection_with(other);
        //     *this = mobj2.intersection_with(*this);
        //     return minDist;
        // }
    };
#pragma pack(pop)

    // template <typename U1, typename U2>  //
    // inline constexpr auto min_dist_change_merge(U1& lhs, U2& rhs) {
    //     if constexpr (std::is_scalar_v<U1>) {
    //         if constexpr (std::is_scalar_v<U2>) {
    //             return std::abs(lhs - rhs);
    //         } else {
    //             return rhs.min_dist_change_with(lhs);
    //         }
    //     } else {
    //         return lhs.min_dist_change_with(rhs);
    //     }
    // }

}  // namespace recti
