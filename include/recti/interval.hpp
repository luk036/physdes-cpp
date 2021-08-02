#pragma once

#include <algorithm>  // import std::min, std::max
#include <cassert>
#include <utility>  // import std::move

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief Interval
     *
     * @tparam T
     */
    template <typename T = int> class interval {
      private:
        T _lower;  //> lower bound
        T _upper;  //> upper bound

      public:
        /**
         * @brief Construct a new interval object
         *
         * @param[in] lower
         * @param[in] upper
         */
        constexpr interval(T&& lower, T&& upper) noexcept
            : _lower{std::move(lower)}, _upper{std::move(upper)} {
            assert(!(_upper < _lower));
        }

        /**
         * @brief Construct a new interval object
         *
         * @param[in] lower
         * @param[in] upper
         */
        constexpr interval(const T& lower, const T& upper) : _lower{lower}, _upper{upper} {
            assert(!(_upper < _lower));
        }

        /**
         * @brief
         *
         * @return const T&
         */
        [[nodiscard]] constexpr auto lower() const -> const T& { return this->_lower; }

        /**
         * @brief
         *
         * @return const T&
         */
        [[nodiscard]] constexpr auto upper() const -> const T& { return this->_upper; }

        /**
         * @brief
         *
         * @return constexpr T
         */
        [[nodiscard]] constexpr auto len() const -> T { return this->upper() - this->lower(); }

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
        template <typename U>  //
        constexpr auto operator==(const interval<U>& rhs) const -> bool {
            return this->lower() == rhs.lower() && this->upper() == rhs.upper();
        }

        /**
         * @brief Not equal to
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U>  //
        constexpr auto operator!=(const interval<U>& rhs) const -> bool {
            return !(*this == rhs);
        }

        // /**
        //  * @brief Spaceship operator
        //  *
        //  * @tparam U
        //  * @param[in] rhs
        //  * @return std::weak_ordering
        //  */
        // template <typename U>  //
        // constexpr auto operator<=>(const U& rhs) const -> std::weak_ordering {
        //     if (this->upper() < rhs) return std::weak_ordering::less;
        //     if (this->lower() > rhs) return std::weak_ordering::greater;
        //     return std::weak_ordering::equivalent;
        // }

        // /**
        //  * @brief Spaceship operator
        //  *
        //  * @param[in] lhs
        //  * @param[in] rhs
        //  * @return std::weak_ordering
        //  */
        // friend constexpr auto operator<=>(const T& lhs, const interval& rhs) ->
        // std::weak_ordering {
        //     if (lhs < rhs.lower()) return std::weak_ordering::less;
        //     if (lhs > rhs.upper()) return std::weak_ordering::greater;
        //     return std::weak_ordering::equivalent;
        // }

        /**
         * @brief Less than other
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U>  //
        constexpr auto operator<(const U& rhs) const -> bool {
            return this->upper() < rhs;
        }

        /**
         * @brief Spaceship operator
         *
         * @param[in] lhs
         * @param[in] rhs
         * @return true
         * @return false
         */
        friend constexpr auto operator<(const T& lhs, const interval& rhs) -> bool {
            return lhs < rhs.lower();
        }

        /**
         * @brief Greater than
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U>  //
        constexpr auto operator>(const U& rhs) const -> bool {
            return rhs < *this;
        }

        /**
         * @brief
         *
         * @param lhs
         * @param rhs
         * @return true
         * @return false
         */
        friend constexpr auto operator>(const T& lhs, const interval& rhs) -> bool {
            return rhs < lhs;
        }

        /**
         * @brief Less than or euqual to
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U>  //
        constexpr auto operator<=(const U& rhs) const -> bool {
            return !(rhs < *this);
        }

        /**
         * @brief
         *
         * @param lhs
         * @param rhs
         * @return true
         * @return false
         */
        friend constexpr auto operator<=(const T& lhs, const interval& rhs) -> bool {
            return !(rhs < lhs);
            ;
        }

        /**
         * @brief Greater than or equal to
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U>  //
        constexpr auto operator>=(const U& rhs) const -> bool {
            return !(*this < rhs);
        }

        /**
         * @brief
         *
         * @param lhs
         * @param rhs
         * @return true
         * @return false
         */
        friend constexpr auto operator>=(const T& lhs, const interval& rhs) -> bool {
            return !(lhs < rhs);
        }

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
        constexpr auto operator-() const -> interval {
            return {-this->_upper, -this->_lower};
        }

        /**
         * @brief Add
         *
         * @param[in] alpha
         * @return interval&
         */
        constexpr auto operator+=(const T& alpha) -> interval& {
            this->_lower += alpha;
            this->_upper += alpha;
            return *this;
        }

        /**
         * @brief Add by a scalar
         *
         * @param[in] x
         * @param[in] alpha
         * @return interval
         */
        friend constexpr auto operator+(interval x, const T& alpha) -> interval {
            return x += alpha;
        }

        /**
         * @brief Add (by a scalar)
         *
         * @param[in] alpha
         * @param[in] x
         * @return interval
         */
        friend constexpr auto operator+(const T& alpha, interval x) -> interval {
            return x += alpha;
        }

        /**
         * @brief Substract
         *
         * @param[in] alpha
         * @return interval&
         */
        constexpr auto operator-=(const T& alpha) -> interval& {
            this->_lower -= alpha;
            this->_upper -= alpha;
            return *this;
        }

        /**
         * @brief Subtract by a scalar
         *
         * @param[in] x
         * @param[in] alpha
         * @return interval
         */
        friend constexpr auto operator-(interval x, const T& alpha) -> interval {
            return x -= alpha;
        }

        /**
         * @brief Enlarge with
         *
         * @param[in] alpha
         * @return interval&
         */
        constexpr auto enlarge_with(const T& alpha) -> interval& {
            this->_lower -= alpha;
            this->_upper += alpha;
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
        template <typename U>  //
        [[nodiscard]] constexpr auto overlaps(const U& a) const -> bool {
            return !(*this < a || a < *this);
        }

        /**
         * @brief contains
         *
         * @tparam U
         * @param[in] a
         * @return true
         * @return false
         */
        template <typename U>  //
        [[nodiscard]] constexpr auto contains(const U& a) const -> bool {
            if constexpr (std::is_scalar_v<U>) {
                return this->lower() <= a && a <= this->upper();
            } else {
                return this->lower() <= a.lower() && a.upper() <= this->upper();
            }
        }

        /**
         * @brief minimum distance with
         *
         * @tparam U
         * @param[in] other
         * @return constexpr auto
         */
        template <typename U>  //
        constexpr auto intersection_with(const U& other) const -> interval {
            if constexpr (std::is_scalar_v<U>) {
                return {other, other};
            } else {
                return {std::max(this->_lower, other._lower), std::min(this->_upper, other._upper)};
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
        [[nodiscard]] constexpr auto min_dist_with(const U& other) const {
            if constexpr (std::is_scalar_v<U>) {
                if (*this < other) {
                    return other - this->_upper;
                }
                if (other < *this) {
                    return this->_lower - other;
                }
            } else {
                if (*this < other) {
                    return other._lower - this->_upper;
                }
                if (other < *this) {
                    return this->_lower - other._upper;
                }
            }
            return 0;
        }

        /**
         * @brief minimum distance with
         *
         * @tparam U
         * @param[in] other
         * @return constexpr auto
         */
        template <typename U>
        [[nodiscard]] constexpr auto min_dist_change_with(U& other) {
            if constexpr (std::is_scalar_v<U>) {
                if (*this < other) {
                    this->_lower = this->_upper;
                    return other - this->_upper;
                }
                if (other < *this) {
                    this->_upper = this->_lower;
                    return this->_lower - other;
                }
                this->_upper = this->_lower = other;
            } else {
                if (*this < other) {
                    this->_lower = this->_upper;
                    return other._lower - this->_upper;
                }
                if (other < *this) {
                    this->_upper = this->_lower;
                    return this->_lower - other._upper;
                }
                *this = other = this->intersection_with(other);
            }
            return 0;
        }

        /**
         * @brief
         *
         * @tparam Stream
         * @param[out] out
         * @param[in] I
         * @return Stream&
         */
        template <class Stream> friend auto operator<<(Stream& out, const interval& I)
            -> Stream& {
            out << "[" << I.lower() << ", " << I.upper() << "]";
            return out;
        }

    };
#pragma pack(pop)

    template <typename U1, typename U2>  //
    inline constexpr auto overlap(const U1& lhs, const U2& rhs) -> bool {
        if constexpr (std::is_scalar_v<U1>) {
            if constexpr (std::is_scalar_v<U2>) {
                if (lhs != rhs) return false;
            } else {
                if (!rhs.overlaps(lhs)) return false;
            }
        } else {
            if (!lhs.overlaps(rhs)) return false;
        }
        return true;
    }

    template <typename U1, typename U2>  //
    inline constexpr auto contain(const U1& lhs, const U2& rhs) -> bool {
        if constexpr (std::is_scalar_v<U1>) {
            if constexpr (std::is_scalar_v<U2>) {
                return lhs == rhs;
            } else {
                return false;
            }
        } else {
            return lhs.contains(rhs);
        }
    }

    template <typename U1, typename U2>  //
    inline constexpr auto intersection(const U1& lhs, const U2& rhs) {
        if constexpr (std::is_scalar_v<U1>) {
            if constexpr (std::is_scalar_v<U2>) {
                assert(lhs == rhs);
                return lhs;
            } else {
                return rhs.intersection(lhs);
            }
        } else {
            return lhs.intersection(rhs);
        }
    }

    template <typename U1, typename U2>  //
    inline constexpr auto min_dist(const U1& lhs, const U2& rhs) {
        if constexpr (std::is_scalar_v<U1>) {
            if constexpr (std::is_scalar_v<U2>) {
                return std::abs(lhs - rhs);
            } else {
                return rhs.min_dist_with(lhs);
            }
        } else {
            return lhs.min_dist_with(rhs);
        }
    }

    template <typename U1, typename U2>  //
    inline constexpr auto enlarge(U1 lhs, const U2& rhs) {
        if constexpr (std::is_scalar_v<U1>) {
            return interval<U1>{lhs - rhs, lhs + rhs};
        } else {
            lhs.enlarge_with(rhs); 
            return lhs;
        }
    }

    template <typename U1, typename U2>  //
    inline constexpr auto min_dist_change(U1& lhs, U2& rhs) {
        if constexpr (std::is_scalar_v<U1>) {
            if constexpr (std::is_scalar_v<U2>) {
                return std::abs(lhs - rhs);
            } else {
                return rhs.min_dist_change_with(lhs);
            }
        } else {
            return lhs.min_dist_change_with(rhs);
        }
    }

    // template <typename U1, typename U2>  //
    // inline constexpr auto min_dist_change_merge(U1& lhs, U2& rhs) {
    //     if constexpr (std::is_scalar_v<U1>) {
    //         if constexpr (std::is_scalar_v<U2>) {
    //             return std::abs(lhs - rhs);
    //         } else {
    //             return rhs.min_dist_change_merge_with(lhs);
    //         }
    //     } else {
    //         return lhs.min_dist_change_with(rhs);
    //     }
    // }

}  // namespace recti
