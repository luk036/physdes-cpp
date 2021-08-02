#pragma once

#include <tuple>    // import std::tie()
#include <utility>  // import std::move

namespace recti {

    /**
     * @brief vector2
     *
     */
    template <typename T1, typename T2 = T1> class vector2 {
      private:
        T1 _x;
        T2 _y;

        auto _tie() const { return std::tie(_x, _y); }

      public:
        /**
         * @brief Construct a new vector2 object
         *
         * @param x
         * @param y
         */
        constexpr vector2(T1&& x, T2&& y) noexcept : _x{std::move(x)}, _y{std::move(y)} {}

        /**
         * @brief Construct a new vector2 object
         *
         * @param x
         * @param y
         */
        constexpr vector2(const T1& x, const T2& y) : _x{x}, _y{y} {}

        /**
         * @brief Construct a new vector2 object
         *
         * @tparam U1
         * @tparam U2
         */
        template <typename U1, typename U2> constexpr vector2(const vector2<U1, U2>& other)
            : _x(other.x()), _y(other.y()) {}

        /**
         * @brief
         *
         * @return constexpr const T1&
         */
        [[nodiscard]] constexpr auto x() const noexcept -> const T1& { return this->_x; }

        /**
         * @brief
         *
         * @return constexpr const T2&
         */
        [[nodiscard]] constexpr auto y() const noexcept -> const T2& { return this->_y; }

        /**
         * @brief
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return constexpr auto
         */
        template <typename U1, typename U2>  //
        [[nodiscard]] constexpr auto cross(const vector2<U1, U2>& other) const {
            return this->_x * other._y - other._x * this->_y;
        }

        /** @name Comparison operators
         *  definie ==, !=, <, >, <=, >=.
         */
        ///@{

        /**
         * @brief Equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto operator==(const vector2<U1, U2>& other) const -> bool {
            return this->_tie() == other._tie();
        }

        /**
         * @brief Less than
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto operator<(const vector2<U1, U2>& other) const -> bool {
            return this->_tie() < other._tie();
        }

        /**
         * @brief Not equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto operator!=(const vector2<U1, U2>& other) const -> bool {
            return !(*this == other);
        }

        /**
         * @brief Greater than
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto operator>(const vector2<U1, U2>& other) const -> bool {
            return other < *this;
        }

        /**
         * @brief Less than or euqual to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto operator<=(const vector2<U1, U2>& other) const -> bool {
            return !(other < *this);
        }

        /**
         * @brief Greater than or equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto operator>=(const vector2<U1, U2>& other) const -> bool {
            return !(*this < other);
        }

        ///@}

        /** @name Arithmetic operators
         *  definie +, -, *, /, +=, -=, *=, /=, etc.
         */
        ///@{

        /**
         * @brief Negate
         *
         * @return vector2
         */
        constexpr auto operator-() const -> vector2 { return vector2(-this->_x, -this->_y); }

        /**
         * @brief Add
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return vector2&
         */
        template <typename U1, typename U2> constexpr auto operator+=(const vector2<U1, U2>& other)
            -> vector2& {
            this->_x += other.x();
            this->_y += other.y();
            return *this;
        }

        /**
         * @brief Substract
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return vector2&
         */
        template <typename U1, typename U2>  //
        constexpr auto operator-=(const vector2<U1, U2>& other) -> vector2& {
            this->_x -= other.x();
            this->_y -= other.y();
            return *this;
        }

        /**
         * @brief Multiply
         *
         * @tparam R
         * @param[in] alpha
         * @return vector2&
         */
        template <typename R> constexpr auto operator*=(const R& alpha) -> vector2& {
            this->_x *= alpha;
            this->_y *= alpha;
            return *this;
        }

        /**
         * @brief Divide
         *
         * @tparam R
         * @param[in] alpha
         * @return vector2&
         */
        template <typename R> constexpr auto operator/=(const R& alpha) -> vector2& {
            this->_x /= alpha;
            this->_y /= alpha;
            return *this;
        }

        /**
         * @brief Add
         *
         * @tparam U1
         * @tparam U2
         * @param[in] x
         * @param[in] y
         * @return vector2
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator+(vector2 x, const vector2<U1, U2>& y) -> vector2 {
            return x += y;
        }

        /**
         * @brief Substract
         *
         * @tparam U1
         * @tparam U2
         * @param[in] x
         * @param[in] y
         * @return vector2
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator-(vector2 x, const vector2<U1, U2>& y) -> vector2 {
            return x -= y;
        }

        /**
         * @brief Multiply by a scalar
         *
         * @tparam R
         * @param[in] x
         * @param[in] alpha
         * @return vector2
         */
        template <typename R> friend constexpr auto operator*(vector2 x, const R& alpha)
            -> vector2 {
            return x *= alpha;
        }

        /**
         * @brief Multiply (by a scalar)
         *
         * @tparam R
         * @param[in] alpha
         * @param[in] x
         * @return vector2
         */
        template <typename R> friend constexpr auto operator*(const R& alpha, vector2 x)
            -> vector2 {
            return x *= alpha;
        }

        /**
         * @brief Divide (by a scalar)
         *
         * @tparam R
         * @param[in] x
         * @param[in] alpha
         * @return vector2
         */
        template <typename R> friend constexpr auto operator/(vector2 x, const R& alpha)
            -> vector2 {
            return x /= alpha;
        }

        ///@}

        /**
         * @brief
         *
         * @tparam Stream
         * @param[out] out
         * @param[in] v
         * @return Stream&
         */
        template <class Stream> friend auto operator<<(Stream& out, const vector2& v) -> Stream& {
            out << "{" << v.x() << ", " << v.y() << "}";
            return out;
        }
    };
}  // namespace recti
