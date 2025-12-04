#pragma once

#include <cmath>
#include <vector>

namespace recti {

    /**
     * @brief van der Corput sequence calculator
     *
     * Computes the van der Corput sequence value for a given number. The van der
     * Corput sequence is a low-discrepancy sequence over the unit interval,
     * which is useful for quasi-random sampling and numerical integration.
     *
     * @param[in] num The number to calculate the van der Corput sequence for
     * @param[in] base The base of the van der Corput sequence (default: 2)
     * @param[in] scale The scale factor determining precision (default: 10)
     * @return unsigned The van der Corput sequence value
     */
    inline auto vdc(const unsigned num, unsigned base = 2, unsigned scale = 10) noexcept
        -> unsigned {
        auto vdc = 0U;
        auto factor = unsigned(std::pow(base, scale));
        auto temp_num = num;
        while (temp_num != 0) {
            auto remainder = temp_num % base;
            factor /= base;
            temp_num /= base;
            vdc += remainder * factor;
        }
        return vdc;
    }

    /**
     * @brief van der Corput sequence generator
     *
     * This class generates van der Corput sequence values incrementally.
     * The van der Corput sequence is a low-discrepancy sequence that provides
     * uniform coverage of the unit interval, making it useful for quasi-random
     * sampling, Monte Carlo methods, and numerical integration.
     */
    class Vdcorput {
      private:
        unsigned _count{0};
        unsigned _base;
        unsigned _scale;

      public:
        /**
         * @brief Construct a new Vdcorput object
         *
         * @param[in] base
         * @param[in] scale
         */
        constexpr explicit Vdcorput(unsigned base = 2, unsigned scale = 10) noexcept
            : _base{base}, _scale{scale} {}

        /**
         * @brief
         *
         * @return double
         */
        auto operator()() noexcept -> unsigned {
            this->_count += 1;
            return vdc(this->_count, this->_base, this->_scale);
        }

        /**
         * @brief
         *
         * @param[in] seed
         */
        auto reseed(unsigned seed) noexcept -> void { this->_count = seed; }
    };

    /**
     * @brief Halton sequence generator
     *
     */
    class halton {
      private:
        Vdcorput _vdc0;
        Vdcorput _vdc1;

      public:
        /**
         * @brief Construct a new halton object
         *
         * @param[in] base
         * @param[in] scale
         */
        explicit constexpr halton(const unsigned base[], const unsigned scale[]) noexcept
            : _vdc0(base[0], scale[0]), _vdc1(base[1], scale[1]) {}

        /**
         * @brief
         *
         * @return auto
         */
        auto operator()() -> std::vector<unsigned> { return {this->_vdc0(), this->_vdc1()}; }

        /**
         * @brief
         *
         * @param[in] seed
         */
        auto reseed(unsigned seed) noexcept -> void {
            this->_vdc0.reseed(seed);
            this->_vdc1.reseed(seed);
        }
    };

}  // namespace recti
