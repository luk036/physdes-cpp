#pragma once

// #include <algorithm> // import std::min, std::max
#include <utility>  // import std::move

#include "generic.hpp"

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief Interval
     *
     * The above code is defining a template class called "Interval" with a default
     * template parameter of type "int". The class has two private member variables:
     * "_lb" (lower bound) and "_ub" (upper bound).
     *
     * @tparam T
     */
    template <typename T = int> class Interval {
      private:
        T _lb;  //> lower bound
        T _ub;  //> upper bound

      public:
        using value_type = T;

        /**
         * @brief Construct a new Interval object
         *
         * The function constructs a new Interval object with given lower and upper values.
         *
         * @param[in] lower The lower bound of the interval.
         * @param[in] upper The "upper" parameter is the upper bound of the interval. It represents
         * the maximum value that can be included in the interval.
         */
        constexpr Interval(T &&lower, T &&upper) noexcept
            : _lb{std::move(lower)}, _ub{std::move(upper)} {}

        /**
         * @brief Construct a new Interval object
         *
         * The function constructs a new Interval object with given lower and upper bounds.
         *
         * @param[in] lower The lower bound of the interval. It represents the minimum value that
         * can be included in the interval.
         * @param[in] upper The "upper" parameter represents the upper bound of the interval. It is
         * the maximum value that can be included in the interval.
         */
        constexpr Interval(const T &lower, const T &upper) : _lb{lower}, _ub{upper} {}

        /**
         * @brief Construct a new Interval object
         *
         * The function constructs a new Interval object with the same lower and upper bounds.
         *
         * @param[in] alpha The parameter "alpha" is of type T, which is a template parameter for
         * the Interval class. It represents the value that will be used as both the lower bound and
         * upper bound of the interval.
         */
        explicit constexpr Interval(const T &alpha) : _lb{alpha}, _ub{alpha} {}

        /**
         * @brief Assignment operator
         *
         * The assignment operator sets the lower and upper bounds of an Interval object to the
         * given value.
         *
         * @param[in] alpha The parameter "alpha" is of type T, which is the type of the object
         * being assigned to the Interval object.
         *
         * @return The assignment operator is returning a reference to the Interval object.
         */
        constexpr auto operator=(const T &alpha) -> Interval & {
            this->_lb = this->_ub = alpha;
            return *this;
        }

        /**
         * @brief lower bound
         *
         * The function returns a constant reference to the lower bound value.
         *
         * @return a reference to a constant object of type T.
         */
        constexpr auto lb() const -> const T & { return this->_lb; }

        /**
         * @brief upper bound
         *
         * The function returns a constant reference to the upper bound value.
         *
         * @return a reference to a constant object of type T.
         */
        constexpr auto ub() const -> const T & { return this->_ub; }

        /**
         * @brief length
         *
         * The function returns the length of a range by subtracting the upper bound from the lower
         * bound.
         *
         * @return a value of type T.
         */
        constexpr auto length() const -> T { return this->ub() - this->lb(); }

        /**
         * @brief Checks if the interval is invalid
         *
         * This function checks if the interval is invalid, which occurs when the lower bound is
         * greater than the upper bound.
         *
         * @return `true` if the interval is invalid, `false` otherwise.
         */
        constexpr auto is_invalid() const -> bool { return this->lb() > this->ub(); }

        /** @name Comparison operators
         *  definie ==, !=, <, >, <=, >=.
         */
        ///@{

        /**
         * @brief Equality comparison operator for `Interval` objects.
         *
         * Compares two `Interval` objects for equality by checking if their lower bounds (`lb`) and
         * upper bounds (`ub`) are equal. If both the lower bounds and upper bounds are equal, the
         * operator returns `true`, otherwise it returns `false`.
         *
         * @tparam U The type of the interval bounds.
         * @param rhs The right-hand side `Interval` object to compare against.
         * @return `true` if the intervals are equal, `false` otherwise.
         */
        template <typename U>  //
        constexpr auto operator==(const Interval<U> &rhs) const -> bool {
            return this->lb() == rhs.lb() && this->ub() == rhs.ub();
        }

        /**
         * @brief Not equal to comparison operator for `Interval` objects.
         *
         * Compares two `Interval` objects for inequality by negating the result of the `==` (equal
         * to) operator. If the two intervals are not equal, the operator returns `true`, otherwise
         * it returns `false`.
         *
         * @tparam U The type of the interval bounds.
         * @param[in] rhs The right-hand side `Interval` object to compare against.
         * @return `true` if the intervals are not equal, `false` otherwise.
         */
        template <typename U>  //
        constexpr auto operator!=(const Interval<U> &rhs) const -> bool {
            return !(*this == rhs);
        }

        /**
         * @brief Spaceship comparison operator for `Interval` objects.
         *
         * Compares the upper bound (`ub`) of the `Interval` object on the left-hand side (LHS) with
         * the value on the right-hand side (RHS). If the LHS upper bound is less than the RHS
         * value, it returns `std::weak_ordering::less`. If the LHS lower bound is greater than the
         * RHS value, it returns `std::weak_ordering::greater`. Otherwise, it returns
         * `std::weak_ordering::equivalent`.
         *
         * @tparam U The type of the value to compare against the interval bounds.
         * @param[in] rhs The value to compare against the interval bounds.
         * @return `std::weak_ordering` indicating the relationship between the interval and the
         * value.
         */
        template <typename U>  //
        constexpr auto operator<=>(const U &rhs) const -> std::weak_ordering {
            if (this->ub() < rhs) return std::weak_ordering::less;
            if (this->lb() > rhs) return std::weak_ordering::greater;
            return std::weak_ordering::equivalent;
        }

        /**
         * @brief Spaceship comparison operator for comparing a value with an `Interval` object.
         *
         * Compares the given value with the bounds of the `Interval` object on the right-hand side.
         * If the value is less than the lower bound of the interval, it returns
         * `std::weak_ordering::less`. If the value is greater than the upper bound of the interval,
         * it returns `std::weak_ordering::greater`. Otherwise, it returns
         * `std::weak_ordering::equivalent`.
         *
         * @tparam T The type of the value to compare against the interval bounds.
         * @param[in] lhs The value to compare against the interval bounds.
         * @param[in] rhs The `Interval` object to compare the value against.
         * @return `std::weak_ordering` indicating the relationship between the value and the
         * interval.
         */
        friend constexpr auto operator<=>(const T &lhs, const Interval &rhs) -> std::weak_ordering {
            if (lhs < rhs.lb()) return std::weak_ordering::less;
            if (lhs > rhs.ub()) return std::weak_ordering::greater;
            return std::weak_ordering::equivalent;
        }

        ///@}

        /** @name Arithmetic operators
         *  definie +, -, *, /, +=, -=, +=, /=, etc.
         */
        ///@{

        /**
         * @brief Negation operator for an `Interval` object.
         *
         * This function returns a new `Interval` object that is the negation of the current
         * `Interval` object. The lower bound of the new interval is the negation of the upper bound
         * of the current interval, and the upper bound of the new interval is the negation of the
         * lower bound of the current interval.
         *
         * @return The negated `Interval` object.
         */
        constexpr auto operator-() const -> Interval { return {-this->_ub, -this->_lb}; }

        /**
         * @brief Add a value to the lower and upper bounds of the interval.
         *
         * This function adds the given value `alpha` to both the lower bound (`_lb`) and upper
         * bound (`_ub`) of the `Interval` object. It then returns a reference to the modified
         * `Interval` object.
         *
         * @tparam U The type of the value to add to the interval bounds.
         * @param[in] alpha The value to add to the interval bounds.
         * @return A reference to the modified `Interval` object.
         */
        template <typename U> constexpr auto operator+=(const U &alpha) -> Interval & {
            this->_lb += alpha;
            this->_ub += alpha;
            return *this;
        }

        /**
         * @brief Add a scalar value to an `Interval` object.
         *
         * This function creates a new `Interval` object by adding the scalar value `alpha` to the
         * lower and upper bounds of the `rhs` `Interval` object.
         *
         * @tparam U The type of the scalar value to add.
         * @param[in] rhs The `Interval` object to add the scalar value to.
         * @param[in] alpha The scalar value to add to the `Interval` object.
         * @return A new `Interval` object with the scalar value added to the bounds.
         */
        template <typename U> friend constexpr auto operator+(Interval rhs, const U &alpha)
            -> Interval {
            return rhs += alpha;
        }

        /**
         * @brief Add (by a scalar)
         *
         * The function adds a scalar value `alpha` to an `Interval` object `rhs` and returns the
         * resulting `Interval` object.
         *
         * @param[in] alpha The scalar value to add to the `Interval` object.
         * @param[in] rhs The `Interval` object to add the scalar value to.
         *
         * @return A new `Interval` object with the scalar value added to the bounds.
         */
        friend constexpr auto operator+(const T &alpha, Interval rhs) -> Interval {
            return rhs += alpha;
        }

        /**
         * @brief Subtract a scalar value from an `Interval` object.
         *
         * This function subtracts the scalar value `alpha` from the lower and upper bounds of the
         * `Interval` object and returns a reference to the modified `Interval` object.
         *
         * @tparam U The type of the scalar value to subtract.
         * @param[in] alpha The scalar value to subtract from the `Interval` object.
         * @return A reference to the modified `Interval` object.
         */
        template <typename U> constexpr auto operator-=(const U &alpha) -> Interval & {
            this->_lb -= alpha;
            this->_ub -= alpha;
            return *this;
        }

        /**
         * @brief Subtract a scalar value from an `Interval` object.
         *
         * This function subtracts the scalar value `alpha` from the lower and upper bounds of the
         * `Interval` object `rhs` and returns a new `Interval` object with the modified bounds.
         *
         * @tparam U The type of the scalar value to subtract.
         * @param[in] rhs The `Interval` object to subtract the scalar value from.
         * @param[in] alpha The scalar value to subtract from the `Interval` object.
         *
         * @return A new `Interval` object with the scalar value subtracted from the bounds.
         */
        template <typename U> friend constexpr auto operator-(const Interval &rhs, const U &alpha)
            -> Interval {
            auto lower = rhs.lb() - alpha;
            auto upper = rhs.ub() - alpha;
            return Interval<decltype(lower)>{std::move(lower), std::move(upper)};
        }

        /**
         * @brief Enlarge the interval by subtracting `alpha` from the lower bound and adding
         * `alpha` to the upper bound.
         *
         * This function modifies the current `Interval` object by subtracting `alpha` from the
         * lower bound (`_lb`) and adding `alpha` to the upper bound (`_ub`). It returns a reference
         * to the modified `Interval` object.
         *
         * @param[in] alpha The value to subtract from the lower bound and add to the upper bound.
         * @return A reference to the modified `Interval` object.
         */
        constexpr auto enlarge_with(const T &alpha) -> Interval & {
            this->_lb -= alpha;
            this->_ub += alpha;
            return *this;
        }

        ///@}

        /**
         * @brief Checks if the current interval overlaps with another interval.
         *
         * This function checks if the current `Interval` object overlaps with the interval
         * represented by the `other` parameter. It returns `true` if the intervals overlap, and
         * `false` otherwise.
         *
         * @tparam U The type of the `other` interval.
         * @param[in] other The interval to check for overlap.
         * @return `true` if the intervals overlap, `false` otherwise.
         */
        template <typename U>  // cppcheck-suppress internalAstError
        constexpr auto overlaps(const U &other) const -> bool {
            return !(*this < other || other < *this);
        }

        /**
         * @brief Checks if the current interval contains another interval.
         *
         * This function checks if the current `Interval` object contains the interval
         * represented by the `other` parameter. If the `other` parameter has a `lb()`
         * and `ub()` member function, it checks if the lower bound of the current
         * interval is less than or equal to the lower bound of the `other` interval,
         * and the upper bound of the current interval is greater than or equal to the
         * upper bound of the `other` interval. Otherwise, it assumes the `other`
         * parameter is a scalar value and checks if it is within the bounds of the
         * current interval.
         *
         * @tparam U The type of the `other` interval or scalar value.
         * @param[in] other The interval or scalar value to check if it is contained
         * within the current interval.
         * @return `true` if the current interval contains the `other` interval or
         * scalar value, `false` otherwise.
         */
        template <typename U>  // cppcheck-suppress internalAstError
        constexpr auto contains(const U &other) const -> bool {
            if constexpr (requires { other.lb(); }) {
                return this->lb() <= other.lb() && other.ub() <= this->ub();
            } else /* constexpr */ {  // assume scalar
                return this->lb() <= other && other <= this->ub();
            }
        }

        /**
         * @brief Computes the intersection of the current interval with another interval or scalar
         * value.
         *
         * This function returns a new `Interval` object that represents the intersection of the
         * current `Interval` object with the interval or scalar value represented by the `other`
         * parameter. If the `other` parameter has `lb()` and `ub()` member functions, it computes
         * the intersection using the lower and upper bounds of both intervals. Otherwise, it
         * assumes the `other` parameter is a scalar value and computes the intersection using the
         * lower and upper bounds of the current interval and the scalar value.
         *
         * @tparam U The type of the `other` interval or scalar value.
         * @param[in] other The interval or scalar value to intersect with the current interval.
         * @return The intersection of the current interval with the `other` interval or scalar
         * value.
         */
        template <typename U>  //
        constexpr auto intersect_with(const U &other) const {
            if constexpr (requires { other.lb(); }) {
                return Interval<T>{this->lb() > other.lb() ? this->lb() : T(other.lb()),
                                   this->ub() < other.ub() ? this->ub() : T(other.ub())};
            } else /* constexpr */ {  // assume scalar
                return Interval<T>{this->lb() > other ? this->lb() : T(other),
                                   this->ub() < other ? this->ub() : T(other)};
            }
        }

        /**
         * @brief Computes the hull of the current interval with another interval or scalar
         * value.
         *
         * This function returns a new `Interval` object that represents the hull of the
         * current `Interval` object with the interval or scalar value represented by the `other`
         * parameter. If the `other` parameter has `lb()` and `ub()` member functions, it computes
         * the hull using the lower and upper bounds of both intervals. Otherwise, it
         * assumes the `other` parameter is a scalar value and computes the hull using the
         * lower and upper bounds of the current interval and the scalar value.
         *
         * @tparam U The type of the `other` interval or scalar value.
         * @param[in] other The interval or scalar value to intersect with the current interval.
         * @return The hull of the current interval with the `other` interval or scalar
         * value.
         */
        template <typename U>  //
        constexpr auto hull_with(const U &other) const {
            if constexpr (requires { other.lb(); }) {
                return Interval<T>{this->lb() < other.lb() ? this->lb() : T(other.lb()),
                                   this->ub() > other.ub() ? this->ub() : T(other.ub())};
            } else /* constexpr */ {  // assume scalar
                return Interval<T>{this->lb() < other ? this->lb() : T(other),
                                   this->ub() > other ? this->ub() : T(other)};
            }
        }

        /**
         * @brief Computes the minimum distance between the current interval and the `other`
         * interval or scalar value.
         *
         * This function returns the minimum distance between the current interval and the interval
         * or scalar value represented by the `other` parameter. If the `other` parameter is less
         * than the current interval, the function returns the distance between the lower bound of
         * the current interval and the `other` parameter. If the `other` parameter is greater than
         * the current interval, the function returns the distance between the upper bound of the
         * current interval and the `other` parameter. If the `other` parameter is within the
         * current interval, the function returns 0.
         *
         * @tparam U The type of the `other` interval or scalar value.
         * @param[in] other The interval or scalar value to compute the minimum distance with.
         * @return The minimum distance between the current interval and the `other` interval or
         * scalar value.
         */
        template <typename U> constexpr auto min_dist_with(const U &other) const -> T {
            if (*this < other) {
                return min_dist(this->_ub, other);
            }
            if (other < *this) {
                return min_dist(this->_lb, other);
            }
            return T(0);
        }

        /**
         * @brief Computes the minimum distance between the current interval and the `other`
         * interval or scalar value, and updates the interval bounds accordingly.
         *
         * This function returns the minimum distance between the current interval and the interval
         * or scalar value represented by the `other` parameter. If the `other` parameter is less
         * than the current interval, the function updates the lower bound of the current interval
         * to the upper bound, and returns the distance between the new lower bound and the `other`
         * parameter. If the `other` parameter is greater than the current interval, the function
         * updates the upper bound of the current interval to the lower bound, and returns the
         * distance between the new upper bound and the `other` parameter. If the `other` parameter
         * is within the current interval, the function updates the interval to the intersection
         * of the current interval and the `other` interval or scalar value, and returns 0.
         *
         * @tparam U The type of the `other` interval or scalar value.
         * @param[in,out] other The interval or scalar value to compute the minimum distance with.
         * @return The minimum distance between the current interval and the `other` interval or
         * scalar value.
         */
        template <typename U> constexpr auto min_dist_change_with(U &other) const -> T {
            if (*this < other) {
                this->_lb = this->_ub;
                return min_dist_change(this->_ub, other);
            }
            if (other < *this) {
                this->_ub = this->_lb;
                return min_dist_change(this->_lb, other);
            }

            if constexpr (requires { other.lb(); }) {
                *this = other = this->intersect_with(other);
            } else /* constexpr */ {  // assume scalar
                this->_ub = this->_lb = other;
            }
            return T(0);
        }

        /**
         * @brief Overloads the stream insertion operator (`<<`) to print an Interval object to the
         * given output stream.
         *
         * This function allows Interval objects to be printed directly to an output stream, such as
         * `std::cout`, by overloading the stream insertion operator (`<<`). The Interval object is
         * printed in the format `[lower_bound, upper_bound]`.
         *
         * @tparam Stream The type of the output stream.
         * @param out The output stream to write the Interval object to.
         * @param intvl The Interval object to be printed.
         * @return A reference to the output stream (`out`) for chaining stream operations.
         */
        template <class Stream> friend auto operator<<(Stream &out, const Interval &intvl)
            -> Stream & {
            out << "[" << intvl.lb() << ", " << intvl.ub() << "]";
            return out;
        }
    };
