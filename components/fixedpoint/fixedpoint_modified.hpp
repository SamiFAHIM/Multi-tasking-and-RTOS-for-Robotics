/**
 * @file FixedPoint.hpp
 * @author Sylvain Mosnier (sylvin.mosnier@energymachines.com)
 * @brief Fixed Point class definition
 * @version 2
 * @date 2022-04-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef FIXED_POINT_HPP_
#define FIXED_POINT_HPP_
#include "sdkconfig.h"
#include <type_traits>
#include <cmath>
#include "miscellaneous.hpp"

class FixedPointBase
{
};

/**
 * @brief Class to handle fixe point basic arithmetic based on int32_t type
 * the integer part argment is used for multiplication and division optimisation
 * @details Naturally, the sum of the template parameters should not exceed the number of bit of the signed int32_t type, aka 31
 * @todo As division is using truncation instead of rounding, it could be worth it to add a few extra code to avoid that
 * @tparam I is the number of bits ahead the point, so it is equivalent to the integer part of the number
 *         It is mostly used as an hint for multiplication and division optimization
 * @tparam E is the number of bits behind the point, so if E==0, it,s like integer operation
 */
template <int I, int E>
class FixedPoint : FixedPointBase
{
public:
    // constraint on E
    static_assert((E >= 0) & (E < 31)); // E should be between 0 and 30 because bit 31 is sign bit
    static_assert((I >= 0) & (I < 31));
    static_assert((I + E) < 31);

    // typedef FixedPoint FixedPoint<I,E>;
    using self = FixedPoint;
    static constexpr int32_t factor = (1 << E);

    static constexpr self &MAX_VAL() { return self(1 << (I + E) + 1); } // if I = 5, E  = 1, MAX_VAL = (2-1) = 1 bit

    static constexpr self &MIN_VAL() { return -MAX_VAL(); }

    // template accessor to get template parameter value
    static constexpr int Ipart() { return I; }
    static constexpr int Fpart() { return E; }

    // constructor
    constexpr FixedPoint() = default; // default constructor M is set to 0

    // direct value constructor
    explicit constexpr FixedPoint(uint32_t d) : m(d) {}

    // default copy constructor
    constexpr FixedPoint(const self &d) = default;

    // default destructor
    ~FixedPoint() = default;

