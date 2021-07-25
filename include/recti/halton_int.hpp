#pragma once

#include <cmath>
#include <vector>

namespace recti {

    /**
     * @brief van der Corput sequence
     *
     * @param[in] k
     * @param[in] base
     * @return unsigned
     */
    inline auto vdc(unsigned k, unsigned base = 2, unsigned scale = 10) noexcept -> unsigned {
        auto vdc = 0;
        auto factor = unsigned(std::pow(base, scale));
        while (k != 0) {
            factor /= base;
            auto remainder = k % base;
            k /= base;
            vdc += remainder * factor;
        }
        return vdc;
    }

    /**
     * @brief van der Corput sequence generator
     *
     */
    class vdcorput {
      private:
        unsigned _count{0};
        unsigned _base;
        unsigned _scale;

      public:
        /**
         * @brief Construct a new vdcorput object
         *
         * @param[in] base
         */
        constexpr vdcorput(unsigned base = 2, unsigned scale = 10) noexcept
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
        constexpr auto reseed(unsigned seed) noexcept -> void { this->_count = seed; }
    };

    /**
     * @brief Halton sequence generator
     *
     */
    class halton {
      private:
        vdcorput _vdc0;
        vdcorput _vdc1;

      public:
        /**
         * @brief Construct a new halton object
         *
         * @param[in] base
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
        constexpr auto reseed(unsigned seed) noexcept -> void {
            this->_vdc0.reseed(seed);
            this->_vdc1.reseed(seed);
        }
    };

}  // namespace recti
