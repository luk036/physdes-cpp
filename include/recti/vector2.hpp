#pragma once

#include <tuple>    // import std::tie()
#include <utility>  // import std::move

#if __cpp_constexpr >= 201304
#    define CONSTEXPR14 constexpr
#else
#    define CONSTEXPR14 inline
#endif

namespace recti {

    /**
     * @brief vector2
     *
     */
    template <typename T1, typename T2 = T1> class Vector2 {
      private:
        T1 _x;
        T2 _y;

      public:
        /**
         * @brief Construct a new Vector2 object
         *
         * This is a constructor for the `Vector2` class. It takes two parameters
         * `vec_x` and `vec_y`, both of which are rvalue references (`T1&&` and
         * `T2&&`). The constructor is marked as `constexpr`, meaning it can be
         * evaluated at compile-time. The constructor is also marked as `noexcept`,
         * indicating that it does not throw any exceptions.
         *
         * @param[in] vec_x The x-coordinate of the vector.
         * @param[in] vec_y The y-coordinate of the vector.
         */
        constexpr Vector2(T1 vec_x, T2 vec_y) noexcept
            : _x{std::move(vec_x)}, _y{std::move(vec_y)} {}


        /**
         * @brief Construct a new Vector2 object
         *
         * This is a constructor for the `Vector2` class. It takes a `Vector2` object of
         * a different type (`U1` and `U2`) and initializes the current `Vector2` object
         * with the x and y coordinates of the other `Vector2` object. This constructor
         * is marked as `constexpr`, meaning it can be evaluated at compile-time.
         *
         * @tparam U1 The type of the x-coordinate of the other `Vector2` object.
         * @tparam U2 The type of the y-coordinate of the other `Vector2` object.
         * @param[in] other_vector The other `Vector2` object to copy from.
         */
        template <typename U1, typename U2> constexpr explicit Vector2(const Vector2<U1, U2>& other_vector)
            : _x(other_vector.x()), _y(other_vector.y()) {}

        /**
         * @brief Returns a const reference to the x-coordinate of the vector.
         *
         * This function provides read-only access to the x-coordinate of the `Vector2` object.
         *
         * @return constexpr const T1& A const reference to the x-coordinate of the vector.
         */
        constexpr auto x() const noexcept -> const T1& { return this->_x; }

        /**
         * @brief Returns a const reference to the y-coordinate of the vector.
         *
         * This function provides read-only access to the y-coordinate of the `Vector2` object.
         *
         * @return constexpr const T1& A const reference to the y-coordinate of the vector.
         */
        constexpr auto y() const noexcept -> const T2& { return this->_y; }

        /**
         * @brief Calculates the cross product of two `Vector2` objects.
         *
         * This function computes the cross product of the current `Vector2` object and the
         * provided `Vector2` object `other_vector`. The cross product of two 2D vectors is a scalar
         * value that represents the signed area of the parallelogram formed by the two vectors.
         *
         * @tparam U1 The type of the x-coordinate of the other `Vector2` object.
         * @tparam U2 The type of the y-coordinate of the other `Vector2` object.
         * @param[in] other_vector The other `Vector2` object to compute the cross product with.
         * @return constexpr auto The cross product of the two `Vector2` objects.
         */
        template <typename U1, typename U2>  //
        constexpr auto cross(const Vector2<U1, U2>& other_vector) const {
            return this->_x * other_vector._y - other_vector._x * this->_y;
        }

        /** @name Comparison operators
         *  definie ==, !=, <, >, <=, >=.
         */
        ///@{

        /**
         * @brief Compares two `Vector2` objects for equality.
         *
         * This operator compares the x and y coordinates of the current `Vector2` object
         * with the x and y coordinates of the provided `Vector2` object `other_vector`. The
         * comparison is done using `std::tie` to compare the individual coordinates.
         *
         * @tparam U1 The type of the x-coordinate of the other `Vector2` object.
         * @tparam U2 The type of the y-coordinate of the other `Vector2` object.
         * @param[in] other_vector The other `Vector2` object to compare against.
         * @return true If the two `Vector2` objects have the same x and y coordinates.
         */
        template <typename U1, typename U2>  //
        constexpr auto operator==(const Vector2<U1, U2>& other_vector) const -> bool {
            return std::tie(this->x(), this->y()) == std::tie(other_vector.x(), other_vector.y());
        }

        /**
         * @brief Compares two `Vector2` objects for inequality.
         *
         * This operator compares the x and y coordinates of the current `Vector2` object
         * with the x and y coordinates of the provided `Vector2` object `other_vector`. The
         * comparison is done using `std::tie` to compare the individual coordinates.
         *
         * @tparam U1 The type of the x-coordinate of the other `Vector2` object.
         * @tparam U2 The type of the y-coordinate of the other `Vector2` object.
         * @param[in] other_vector The other `Vector2` object to compare against.
         * @return true If the two `Vector2` objects have different x or y coordinates.
         */
        template <typename U1, typename U2>  //
        constexpr auto operator!=(const Vector2<U1, U2>& other_vector) const -> bool {
            return !(*this == other_vector);
        }

        ///@}

        /** @name Arithmetic operators
         *  definie +, -, *, /, +=, -=, *=, /=, etc.
         */
        ///@{

        /**
         * @brief Negate the vector, returning a new vector with the negated components.
         *
         * @return A new vector with the negated x and y components.
         */
        constexpr auto operator-() const -> Vector2 { return Vector2(-this->_x, -this->_y); }

        /**
         * @brief Add the components of the given vector to this vector.
         *
         * @tparam U1 The type of the x component of the other vector.
         * @tparam U2 The type of the y component of the other vector.
         * @param[in] other_vector The vector to add to this vector.
         * @return A reference to this vector, after the addition.
         */
        template <typename U1, typename U2>
        CONSTEXPR14 auto operator+=(const Vector2<U1, U2>& other_vector) -> Vector2& {
            this->_x += other_vector.x();
            this->_y += other_vector.y();
            return *this;
        }

        /**
         * @brief Subtract the components of the given vector from this vector.
         *
         * @tparam U1 The type of the x component of the other vector.
         * @tparam U2 The type of the y component of the other vector.
         * @param[in] other_vector The vector to subtract from this vector.
         * @return A reference to this vector, after the subtraction.
         */
        template <typename U1, typename U2>  //
        CONSTEXPR14 auto operator-=(const Vector2<U1, U2>& other_vector) -> Vector2& {
            this->_x -= other_vector.x();
            this->_y -= other_vector.y();
            return *this;
        }

        /**
         * @brief Multiply the components of this vector by the given scalar.
         *
         * @tparam R The type of the scalar to multiply by.
         * @param[in] scalar The scalar to multiply the vector components by.
         * @return A reference to this vector, after the multiplication.
         */
        template <typename R> CONSTEXPR14 auto operator*=(const R& scalar) -> Vector2& {
            this->_x *= scalar;
            this->_y *= scalar;
            return *this;
        }

        /**
         * @brief Divide the components of this vector by the given scalar.
         *
         * @tparam R The type of the scalar to divide by.
         * @param[in] scalar The scalar to divide the vector components by.
         * @return A reference to this vector, after the division.
         */
        template <typename R> CONSTEXPR14 auto operator/=(const R& scalar) -> Vector2& {
            this->_x /= scalar;
            this->_y /= scalar;
            return *this;
        }

        /**
         * @brief Add the components of the given vector to this vector.
         *
         * @tparam U1 The type of the x component of the other vector.
         * @tparam U2 The type of the y component of the other vector.
         * @param[in] right_vector The vector to add to this vector.
         * @param[in] left_vector The vector to add to the right-hand side vector.
         * @return A new vector that is the sum of the two input vectors.
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator+(Vector2 right_vector, const Vector2<U1, U2>& left_vector) -> Vector2 {
            return right_vector += left_vector;
        }

        /**
         * @brief Subtract the components of the given vector from this vector.
         *
         * @tparam U1 The type of the x component of the other vector.
         * @tparam U2 The type of the y component of the other vector.
         * @param[in] right_vector The vector to subtract from this vector.
         * @param[in] left_vector The vector to subtract the right-hand side vector from.
         * @return A new vector that is the difference of the two input vectors.
         */
        template <typename U1, typename U2>  //
        friend constexpr auto operator-(Vector2 right_vector, const Vector2<U1, U2>& left_vector) -> Vector2 {
            return right_vector -= left_vector;
        }

        /**
         * @brief Multiply the components of this vector by the given scalar.
         *
         * @tparam R The type of the scalar to multiply by.
         * @param[in] right_vector The vector to multiply.
         * @param[in] scalar The scalar to multiply the vector components by.
         * @return A new vector that is the result of multiplying the input vector by the scalar.
         */
        template <typename R> friend constexpr auto operator*(Vector2 right_vector, const R& scalar)
            -> Vector2 {
            return right_vector *= scalar;
        }

        /**
         * @brief Multiply the components of the given vector by the scalar.
         *
         * @tparam R The type of the scalar to multiply by.
         * @param[in] scalar The scalar to multiply the vector components by.
         * @param[in] left_vector The vector to multiply.
         * @return A new vector that is the result of multiplying the input vector by the scalar.
         */
        template <typename R> friend constexpr auto operator*(const R& scalar, Vector2 left_vector)
            -> Vector2 {
            return left_vector *= scalar;
        }

        /**
         * @brief Divide the components of this vector by the given scalar.
         *
         * @tparam R The type of the scalar to divide by.
         * @param[in] right_vector The vector to divide.
         * @param[in] scalar The scalar to divide the vector components by.
         * @return A new vector that is the result of dividing the input vector by the scalar.
         */
        template <typename R> friend constexpr auto operator/(Vector2 right_vector, const R& scalar)
            -> Vector2 {
            return right_vector /= scalar;
        }

        ///@}

        /**
         * @brief Overload the stream insertion operator to print a Vector2 object.
         *
         * @tparam Stream The stream type to insert the Vector2 into.
         * @param[out] output_stream The output stream to insert the Vector2 into.
         * @param[in] vector The Vector2 object to insert into the stream.
         * @return Stream& The modified output stream.
         */
        template <class Stream> friend auto operator<<(Stream& output_stream, const Vector2& vector)
            -> Stream& {
            output_stream << "{" << vector.x() << ", " << vector.y() << "}";
            return output_stream;
        }
    };
}  // namespace recti
