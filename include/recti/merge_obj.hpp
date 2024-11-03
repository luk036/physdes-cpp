#pragma once

// #include <algorithm> // import std::min, std::max
#include <cassert>
#include <utility>  // import std::move

#include "interval.hpp"
#include "point.hpp"

namespace recti {

#pragma pack(push, 1)
    /**
     * @brief Merging Object (for deferred-merge embedding (DME) algorithm)
     *
     * The code is defining a class template called `MergeObj` that represents a
     * merging segment. The merging segment can include a single point, segment, or
     * region. The template has two type parameters `T1` and `T2`, which default to
     * `int` if not specified. The class inherits from the `Point` class, which
     * represents a point in a 2D coordinate system. The `MergeObj` class provides
     * various operations and functions for manipulating and comparing merging
     * segments, such as adding and subtracting vectors, checking for overlap and
     * intersection with other merging segments, calculating the minimum distance
     * between merging segments, and merging two merging segments. The class also
     * provides comparison operators and a stream insertion operator for convenient
     * usage. The purpose of the `MergeObj` class is to support the deferred-merge
     * embedding (DME) algorithm, as referenced in the code comments.
     *
     * Reference:
     *  - Ting-Hai Chao, Yu-Chin Hsu, Jan-Ming Ho and A. B. Kahng, "Zero skew clock
     * routing with minimum wirelength," in IEEE Transactions on Circuits and
     * Systems II: Analog and Digital Signal Processing, vol. 39, no. 11, pp.
     * 799-814, Nov. 1992, doi: 10.1109/82.204128.
     *
     * @tparam T1 int or Interval
     * @tparam T2 int or Interval
     */
    template <typename T1 = int, typename T2 = T1> class MergeObj : private Point<T1, T2> {
      public:
        /**
         * @brief Construct a new MergeObj object
         *
         * This code defines a constructor for the `MergeObj` class template. The
         * constructor takes two parameters `xcoord` and `ycoord`, which are of type
         * `T1` and `T2` respectively. The constructor is marked as `constexpr` and
         * `noexcept`, indicating that it can be evaluated at compile-time and it does
         * not throw any exceptions.
         *
         * @param[in] xcoord The x coordinate value.
         * @param[in] ycoord The y coordinate value.
         */
        constexpr MergeObj(T1 &&xcoord, T2 &&ycoord) noexcept
            : Point<T1, T2>{std::move(xcoord), std::move(ycoord)} {}

        /**
         * @brief Construct a new MergeObj object from the given x and y coordinates.
         *
         * This static member function constructs a new `MergeObj` object by combining the
         * given `xcoord` and `ycoord` parameters in a specific way. The resulting `MergeObj`
         * object represents a merging segment that can include a single point, segment, or
         * region.
         *
         * @param[in] xcoord The x coordinate value.
         * @param[in] ycoord The y coordinate value.
         * @return A new `MergeObj` object constructed from the given coordinates.
         */
        static constexpr auto construct(T1 &&xcoord, T2 &&ycoord) -> MergeObj {
            return MergeObj{xcoord + ycoord, xcoord - ycoord};
        }

        /** @name Comparison operators
         *  definie ==, !=, <, >, <=, >=.
         */
        ///@{

        /**
         * @brief Compares two `MergeObj` objects for equality.
         *
         * This operator overload compares two `MergeObj` objects for equality. It returns `true` if
         * the underlying `Point<T1, T2>` objects are equal, and `false` otherwise.
         *
         * @tparam U1 The type of the x-coordinate of the right-hand side `MergeObj`.
         * @tparam U2 The type of the y-coordinate of the right-hand side `MergeObj`.
         * @param[in] rhs The `MergeObj` object to compare against.
         * @return `true` if the two `MergeObj` objects are equal, `false` otherwise.
         */
        template <typename U1, typename U2>
        constexpr auto operator==(const MergeObj<U1, U2> &rhs) const -> bool {
            return Point<T1, T2>::operator==(rhs);
        }

        /**
         * @brief Not equal to
         *
         * Compares two `MergeObj` objects for inequality.
         *
         * This operator overload compares two `MergeObj` objects for inequality. It returns `true`
         * if the underlying `Point<T1, T2>` objects are not equal, and `false` otherwise.
         *
         * @tparam U1 The type of the x-coordinate of the right-hand side `MergeObj`.
         * @tparam U2 The type of the y-coordinate of the right-hand side `MergeObj`.
         * @param[in] rhs The `MergeObj` object to compare against.
         * @return `true` if the two `MergeObj` objects are not equal, `false` otherwise.
         */
        template <typename U1, typename U2>
        constexpr auto operator!=(const MergeObj<U1, U2> &rhs) const -> bool {
            return Point<T1, T2>::operator!=(rhs);
        }

        /**
         * @brief Add a vector (translation) to this `MergeObj`.
         *
         * This operator overload adds a `Vector2<U>` to this `MergeObj` object, modifying the x and
         * y coordinates accordingly. The x coordinate is updated by adding the sum of the x and y
         * components of the `Vector2<U>`, while the y coordinate is updated by adding the
         * difference of the x and y components.
         *
         * @tparam U The type of the components of the `Vector2<U>` to add.
         * @param[in] rhs The `Vector2<U>` to add to this `MergeObj`.
         * @return A reference to this `MergeObj` after the addition.
         */
        template <typename U> constexpr auto operator+=(const Vector2<U> &rhs) -> MergeObj & {
            this->get_xcoord() += rhs.x() + rhs.y();
            this->get_ycoord() += rhs.x() - rhs.y();  // notice here!
            return *this;
        }

        /**
         * @brief Subtract a vector (translation)
         *
         * This operator overload subtracts a `Vector2<U>` from this `MergeObj` object, modifying
         * the x and y coordinates accordingly. The x coordinate is updated by subtracting the sum
         * of the x and y components of the `Vector2<U>`, while the y coordinate is updated by
         * subtracting the difference of the x and y components.
         *
         * @tparam U The type of the components of the `Vector2<U>` to subtract.
         * @param[in] rhs The `Vector2<U>` to subtract from this `MergeObj`.
         * @return A reference to this `MergeObj` after the subtraction.
         */
        template <typename U> constexpr auto operator-=(const Vector2<U> &rhs) -> MergeObj & {
            this->get_xcoord() -= rhs.x() + rhs.y();
            this->get_ycoord() -= rhs.x() - rhs.y();  // notice here!
            return *this;
        }

        /**
         * @brief Add a vector (translation) to a `MergeObj`.
         *
         * This operator overload adds a `Vector2<U>` to a `MergeObj` object, modifying the x and y
         * coordinates accordingly. The x coordinate is updated by adding the sum of the x and y
         * components of the `Vector2<U>`, while the y coordinate is updated by adding the
         * difference of the x and y components.
         *
         * @tparam U The type of the components of the `Vector2<U>` to add.
         * @param lhs The `MergeObj` to add the vector to.
         * @param rhs The `Vector2<U>` to add to the `MergeObj`.
         * @return A new `MergeObj` with the vector added.
         */
        template <typename U>  //
        friend constexpr auto operator+(MergeObj lhs, const Vector2<U> &rhs) -> MergeObj {
            return lhs += rhs;
        }

        /**
         * @brief Subtract a vector (translation) from a `MergeObj`.
         *
         * This operator overload subtracts a `Vector2<U>` from a `MergeObj` object, modifying the x
         * and y coordinates accordingly. The x coordinate is updated by subtracting the sum of the
         * x and y components of the `Vector2<U>`, while the y coordinate is updated by subtracting
         * the difference of the x and y components.
         *
         * @tparam U The type of the components of the `Vector2<U>` to subtract.
         * @param lhs The `MergeObj` to subtract the vector from.
         * @param rhs The `Vector2<U>` to subtract from the `MergeObj`.
         * @return A new `MergeObj` with the vector subtracted.
         */
        template <typename U>  //
        friend constexpr auto operator-(MergeObj lhs, const Vector2<U> &rhs) -> MergeObj {
            return lhs -= rhs;
        }

        /**
         * @brief Check if two `MergeObj` objects overlap.
         *
         * This function checks if the x and y coordinates of the current `MergeObj` object overlap
         * with the x and y coordinates of the provided `MergeObj` object `other`. The overlap is
         * determined by checking if the ranges of the x and y coordinates intersect.
         *
         * @tparam U1 The type of the x and y coordinates of the current `MergeObj` object.
         * @tparam U2 The type of the x and y coordinates of the `MergeObj` object `other`.
         * @param other The `MergeObj` object to check for overlap.
         * @return `true` if the two `MergeObj` objects overlap, `false` otherwise.
         */
        template <typename U1, typename U2>  //
        constexpr auto overlaps(const MergeObj<U1, U2> &other) const -> bool {
            return overlap(this->xcoord(), other.xcoord())
                   && overlap(this->ycoord(), other.ycoord());
        }

        /**
         * @brief Compute the intersection of two `MergeObj` objects.
         *
         * This function computes the intersection of the current `MergeObj` object with the
         * provided `MergeObj` object `other`. The intersection is calculated by finding the
         * intersection of the x and y coordinate ranges of the two `MergeObj` objects. The
         * resulting `MergeObj` object represents the overlapping region between the two input
         * `MergeObj` objects.
         *
         * @tparam U1 The type of the x and y coordinates of the current `MergeObj` object.
         * @tparam U2 The type of the x and y coordinates of the `MergeObj` object `other`.
         * @param[in] other The `MergeObj` object to intersect with the current `MergeObj` object.
         * @return A new `MergeObj` object representing the intersection of the two input `MergeObj`
         * objects.
         */
        template <typename U1, typename U2>  //
        constexpr auto intersect_with(const MergeObj<U1, U2> &other) const {
            auto xcoord = intersection(this->xcoord(), other.xcoord());
            auto ycoord = intersection(this->ycoord(), other.ycoord());
            return MergeObj<decltype(xcoord), decltype(ycoord)>{std::move(xcoord),
                                                                std::move(ycoord)};
        }

        /**
         * @brief Compute the minimum distance between the x and y coordinates of two `MergeObj`
         * objects.
         *
         * This function calculates the maximum distance between the x and y coordinates of the
         * current `MergeObj` object and the provided `MergeObj` object `other`. The distance is
         * computed by taking the maximum of the distances between the x coordinates and the
         * distances between the y coordinates of the two `MergeObj` objects.
         *
         * @tparam U1 The type of the x and y coordinates of the current `MergeObj` object.
         * @tparam U2 The type of the x and y coordinates of the `MergeObj` object `other`.
         * @param[in] other The `MergeObj` object to compute the maximum distance with.
         * @return The maximum distance between the two `MergeObj` objects.
         */
        template <typename U1, typename U2>  //
        constexpr auto min_dist_with(const MergeObj<U1, U2> &other) const {
            return std::max(min_dist(this->xcoord(), other.xcoord()),
                            min_dist(this->ycoord(), other.ycoord()));
        }

        /**
         * @brief Enlarge a `MergeObj` object by a given scale factor.
         *
         * This function creates a new `MergeObj` object by enlarging the x and y coordinates of the
         * input `MergeObj` object `lhs` by the given scale factor `alpha`. The resulting `MergeObj`
         * object will have larger x and y coordinate ranges compared to the input `MergeObj`
         * object.
         *
         * @tparam R The type of the scale factor `alpha`.
         * @param[in] lhs The `MergeObj` object to be enlarged.
         * @param[in] alpha The scale factor to enlarge the `MergeObj` object by.
         * @return A new `MergeObj` object with enlarged x and y coordinates.
         */
        template <typename R>  //
        friend constexpr auto enlarge(const MergeObj &lhs, const R &alpha) {
            auto xcoord = enlarge(lhs.xcoord(), alpha);
            auto ycoord = enlarge(lhs.ycoord(), alpha);
            return MergeObj<decltype(xcoord), decltype(ycoord)>{std::move(xcoord),
                                                                std::move(ycoord)};
        }

        /**
         * @brief Compute the intersection of two `MergeObj` objects by enlarging them and finding
         * the overlap.
         *
         * This function first calculates the minimum distance between the x and y coordinates of
         * the current `MergeObj` object and the provided `MergeObj` object `other`. It then uses
         * this distance to enlarge both `MergeObj` objects, with the current object being enlarged
         * by half the distance and the `other` object being enlarged by the remaining half.
         * Finally, it computes the intersection of the two enlarged `MergeObj` objects and returns
         * the result.
         *
         * @tparam U1 The type of the x and y coordinates of the current `MergeObj` object.
         * @tparam U2 The type of the x and y coordinates of the `MergeObj` object `other`.
         * @param[in] other The `MergeObj` object to compute the intersection with.
         * @return The intersection of the two `MergeObj` objects.
         */
        template <typename U1, typename U2>  //
        constexpr auto merge_with(const MergeObj<U1, U2> &other) const {
            auto alpha = this->min_dist_with(other);
            auto half = alpha / 2;
            auto trr1 = enlarge(*this, half);
            auto trr2 = enlarge(other, alpha - half);
            return intersection(trr1, trr2);
        }

        /**
         * @brief Overload the stream insertion operator `<<` to output a `MergeObj` object.
         *
         * This function overloads the stream insertion operator `<<` to output a `MergeObj` object
         * in the format `"/{xcoord}, {ycoord}/"`, where `{xcoord}` and `{ycoord}` are the x and y
         * coordinates of the `MergeObj` object, respectively.
         *
         * @tparam Stream The type of the output stream.
         * @param[out] out The output stream to write the `MergeObj` object to.
         * @param[in] merge_obj The `MergeObj` object to be written to the output stream.
         * @return The modified output stream.
         */
        template <class Stream>
        friend auto operator<<(Stream &out, const MergeObj &merge_obj) -> Stream & {
            out << "/" << merge_obj.xcoord() << ", " << merge_obj.ycoord() << "/";
            return out;
        }

        // /**
        //  * @brief minimum distance with
        //  *
        //  * @param[in] other
        //  * @return constexpr auto
        //  */
        // constexpr auto min_dist_change_with(MergeObj& other) {
        //     auto minDist = this->min_dist_with(other);
        //     auto mobj1 = this->enlarge_with(minDist);
        //     auto mobj2 = other.enlarge_with(minDist);
        //     other = mobj1.intersect_with(other);
        //     *this = mobj2.intersect_with(*this);
        //     return minDist;
        // }
    };
#pragma pack(pop)

    // template <typename U1, typename U2>  //
    // constexpr auto min_dist_change_merge(U1& lhs, U2& rhs) {
    //     if constexpr (std::is_scalar<U1>::value) {
    //         if constexpr (std::is_scalar<U2>::value) {
    //             return std::abs(lhs - rhs);
    //         } else {
    //             return rhs.min_dist_change_with(lhs);
    //         }
    //     } else {
    //         return lhs.min_dist_change_with(rhs);
    //     }
    // }

}  // namespace recti
