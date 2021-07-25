#pragma once

#include <tuple>    // import std::tie()
#include <utility>  // import std::move

namespace recti {

    /**
     * @brief vector2
     *
     */
    template <typename T = int> class vector2 {
      private:
        T _x;
        T _y;

        auto _tie() const { return std::tie(_x, _y); }

      public:
        /**
         * @brief
         *
         */
        constexpr vector2(T&& x, T&& y) noexcept : _x{std::move(x)}, _y{std::move(y)} {}

        /**
         * @brief
         *
         */
        constexpr vector2(const T& x, const T& y) : _x{x}, _y{y} {}

        /**
         * @brief
         *
         * @return constexpr const T&
         */
        [[nodiscard]] constexpr auto x() const noexcept -> const T& { return this->_x; }

        /**
         * @brief
         *
         * @return constexpr const T&
         */
        [[nodiscard]] constexpr auto y() const noexcept -> const T& { return this->_y; }

        /**
         * @brief
         *
         * @param[in] rhs
         * @return T
         */
        template <typename U>  //
        [[nodiscard]] constexpr auto cross(const vector2<U>& rhs) const {
            return this->_x * rhs._y - rhs._x * this->_y;
        }

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
        constexpr auto operator==(const vector2<U>& rhs) const -> bool {
            return this->_tie() == rhs._tie();
        }

        /**
         * @brief Less than
         *
         * @tparam U
         * @param rhs
         * @return true
         * @return false
         */
        template <typename U>  //
        constexpr auto operator<(const vector2<U>& rhs) const -> bool {
            return this->_tie() < rhs._tie();
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
        constexpr auto operator!=(const vector2<U>& rhs) const -> bool {
            return !(*this == rhs);
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
        constexpr auto operator>(const vector2<U>& rhs) const -> bool {
            return rhs < *this;
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
        constexpr auto operator<=(const vector2<U>& rhs) const -> bool {
            return !(rhs < *this);
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
        constexpr auto operator>=(const vector2<U>& rhs) const -> bool {
            return !(*this < rhs);
        }

        ///@}

        /** @name Arithmetic operators
         *  definie +, -, *, /, +=, -=, *=, /=, etc.
         */
        ///@{

        /**
         * @brief Negate
         *
         * @return vector2<T>
         */
        constexpr auto operator-() const -> vector2<T> { return vector2<T>(-this->_x, -this->_y); }

        /**
         * @brief Add
         *
         * @tparam U
         * @param[in] rhs
         * @return vector2<T>&
         */
        template <typename U> constexpr auto operator+=(const vector2<U>& rhs) -> vector2<T>& {
            this->_x += rhs.x();
            this->_y += rhs.y();
            return *this;
        }

        /**
         * @brief Substract
         *
         * @tparam U
         * @param[in] rhs
         * @return vector2<T>&
         */
        template <typename U>  //
        constexpr auto operator-=(const vector2<U>& rhs) -> vector2<T>& {
            this->_x -= rhs.x();
            this->_y -= rhs.y();
            return *this;
        }

        /**
         * @brief Multiply
         *
         * @param[in] alpha
         * @return vector2<T>&
         */
        constexpr auto operator*=(const T& alpha) -> vector2<T>& {
            this->_x *= alpha;
            this->_y *= alpha;
            return *this;
        }

        /**
         * @brief Divide
         *
         * @param[in] alpha
         * @return vector2<T>&
         */
        constexpr auto operator/=(const T& alpha) -> vector2<T>& {
            this->_x /= alpha;
            this->_y /= alpha;
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
        friend constexpr auto operator+(vector2<T> x, const vector2<U>& y) -> vector2<T> {
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
        friend constexpr auto operator-(vector2<T> x, const vector2<U>& y) -> vector2<T> {
            return x -= y;
        }

        /**
         * @brief Multiply by a scalar
         *
         * @param[in] x
         * @param[in] alpha
         * @return vector2<T>
         */
        friend constexpr auto operator*(vector2<T> x, const T& alpha) -> vector2<T> {
            return x *= alpha;
        }

        /**
         * @brief Multiply (by a scalar)
         *
         * @param[in] alpha
         * @param[in] x
         * @return vector2<T>
         */
        friend constexpr auto operator*(const T& alpha, vector2<T> x) -> vector2<T> {
            return x *= alpha;
        }

        /**
         * @brief Divide (by a scalar)
         *
         * @param[in] x
         * @param[in] alpha
         * @return vector2<T>
         */
        friend constexpr auto operator/(vector2<T> x, const T& alpha) -> vector2<T> {
            return x /= alpha;
        }

        ///@}
    };

    /**
     * @brief
     *
     * @tparam Stream
     * @tparam T
     * @param[in] out
     * @param[in] v
     * @return Stream&
     */
    template <typename T, class Stream> auto operator<<(Stream& out, const vector2<T>& v)
        -> Stream&;

}  // namespace recti