    // concept FixedPointClass = requires std::convertible_to<self, FixedPoint<J,E>>;
    /**
     * @brief Construct a new Fixed Point object from another one
     * @bug In the case where the new object has a lower number of fractionnal part bits, add proper rounding and not truncation
     * @tparam J number of bits of the integer part of the parameter
     * @tparam F number of bits of the fractional part of the parameter
     * @param d
     */
    template <int J, int F>
    constexpr FixedPoint(const FixedPoint<J, F> &d)
    {
        if constexpr (E == F)
        {
            m = d.getM();
        }
        else if constexpr (E > F)
        {
            m = (d.getM()) * (1 << (E - F));
        }
        else // E<F
        {
            // FIXME add rounding stuff here ( because it's truncated for now)
            // https://www.embeddedrelated.com/showarticle/1015.php
            m = (d.getM()) / (1 << (F - E));
        }
    }
    // constructor
    template <typename U>
        requires (std::integral<U> || std::floating_point<U>)
    constexpr FixedPoint(const U &d) : m(static_cast<int32_t>(d * factor)){};
    // cast
    explicit constexpr operator int() const { return static_cast<int>(m) / factor; }
    // operation
    template <int J, int F>
    constexpr self &operator+=(const FixedPoint<J, F> &x)
    {
        if constexpr (E == F)
        {
            m += x.getM();
        }
        else if constexpr (E > F)
        {
            m += (x.getM() * (1 << (E - F)));
        }
        else
        {
            m += (x.getM() / (1 << (F - E)));
        }
        return *this;
    }
    template <int J, int F>
    constexpr self &operator-=(const FixedPoint<J, F> &x)
    {
        if constexpr (E == F)
        {
            m -= x.getM();
        }
        else if constexpr (E > F)
        {
            m -= (x.getM() * (1 << (E - F)));
        }
        else
        {
            m -= (x.getM() / (1 << (F - E)));
        }
        return *this;
    }
    template <int J, int F>
    constexpr self &operator*=(const FixedPoint<J, F> &x)
    {
        if constexpr (((I + E) + (J + F)) < 31) // optimisation in the case that the product can fit directly in an int32
        {
            m *= x.getM();
            m /= x.factor;
        }
        else
        {
            int64_t res = (int64_t)(m)*x.getM(); // using int64_t is costly, but it is the general case
            m = (int32_t)(res / x.factor);
        }
        return *this;
    }
    template <int J, int F>
    constexpr self &operator/=(const FixedPoint<J, F> &x)
    {
        if constexpr (((I + E) + F < 31)) // optimisation if the product with the factor does not overflow
        {
            m *= x.factor; // thanks to the condition, this will not overflow
            m /= x.getM();
        }
        else
        {
            int64_t res = (int64_t)(m)*x.factor; // using int64_t is costly, but it is the general case
            m = (int32_t)(res / x.getM());
        }
        return *this;
    }

    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr self &operator+=(const U &x)
    {
        *this += self(x);
        return *this;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr self &operator-=(const U &x)
    {
        *this -= self(x);
        return *this;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr self &operator*=(const U &x)
    {
        *this *= self(x);
        return *this;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr self &operator/=(const U &x)
    {
        *this /= self(x);
        return *this;
    }

    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr self operator+(const self &y, const U &x)
    {
        return self(x) += y;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr self operator-(const self &y, const U &x)
    {
        return self(x) -= y;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr self operator*(const self &y, const U &x)
    {
        return self(x) *= y;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr self operator/(const self &y, const U &x)
    {
        return self(x) /= y;
    }

    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr self operator+(const U &y)
    {
        return self(y) += *this;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr self operator-(const U &y)
    {
        return self(-y) += *this;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr self operator*(const U &y)
    {
        return self(y) *= *this;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr self operator/(const U &y)
    {
        return self(*this) /= self(y);
    }

    // cast
    explicit constexpr operator float() const { return static_cast<float>(m) / factor; }
    explicit constexpr operator double() const { return static_cast<double>(m) / factor; }

    constexpr self operator-() const
    {
        return self(static_cast<uint32_t>((-this->m)));
    }

    // assignation operator
    /*
    FixedPoint<I,E> &operator=(const FixedPoint<I,E> &x)
    {
        m = x.m;
        return *this;
    }*/

    // trivial assignation operator
    constexpr self &operator=(const self &x) = default;

    // trivial move operator
    constexpr self &operator=(self &&x) = default;

    constexpr self &operator=(const self &&x)
    {
        m = std::move(x.getM());
        return *this;
    }
    // cast

    // abs value
    friend constexpr self abs(const self &x) { return (x.m < 0) ? -x : x; }

    /**
     * @brief Function based on fast exponentiation algorithm
     * @todo add a recursive template function for that
     * @param x
     * @param n
     * @return FixedPoint<I,E>
     */
    friend constexpr self pow(const self &x, uint32_t n)
    {
        int64_t res = static_cast<uint64_t>(x.getM());
        int64_t prod = x.factor; // equivalent to 1 in base E
        while (n > 0)
        {
            if ((n & 1) == 1)
            {
                prod *= res;      // save product if it appear in binary form of n
                prod /= x.factor; // scale the result
            }
            n >>= 1;         // devide n by 2
            res *= res;      // square value
            res /= x.factor; // scale value to E point size
        }
        return self(static_cast<uint32_t>(prod));
    }
    /**
     * @brief Exponentiation function for fixed point number : this function use double calculus,
     *          because it's really hard to do some exponentiation algorithm with fixed point number
     *          and to guarantee the accuracy of the result
     *
     * @tparam F
     * @param x the number
     * @param exp is the exponent
     * @return auto
     */
    template <int J, int F>
    friend auto pow(const self &x, const FixedPoint<J, F> &exp)
    {
        return self(pow(double(x), double(exp)));
    }

    /**
     * @brief Calculatre log2 of input (non-recursive) (no runtime test for negative values)
     * @details If input is equal to 0, it return (-E-1)
     *
     * @param x
     * @return constexpr int
     */
    // constexpr friend int log2(const self &x)
    //{
    //     // static_assert((x.getM()<=0),"X must be positive");
    //     int res = (-E - 1);
    //     if (x.getM() == 0)
    //     {
    //         return res;
    //     } // unlikely to happend but still
    //     int m = x.getM();
    //     while (m)
    //     {
    //         m /= 2;
    //         ++res;
    //     }
    //     return res;
    // }
    friend constexpr double sqrt(const self &x){
        return sqrt(double(x));
    }

    friend constexpr auto log2(const self &x)
    {
        // int res = (-E - 1);
        //  this is the correct format to store the result of log2 operation
        FixedPoint<std::max(misc::log2(I) + 1, 0), (31 - 1) - std::max((misc::log2(I) + 1), 0)> result(0);
        if (x.getM() == 0)
        {
            return (decltype(result))(-E - 1);
        }
        int32_t m = x.getM();
        while (m < x.factor)
        {
            m <<= 1;
            result -= 1;
        }
        while (m >= 2 * x.factor)
        {
            m >>= 1;
            result += 1;
        }
        auto z = m;
        auto b = result.factor >> 1;

        for (size_t i = 0; i < E; i++)
        {
            z = z * z >> E;
            if (z >= 2U << E)
            {
                z >>= 1;
                result += FixedPoint<misc::log2(I) + 1, (31 - 1) - (misc::log2(I) + 1)>(static_cast<uint32_t>(b));
            }
            b >>= 1;
        }
        return result;
        /*
        int32_t log2fix(uint32_t x, size_t precision)
        {
            // This implementation is based on Clay. S. Turner's fast binary logarithm
            // algorithm[1].

            int32_t b = 1U << (precision - 1);
            int32_t y = 0;

            if (precision < 1 || precision > 31)
            {
                errno = EINVAL;
                return INT32_MAX; // indicates an error
            }

            if (x == 0)
            {
                return INT32_MIN; // represents negative infinity
            }
            // rescale x between 1 and 2
            while (x < 1U << precision)
            {
                x <<= 1;
                y -= 1U << precision;
            }

            while (x >= 2U << precision)
            {
                x >>= 1;
                y += 1U << precision;
            }

            uint64_t z = x;

            for (size_t i = 0; i < precision; i++)
            {
                z = z * z >> precision;
                if (z >= 2U << precision)
                {
                    z >>= 1;
                    y += b ;
                }
                b >>= 1;
            }

            return y;
        }*/
    }

    friend constexpr auto floor(const self &x)
    {
        auto r = int(x);
        return r - (x.getM() < 0) * (1 - (r * x.factor == x.getM()));
    }

    friend constexpr auto ceil(const self &x)
    {
        auto r = int(x);
        return r + (x.getM() > 0) * (1 - (r * x.factor == x.getM()));
    }
    // comparison operators
    constexpr bool operator==(const self &x) { return m == x.getM(); }
    constexpr bool operator!=(const self &x) { return m != x.getM(); }
    constexpr bool operator>(const self &x) { return m > x.getM(); }
    constexpr bool operator<(const self &x) { return m < x.getM(); }
    constexpr bool operator>=(const self &x) { return m >= x.getM(); }
    constexpr bool operator<=(const self &x) { return m <= x.getM(); }

    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr bool operator==(const self &x, const U &y)
    {
        return x == self(y);
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr bool operator!=(const self &x, const U &y)
    {
        return self(y) != x;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr bool operator>(const self &x, const U &y)
    {
        return self(y) > x;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr bool operator<(const self &x, const U &y)
    {
        return self(y) < x;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr bool operator>=(const self &x, const U &y)
    {
        return self(y) >= x;
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    friend constexpr bool operator<=(const self &x, const U &y)
    {
        return self(y) <= x;
    }

    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr bool operator==(const U &y)
    {
        return *this == self(y);
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr bool operator!=(const U &y)
    {
        return *this != self(y);
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr bool operator>(const U &y)
    {
        return *this > self(y);
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr bool operator<(const U &y)
    {
        return *this < self(y);
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr bool operator>=(const U &y)
    {
        return *this >= self(y);
    }
    template <typename U>
        requires((std::convertible_to<U, self>) && (!std::derived_from<U, FixedPointBase>))
    constexpr bool operator<=(const U &y)
    {
        return *this <= self(y);
    }

    // cross-fixedpoint operations
    // all validated
    template <int J, int F>
    friend constexpr auto operator+(const self &x, const FixedPoint<J, F> &y)
    {
        if constexpr ((I >= J) && (E >= F))
        {
            return FixedPoint<std::min<int>(I, 30 - F), F>(x) += y;
        }
        else if constexpr ((I < J) && (E >= F))
        {
            return FixedPoint<J, F>(x) += y;
        }
        else if constexpr ((I >= J) && (E < F))
        {
            return self(y) += x;
        }
        else // if constexpr ((I < J) && (E < F))
        {
            return FixedPoint<std::min<int>(J, 30 - E), E>(y) += x;
        }
    }
    template <int J, int F>
    friend constexpr auto operator-(const self &x, const FixedPoint<J, F> &y)
    {
        if constexpr ((I >= J) && (E >= F))
        {
            return FixedPoint<std::min<int>(I, 30 - F), F>(x) -= y;
        }
        else if constexpr ((I < J) && (E >= F))
        {
            return FixedPoint<J, F>(x) -= y;
        }
        else if constexpr ((I >= J) && (E < F))
        {
            return self(x) -= y;
        }
        else // if constexpr ((I < J) && (E < F))
        {
            return FixedPoint<std::min<int>(J, 30 - E), E>(x) -= y;
        }
    }
    template <int J, int F>
    friend constexpr auto operator*(const self &x, const FixedPoint<J, F> &y)
    {
        // X has more decimale than Y :
        // product is difficult to predict but we can't expect more accuracy thant the less accurate of the 2 numbers
        if constexpr (E > F)
        {                                                                                 // we need to keep track of I and J to do the product, and then return with the appropriate integer part
            return FixedPoint<std::min<int>(I + J, 30 - F), F>(FixedPoint<J, F>(y) *= x); // copy constructor of y
        }
        else
        {                                                                     // we need to keep track of I and J to do the product, and then return with the approcpirate integer part
            return FixedPoint<std::min<int>(I + J, 30 - E), E>(self(x) *= y); // copy constructor on Y and product with x
        }
    }
    // for division, the size of the integer part can not be predicted, because it may be smaller than I, or larger depending on the value of the denominator
    template <int J, int F>
    friend constexpr auto operator/(const self &x, const FixedPoint<J, F> &y)
    { // validated, may be unstable depending on what is divided...
        if constexpr (E > F)
        {
            return FixedPoint<std::min<int>(I, 30 - F), F>(self(x) /= y); // cast x to F at the end to keep maximum precision
        }
        else
        {
            return self(x) /= y; // cast y to E (not possible to simpli avoid 2nd constructor call as division is not commutative)
        }
    }

    constexpr auto getM() const { return m; };

    /**
     * @brief Process convolution
     *
     * @tparam ITR1 iterator type of coefficient array (this might just be an iterator)
     * @tparam ITR2 iterator type of value array :
     * @tparam unrollfactor
     * @tparam length
     * @param begin1
     * @param begin2
     * @return auto: the return type is optimized to avoid precision loss
     */
    // change template definition to in clude coefs inside
    // template <std::size_t N, typename ITR, size_t unrollfactor>
    // friend auto convolution(self (&coefs)[N], ITR values)
    //{
    //    // this is compile time calculus to get the best size for the accumulator and not loose any precision
    //    constexpr auto clog2alpha = misc::force_consteval<ceil(log2(std::max(misc::accumulate<self>(coefs, misc::positiv_relu<self>), -misc::accumulate<self>(coefs, misc::negativ_relu<self>))))>;
    //    constexpr auto sum_prec = E + IRT::value_type::Fpart() + clog2alpha; // this is the optimal number of bit of the accumulator
    //    //  constexpr auto alpha = misc::accumulate<Coefs_t>(coefs,misc::abs<Coefs_t>);
    //    //  constexpr auto bprim = log2((1<<(32 - 1) - 1))/double(max_element<double>(coefs, abs<Coefs_t>)))); // because the division may overflow int, so we use double
    //    //  calculating accumulator size
    //}

private:
    int32_t m;
};

// test code here
/*
FixedReal x(0);
FixedReal y(-1);
FixedReal z(-0.0625);
FixedReal t = z+y;
printf("z : %f, y : %f , t : %f, abs(t) :%f\n",(double)z,(double)y,(double)t,(double)abs(t));
for (int32_t i=0; i < 100; ++i) {
x += z;
}
*/
/*
    FixedPoint<5, 5> x(-1);
    FixedPoint<5, 5> x1(0.5);
    x1-=10;
    FixedPoint<5, 10> y(-2);     //=1/64
    FixedPoint<10, 5> z(64.0625); // 2**6 +2**-5
    printf("x : %f, x1 : %f, y : %f, x/y : %f, 1/y : %f,x-y : %f\n", (double)x, (double)x1, (double)y, (double)(x / y), (double)(FixedPoint<10,10>((int32_t)1) / y), (double)(x - y));
    printf("x*x : %f, x1/x1 : %f, y*y : %f, y/x : %f, z : %f,z5 : %f\n", (double)(x * x), (double)(x1 / x1), (double)(y * y), (double)(y / x), (double)z, (double)(FixedPoint<10,10>(z)));
    while (true)
    {
        vTaskDelay(100);
        printf("coucou\n");
    }
*/
#endif /*FIXED_POINT_HPP_*/