#pragma pack(pop)

    /**
     * @brief Computes the hull of two objects.
     *
     * This function computes the hull of the two input objects `lhs` and `rhs`. It first
     * checks if `lhs` has an `hull_with` member function that can be called with `rhs` as an
     * argument, and returns the result of that call. If `lhs` does not have an `hull_with`
     * member function, it checks if `rhs` has an `hull_with` member function that can be
     * called with `lhs` as an argument, and returns the result of that call. If neither object has
     * an `hull_with` member function, the function asserts that `lhs` and `rhs` are equal and
     * returns `lhs`.
     *
     * @tparam U1 The type of the first object.
     * @tparam U2 The type of the second object.
     * @param[in] lhs The first object.
     * @param[in] rhs The second object.
     * @return The hull of `lhs` and `rhs`.
     */
    template <typename U1, typename U2>  //
    constexpr auto hull(const U1 &lhs, const U2 &rhs) {
        if constexpr (requires { lhs.hull_with(rhs); }) {
            return lhs.hull_with(rhs);
        } else if constexpr (requires { rhs.hull_with(lhs); }) {
            return rhs.hull_with(lhs);
        } else /* constexpr */ {
            if (lhs < rhs) {
               return Interval<U1>(lhs, rhs);
            }
            return Interval<U2>(rhs, lhs);
        }
    }

    /**
     * @brief Enlarges an interval or scalar value by adding and subtracting a given value.
     *
     * This function takes two parameters, `lhs` and `rhs`, and returns a new interval that
     * is enlarged by the value of `rhs`. If `lhs` has an `enlarge_with` member function,
     * it is used to enlarge the interval. Otherwise, a new interval is created with the
     * lower bound as `lhs - rhs` and the upper bound as `lhs + rhs`.
     *
     * @tparam U1 The type of the first parameter `lhs`.
     * @tparam U2 The type of the second parameter `rhs`.
     * @param lhs The interval or scalar value to be enlarged.
     * @param rhs The value to enlarge the interval or scalar value by.
     * @return The enlarged interval or scalar value.
     */
    template <typename U1, typename U2>  //
    constexpr auto enlarge(const U1 &lhs, const U2 &rhs) {
        if constexpr (requires { lhs.enlarge_with(rhs); }) {
            auto res{lhs};
            res.enlarge_with(rhs);
            return res;
        } else /* constexpr */ {
            return Interval<U1>{lhs - rhs, lhs + rhs};
        }
    }
}  // namespace recti
