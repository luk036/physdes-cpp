#pragma once

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

        /**
         * @brief Less than
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
         * @brief
         *
         * @param lhs
         * @param rhs
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
         *  definie +, -, *, /, +=, -=, *=, /=, etc.
         */
        ///@{

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
            return !(a < *this || *this < a);
        }

        /**
         * @brief
         *
         * @tparam U
         * @param[in] a
         * @return true
         * @return false
         */
        template <typename U>  //
        [[nodiscard]] constexpr auto contains(const interval<U>& a) const -> bool {
            return this->lower() <= a.lower() && a.upper() <= this->upper();
        }

        /**
         * @brief contains
         *
         * @param[in] x
         * @return true
         * @return false
         */
        [[nodiscard]] constexpr auto contains(const T& a) const -> bool {
            return this->lower() <= a && a <= this->upper();
        }
    };
#pragma pack(pop)

    /**
     * @brief
     *
     * @tparam Stream
     * @tparam T
     * @param[out] out
     * @param[in] p
     * @return Stream&
     */
    template <typename T, class Stream> auto operator<<(Stream& out, const interval<T>& Intv)
        -> Stream&;
}  // namespace recti
