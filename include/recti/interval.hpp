#pragma once

// #include <algorithm> // import std::min, std::max
#include "generic.hpp"
#include <utility>  // import std::move

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
        constexpr auto len() const -> T { return this->ub() - this->lb(); }

        /** @name Comparison operators
         *  definie ==, !=, <, >, <=, >=.
         */
        ///@{

        /**
         * @brief Equal to
         *
         * The above code is defining the equality operator (==) for a class template called
         * Interval. The operator compares two Interval objects for equality by checking if their
         * lower bounds (lb) and upper bounds (ub) are equal. If both the lower bounds and upper
         * bounds are equal, the operator returns true, otherwise it returns false.
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U>  //
        constexpr auto operator==(const Interval<U> &rhs) const -> bool {
            return this->lb() == rhs.lb() && this->ub() == rhs.ub();
        }

        /**
         * @brief Not equal to
         *
         * The above code is defining the "!=" (not equal to) operator for a class template called
         * "Interval". This operator compares two Interval objects for inequality. It takes a
         * reference to an Interval object "rhs" as a parameter and returns a boolean value. It uses
         * the "==" (equal to) operator to determine if the two objects are equal, and then negates
         * the result to return the opposite.
         *
         * @tparam U
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U>  //
        constexpr auto operator!=(const Interval<U> &rhs) const -> bool {
            return !(*this == rhs);
        }

        /**
         * @brief Spaceship operator
         *
         * The above code is defining a spaceship operator for a template class. The spaceship
         * operator is a three-way comparison operator introduced in C++20. It compares the object
         * on the left-hand side (LHS) with the object on the right-hand side (RHS) and returns a
         * std::weak_ordering value indicating the relationship between the two objects.
         *
         * @tparam U
         * @param[in] rhs
         * @return std::weak_ordering
         */
        template <typename U>  //
        constexpr auto operator<=>(const U &rhs) const -> std::weak_ordering {
            if (this->ub() < rhs) return std::weak_ordering::less;
            if (this->lb() > rhs) return std::weak_ordering::greater;
            return std::weak_ordering::equivalent;
        }

        /**
         * @brief Spaceship operator
         *
         * The above code is defining a spaceship operator for comparing a value of type `T` with an
         * `Interval` object. The spaceship operator (`<=>`) is a three-way comparison operator
         * introduced in C++20.
         *
         * @param[in] lhs
         * @param[in] rhs
         * @return std::weak_ordering
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
         * @brief Negation
         *
         * The above function returns the negation of an interval.
         *
         * @return The `operator-` function is returning an `Interval` object.
         */
        constexpr auto operator-() const -> Interval { return {-this->_ub, -this->_lb}; }

        /**
         * @brief Add
         *
         * The above code is defining an `operator+=` function for the `Interval` class. This
         * function takes a parameter `alpha` of type `U` and adds it to both the lower bound
         * (`_lb`) and upper bound (`_ub`) of the `Interval` object. It then returns a reference to
         * the modified `Interval` object.
         *
         * @param[in] alpha
         * @return Interval&
         */
        template <typename U> constexpr auto operator+=(const U &alpha) -> Interval & {
            this->_lb += alpha;
            this->_ub += alpha;
            return *this;
        }

        /**
         * @brief Add by a scalar
         *
         * The above code is defining a friend function named `operator+` for the `Interval` class.
         * This function takes an `Interval` object `rhs` and a scalar value `alpha` as parameters.
         * It returns a new `Interval` object that is the result of adding `rhs` and `alpha`.
         *
         * @param[in] rhs The parameter `rhs` represents the right-hand side of the addition
         * operation. It is an object of type `Interval`.
         * @param[in] alpha The parameter `alpha` is a scalar value that will be added to the `rhs`
         * interval.
         *
         * @return an `Interval` object.
         */
        template <typename U> friend constexpr auto operator+(Interval rhs, const U &alpha)
            -> Interval {
            return rhs += alpha;
        }

        /**
         * @brief Add (by a scalar)
         *
         * The function adds a scalar value to an interval.
         *
         * @param[in] alpha The parameter `alpha` is a scalar value that will be added to the `rhs`
         * interval.
         * @param[in] rhs The parameter `rhs` represents the right-hand side of the addition
         * operation. It is an object of type `Interval`.
         *
         * @return an `Interval` object.
         */
        friend constexpr auto operator+(const T &alpha, Interval rhs) -> Interval {
            return rhs += alpha;
        }

        /**
         * @brief Subtract
         *
         * The above code is defining an `operator-=` function for the `Interval` class. This
         * function takes a parameter `alpha` of type `U` and subtracts it to both the lower bound
         * (`_lb`) and upper bound (`_ub`) of the `Interval` object. It then returns a reference to
         * the modified `Interval` object.
         *
         * @param[in] alpha
         * @return Interval&
         */
        template <typename U> constexpr auto operator-=(const U &alpha) -> Interval & {
            this->_lb -= alpha;
            this->_ub -= alpha;
            return *this;
        }

        /**
         * @brief Subtract by a scalar
         *
         * The above code is defining a friend function named `operator-` for the `Interval` class.
         * This function takes an `Interval` object `rhs` and a scalar value `alpha` as parameters.
         * It returns a new `Interval` object that is the result of subtracting `rhs` by `alpha`.
         *
         * @param[in] rhs The parameter `rhs` represents the right-hand side of the subtraction
         * operation. It is an object of type `Interval`.
         * @param[in] alpha The parameter `alpha` is a scalar value that will be subtracted to the
         * `rhs` interval.
         *
         * @return an `Interval` object.
         */
        template <typename U> friend constexpr auto operator-(const Interval &rhs, const U &alpha)
            -> Interval {
            auto lower = rhs.lb() - alpha;
            auto upper = rhs.ub() - alpha;
            return Interval<decltype(lower)>{std::move(lower), std::move(upper)};
        }

        /**
         * @brief Enlarge with
         *
         * The `enlarge_with` function enlarges the interval by subtracting `alpha` from the lower
         * bound and adding `alpha` to the upper bound.
         *
         * @param[in] alpha The parameter "alpha" is of type T, which is not specified in the code
         * snippet. It could be any data type, such as int, float, double, etc.
         *
         * @return a reference to the current object, which is an instance of the `Interval` class.
         */
        constexpr auto enlarge_with(const T &alpha) -> Interval & {
            this->_lb -= alpha;
            this->_ub += alpha;
            return *this;
        }

        ///@}

        /**
         * @brief Overlaps
         *
         * The above code is defining a template function named "overlaps" that takes a parameter
         * "other" of type U. The function returns a boolean value.
         *
         * @tparam U
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U>  // cppcheck-suppress internalAstError
        constexpr auto overlaps(const U &other) const -> bool {
            return !(*this < other || other < *this);
        }

        /**
         * @brief contains
         *
         * The above code is defining a template function named "contains" that takes a parameter
         * "other" of type U. The function returns a boolean value.
         *
         * @tparam U
         * @param[in] other
         * @return true
         * @return false
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
         * @brief intersection with
         *
         * The above code is defining a template function called `intersection_with` that takes a
         * parameter `other`. The function returns the intersection of the current object (an
         * `Interval`) with `other`.
         *
         * @tparam U
         * @param[in] other
         * @return constexpr auto
         */
        template <typename U>  //
        constexpr auto intersection_with(const U &other) const {
            if constexpr (requires { other.lb(); }) {
                return Interval<T>{this->lb() > other.lb() ? this->lb() : T(other.lb()),
                                   this->ub() < other.ub() ? this->ub() : T(other.ub())};
            } else /* constexpr */ {  // assume scalar
                return Interval<U>{other, other};
            }
        }

        /**
         * @brief minimum distance with
         *
         * The above code is a C++ template function called `min_dist_with`. It takes a parameter
         * `other` of type `U` and returns a value of type `T`.
         *
         * @tparam U
         * @param[in] other
         * @return constexpr auto
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
         * @brief minimum distance with
         *
         * The above code is a C++ template function called `min_dist_change_with`. It takes a
         * parameter `other` of type `U` and returns a value of type `T`.
         *
         * @tparam U
         * @param[in] other
         * @return constexpr auto
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
                *this = other = this->intersection_with(other);
            } else /* constexpr */ {  // assume scalar
                this->_ub = this->_lb = other;
            }
            return T(0);
        }

        /**
         * @brief output stream
         *
         * The above code is defining a friend function for the Interval class in C++. This friend
         * function is an output stream operator overload (<<) that takes a reference to a Stream
         * object (out) and a const reference to an Interval object (intvl).
         *
         * @tparam Stream
         * @param[out] out
         * @param[in] intvl
         * @return Stream&
         */
        template <class Stream> friend auto operator<<(Stream &out, const Interval &intvl)
            -> Stream & {
            out << "[" << intvl.lb() << ", " << intvl.ub() << "]";
            return out;
        }
    };
#pragma pack(pop)

    /** The above code is defining a template function called `enlarge`. This function takes two
    parameters, `lhs` and `rhs`, of any type `U1` and `U2` respectively. */
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
