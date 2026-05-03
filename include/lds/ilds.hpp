#pragma once

#include <array>
#include <cmath>

namespace ilds {

    using std::array;

    // Constants for magic numbers
    /**
     * @brief Default number of digits for the van der Corput sequence
     *
     * This value determines the precision/scale of the integer Halton sequence.
     * Default is 10 digits.
     */
    constexpr unsigned int DEFAULT_SCALE = 10;
    constexpr unsigned int MAX_REVERSE_BITS = 64;

    /**
     * @brief van der Corput sequence generator
     *
     * Implementation based on pre-calculating the scale factor.
     *
     */
    template <unsigned long Base = 2>
    class VdCorput {
        unsigned long _count;  ///< Current count in the sequence
        std::array<unsigned long, MAX_REVERSE_BITS> factor_lst;  ///< Precomputed scale factors for each digit
        static_assert(MAX_REVERSE_BITS >= sizeof(unsigned long) * 8, "MAX_REVERSE_BITS must be at least the number of bits in unsigned long");

      public:
        /**
         * @brief Construct a new VdCorput object
         *
         * @param[in] scale The number of digits (default: 10)
         */
        constexpr explicit VdCorput(unsigned int scale = DEFAULT_SCALE)
            : _count{0},  factor_lst{} {
            unsigned long factor = static_cast<unsigned long>(std::pow(Base, scale));
            for (unsigned int i = 0; i < MAX_REVERSE_BITS; ++i) {
                factor /= Base;
                this->factor_lst[i] = factor;
            }
        }

        /**
         * @brief Increments count and calculates the next value in the sequence.
         *
         * @return unsigned long
         */
        [[nodiscard]] constexpr auto pop() -> unsigned long {
            unsigned long count = ++this->_count;
            unsigned long reslt = 0;
            unsigned int idx = 0;
            while (count != 0) {
                const unsigned long remainder = count % Base;
                count /= Base;
                reslt += remainder * this->factor_lst[idx];
                ++idx;
            }
            return reslt;
        }

        /**
         * @brief Resets the state of the sequence generator.
         *
         * @param[in] seed
         */
        constexpr auto reseed(const unsigned long& seed) -> void {
            this->_count = seed;
        }

        VdCorput(VdCorput&&) noexcept = delete;
        VdCorput& operator=(VdCorput&&) noexcept = delete;
    };

    /**
     * @brief Halton sequence generator
     *
     * @verbatim
     *     Integer Halton([2,3], [2,2]):
     *     pop() -> [1, 4]   (VdC_i(2,2,2), VdC_i(2,3,2))
     *     pop() -> [2, 5]   (next in each sequence)
     *     ...
     * @endverbatim
     */
    template <unsigned long Base1, unsigned long Base2>
    class Halton {
        VdCorput<Base1> vdc0;
        VdCorput<Base2> vdc1;

      public:
        /**
         * @brief Construct a new Halton object
         *
         * Constructs a Halton sequence generator with the specified bases and scale values.
         *
         * @param[in] scale array of two unsigned int values representing the number of digits for
         * each generator
         */
        constexpr explicit Halton(const std::array<unsigned int, 2>& scale)
            : vdc0(scale[0]), vdc1(scale[1]) {}

        /**
         * @brief Generate the next point in the Halton sequence
         *
         * Returns the next point in the Halton sequence as an array of two unsigned long values.
         *
         * @return array<unsigned long, 2> the next point in the sequence
         */
        constexpr auto pop() -> array<unsigned long, 2> {  //
            return {this->vdc0.pop(), this->vdc1.pop()};
        }

        /**
         * @brief Reset the state of the Halton sequence generator
         *
         * Resets the state of the sequence generator to a specific seed value.
         *
         * @param[in] seed the seed value to reset the sequence generator to
         */
        constexpr auto reseed(const unsigned long& seed) -> void {
            this->vdc0.reseed(seed);
            this->vdc1.reseed(seed);
        }
    };

}  // namespace ilds
