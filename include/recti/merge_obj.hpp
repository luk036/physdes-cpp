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
     * @tparam T1
     * @tparam T2
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
         * @param[in] xcoord x coordinate
         * @param[in] ycoord y coordinate
         */
        constexpr MergeObj(T1 &&xcoord, T2 &&ycoord) noexcept
            : Point<T1, T2>{std::move(xcoord), std::move(ycoord)} {}

        /**
         * @brief Construct a new MergeObj object
         *
         * @param[in] xcoord x coordinate
         * @param[in] ycoord y coordinate
         */
        static constexpr auto construct(T1 &&xcoord, T2 &&ycoord) -> MergeObj {
            return MergeObj{xcoord + ycoord, xcoord - ycoord};
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
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        constexpr auto operator==(const MergeObj<U1, U2> &rhs) const -> bool {
            return Point<T1, T2>::operator==(rhs);
        }

        /**
         * @brief Not equal to
         *
         * @tparam U1
         * @tparam U2
         * @param[in] rhs
         * @return true
         * @return false
         */
        template <typename U1, typename U2>
        constexpr auto operator!=(const MergeObj<U1, U2> &rhs) const -> bool {
            return Point<T1, T2>::operator!=(rhs);
        }

        /**
         * @brief Add a vector (translation)
         *
         * @tparam U
         * @param[in] rhs
         * @return MergeObj&
         */
        template <typename U> constexpr auto operator+=(const Vector2<U> &rhs) -> MergeObj & {
            this->get_xcoord() += rhs.x() + rhs.y();
            this->get_ycoord() += rhs.x() - rhs.y();  // notice here!
            return *this;
        }

        /**
         * @brief Substract a vector (translation)
         *
         * @tparam U
         * @param[in] rhs
         * @return MergeObj&
         */
        template <typename U> constexpr auto operator-=(const Vector2<U> &rhs) -> MergeObj & {
            this->get_xcoord() -= rhs.x() + rhs.y();
            this->get_ycoord() -= rhs.x() - rhs.y();  // notice here!
            return *this;
        }

        /**
         * @brief Add
         *
         * @tparam U
         * @param[in] xcoord
         * @param[in] ycoord
         * @return Vector2<T>
         */
        template <typename U>  //
        friend constexpr auto operator+(MergeObj lhs, const Vector2<U> &rhs) -> MergeObj {
            return lhs += rhs;
        }

        /**
         * @brief Substract
         *
         * @tparam U
         * @param[in] xcoord
         * @param[in] ycoord
         * @return Vector2<T>
         */
        template <typename U>  //
        friend constexpr auto operator-(MergeObj lhs, const Vector2<U> &rhs) -> MergeObj {
            return lhs -= rhs;
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto overlaps(const MergeObj<U1, U2> &other) const -> bool {
            return overlap(this->xcoord(), other.xcoord())
                   && overlap(this->ycoord(), other.ycoord());
        }

        /**
         * @brief intersection
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto intersect_with(const MergeObj<U1, U2> &other) const {
            auto xcoord = intersection(this->xcoord(), other.xcoord());
            auto ycoord = intersection(this->ycoord(), other.ycoord());
            return MergeObj<decltype(xcoord), decltype(ycoord)>{std::move(xcoord),
                                                                std::move(ycoord)};
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
         */
        template <typename U1, typename U2>  //
        constexpr auto min_dist_with(const MergeObj<U1, U2> &other) const {
            return std::max(min_dist(this->xcoord(), other.xcoord()),
                            min_dist(this->ycoord(), other.ycoord()));
            // auto min_dist_x = min_dist(this->xcoord(), other.xcoord());
            // auto min_dist_y = min_dist(this->ycoord(), other.ycoord());
            // return min_dist_x > min_dist_y ? min_dist_x : min_dist_y;
        }

        template <typename R>  //
        friend constexpr auto enlarge(const MergeObj &lhs, const R &alpha) {
            auto xcoord = enlarge(lhs.xcoord(), alpha);
            auto ycoord = enlarge(lhs.ycoord(), alpha);
            return MergeObj<decltype(xcoord), decltype(ycoord)>{std::move(xcoord),
                                                                std::move(ycoord)};
        }

        /**
         * @brief overlap
         *
         * @tparam U1
         * @tparam U2
         * @param[in] other
         * @return true
         * @return false
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
         * @brief
         *
         * @tparam T1
         * @tparam T2
         * @tparam Stream
         * @param[out] out
         * @param[in] p
         * @return Stream&
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
