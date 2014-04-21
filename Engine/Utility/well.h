// Copyright (c) Sergiu Dotenco 2010, 2011, 2012
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/**
 * @brief Implementation of the Well Equidistributed Long-period Linear (WELL)
 *        pseudo-random number generator.
 * @file well.hpp
 */

#ifndef WELL_HPP
#define WELL_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/bitand.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/minus.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/ref.hpp>
#include <boost/static_assert.hpp>
#include <boost/throw_exception.hpp>

//! @cond hide_private

namespace Detail {

template<class UIntType, unsigned N>
struct Left
{
    static UIntType shift(UIntType a)
    {
        return a << N;
    }
};

template<class UIntType, unsigned N>
struct Right
{
    static UIntType shift(UIntType a)
    {
        return a >> N;
    }
};

template<int N, class UIntType>
inline UIntType shift(UIntType a)
{
    return boost::mpl::if_c<(N < 0),
                Left<UIntType, -N>,
                Right<UIntType, N>
            >::type::shift(a);
}

/**
 * @name Transformation matrices @f$M0,\dotsc,M6@f$ from Table I
 * @{
 */

struct M0
{
    template<class T>
    static T transform(T)
    {
        return T(0);
    }
};

struct M1
{
    template<class T>
    static T transform(T x)
    {
        return x;
    }
};

template<int N>
struct M2
{
    template<class T>
    static T transform(T x)
    {
        return shift<N>(x);
    }
};

template<int N>
struct M3
{
    template<class T>
    static T transform(T x)
    {
        return x ^ shift<N>(x);
    }
};

template<boost::uint_least32_t a>
struct M4
{
    template<class T>
    static T transform(T x)
    {
        T result = x >> 1;

        if ((x & 1) == 1)
            result ^= a;

        return result;
    }
};

template<int N, boost::uint_least32_t b>
struct M5
{
    template<class T>
    static T transform(T x)
    {
        return x ^ (shift<N>(x) & b);
    }
};

template
<
    std::size_t w,
    boost::uint_least32_t q,
    boost::uint_least32_t a,
    boost::uint_least32_t ds,
    boost::uint_least32_t dt
>
struct M6
{
    template<class T>
    static T transform(T x)
    {
        T result = ((x << q) ^ (x >> (w - q))) & ds;

        if ((x & dt) != 0)
            result ^= a;

        return result;
    }
};

//! @}

/**
 * Conditional expression of type (r & (r - 1)) == 0 which allows to check
 * whether a number @f$r@f$ is of type @f$2^n@f$.
 */
typedef boost::mpl::equal_to<
            boost::mpl::bitand_<
                boost::mpl::_,
                boost::mpl::minus<boost::mpl::_, boost::mpl::int_<1>
            >
        >,
        boost::mpl::int_<0>
    > IsPowerOfTwo;

template<class UIntType, UIntType r>
struct Power2Modulo
{
    typedef typename boost::mpl::apply<
            IsPowerOfTwo,
            boost::mpl::integral_c<UIntType, r>
        >::type type;

    BOOST_STATIC_ASSERT(type::value);

    template<class T>
    static T calc(T value)
    {
        return value & (r - 1);
    }
};

template<class UIntType, UIntType r>
struct GenericModulo
{
    /**
     * @brief Determines @a value modulo @a r.
     *
     * @pre value >= 0 and value < 2 * r
     * @post value >= 0 and value < r
     */
    template<class T>
    static T calc(T value)
    {
        BOOST_STATIC_ASSERT(!std::numeric_limits<UIntType>::is_signed);
        assert(value < 2 * r);

        if (value >= r)
            value -= r;

        return value;
    }
};

template<class UIntType, UIntType r>
struct Modulo
{
    typedef typename boost::mpl::apply<
            IsPowerOfTwo,
            boost::mpl::integral_c<UIntType, r>
        >::type rIsPowerOfTwo;

    static UIntType calc(UIntType value)
    {
        // Use the bitwise AND for power 2 modulo arithmetic, or subtraction
        // otherwise. Subtraction is about two times faster than direct modulo
        // calculation.
        return boost::mpl::if_<
                    rIsPowerOfTwo,
                        Power2Modulo<UIntType, r>,
                        GenericModulo<UIntType, r>
                >::type::calc(value);
    }
};

template<boost::uint_least32_t b, boost::uint_least32_t c>
struct MatsumotoKuritaTempering
{
    template<std::size_t r, class UIntType, std::size_t N>
    static UIntType apply(UIntType x, UIntType (&)[N], std::size_t)
    {
        x ^= (x << 7) & b;
        x ^= (x << 15) & c;

        return x;
    }
};

template<boost::uint_least32_t mask>
struct HaraseTempering
{
    template<std::size_t r, class UIntType, std::size_t N>
    static UIntType apply(UIntType x, UIntType (&s)[N], std::size_t m2)
    {
        return x ^ (s[Modulo<UIntType, r>::calc(m2 + 1)] & mask);
    }
};

struct NoTempering
{
    template<std::size_t r, class UIntType, std::size_t N>
    static UIntType apply(UIntType x, UIntType (&)[N], std::size_t)
    {
        return x;
    }
};

} // namespace Detail

//! @endcond

/**
 * @brief Well Equidistributed Long-period Linear (WELL) pseudo-random number
 *        generator.
 *
 * The implementation is based on the "Improved Long-Period Generators Based on
 * Linear Recurrences Modulo 2" paper by Francois Panneton, Pierre L'Ecuyer and
 * Makoto Matsumoto from ACM Transactions on Mathematical Software, 32 (1,
 * March) 2006, pp. 1-16.
 *
 * @tparam UIntType The unsigned integer type.
 * @tparam w Word size.
 * @tparam r State size.
 */
template
<
    class UIntType,
    std::size_t w,
    std::size_t r,
    std::size_t p,
    std::size_t m1,
    std::size_t m2,
    std::size_t m3,
    class T0,
    class T1,
    class T2,
    class T3,
    class T4,
    class T5,
    class T6,
    class T7,
    class Tempering // mpl pluggable
>
class Well
{
    BOOST_STATIC_ASSERT(!std::numeric_limits<UIntType>::is_signed);
    BOOST_STATIC_ASSERT(w <=
            static_cast<std::size_t>(std::numeric_limits<UIntType>::digits));
    BOOST_STATIC_ASSERT(r > 0 && p < w);
    BOOST_STATIC_ASSERT(m1 > 0 && m1 < r);
    BOOST_STATIC_ASSERT(m2 > 0 && m2 < r);
    BOOST_STATIC_ASSERT(m3 > 0 && m3 < r);

public:
    //! The unsigned integer type.
    typedef UIntType result_type;

    //! Word size.
    BOOST_STATIC_CONSTANT(std::size_t, word_size = w);
    //! State size.
    BOOST_STATIC_CONSTANT(std::size_t, state_size = r);
    //! Number of mask bits.
    BOOST_STATIC_CONSTANT(std::size_t, mask_bits = p);
    //! Default seed value.
    BOOST_STATIC_CONSTANT(UIntType, default_seed = 5489U);

    /**
     * @brief Initializes the class using the specified seed @a value.
     *
     * @param value The seed value to be used for state initialization.
     */
    explicit Well(result_type value = default_seed)
    {
        seed(value);
    }

    template<class InputIterator>
    Well(InputIterator& first, InputIterator last)
    {
        seed(first, last);
    }

    template<class Generator>
    explicit Well(Generator& g)
    {
        seed(g);
    }

    template<class Generator>
    void seed(Generator& g)
    {
        // Ensure std::generate_n doesn't copy the generator g by using
        // boost::reference_wrapper
        std::generate_n(state_, state_size, boost::ref(g));
    }

    void seed(result_type value = default_seed)
    {
        if (value == 0U)
            value = default_seed;

        state_[0] = value;

        std::size_t i = 1;
        UIntType *const s = state_;

        // Same generator used to seed Mersenne twister
        for ( ; i != state_size; ++i)
            s[i] = (1812433253U * (s[i - 1] ^ (s[i - 1] >> (w - 2))) + i);

        index_ = i;
    }

    template<class InputIterator>
    void seed(InputIterator& first, InputIterator last)
    {
        index_ = 0;
        std::size_t i = 0;

        for ( ; i != state_size && first != last; ++i, ++first)
            state_[i] = *first;

        if (first == last && i != state_size)
            BOOST_THROW_EXCEPTION
                (std::invalid_argument("Seed sequence too short"));
    }

    /**
     * @brief Generates a random number.
     */
    result_type operator()()
    {
        const UIntType upper_mask = ~0U << p;
        const UIntType lower_mask = ~upper_mask;

        // v[i,j] = state[(r-i+j) mod r]
        std::size_t i = index_;
        // Equivalent to r-i but allows to avoid negative values in the
        // following two expressions
        std::size_t j = i + r;
        std::size_t k = mod(j - 1); // [i,r-1]
        std::size_t l = mod(j - 2); // [i,r-2]

        std::size_t im1 = i + m1;
        std::size_t im2 = i + m2;
        std::size_t im3 = i + m3;

        UIntType z0, z1, z2, z3, z4;

        z0 = (state_[k] & upper_mask) | (state_[l] & lower_mask);
        z1 = T0::transform(state_[i]) ^
             T1::transform(state(im1));
        z2 = T2::transform(state(im2)) ^
             T3::transform(state(im3));
        z3 = z1 ^ z2;
        z4 = T4::transform(z0) ^ T5::transform(z1) ^
             T6::transform(z2) ^ T7::transform(z3);

        state_[i] = z3; // v[i+1,1]
        state_[k] = z4; // v[i+1,0]

        index_ = k;

        return Tempering::template apply<r>(z4, state_, im2);
    }

    result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const
    {
        return 0U;
    }

    result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const
    {
        return ~0U >> (std::numeric_limits<UIntType>::digits - w);
    }

    void discard(boost::ulong_long_type z)
    {
        while (z-- > 0) {
            (*this)();
        }
    }

    /**
     * @brief Compares the state of two generators for equality.
     */
    friend bool operator==(const Well& lhs, const Well& rhs)
    {
        for (std::size_t i = 0; i != state_size; ++i)
            if (lhs.compute(i) != rhs.compute(i))
                return false;

        return true;
    }

    /**
     * @brief Compares the state of two generators for inequality.
     */
    friend bool operator!=(const Well& lhs, const Well& rhs)
    {
        return !(lhs == rhs);
    }

    /**
     * @brief Writes the state to the specified stream.
     */
    template<class E, class T>
    friend std::basic_ostream<E, T>&
        operator<<(std::basic_ostream<E, T>& out, const Well& well)
    {
        E space = out.widen(' ');

        for (std::size_t i = 0; i != state_size; ++i)
            out << well.compute(i) << space;

        return out;
    }

    /**
     * @brief Reads the generator state from the specified input stream.
     */
    template<class E, class T>
    friend std::basic_istream<E, T>&
        operator>>(std::basic_istream<E, T>& in, Well& well)
    {
        for (std::size_t i = 0; i != state_size; ++i)
            in >> well.state_[i] >> std::ws;

        well.index_ = state_size;

        return in;
    }

private:
    template<class T>
    static T mod(T value)
    {
        return Detail::Modulo<T, r>::calc(value);
    }

    UIntType state(std::size_t index) const
    {
        return state_[mod(index)];
    }

    UIntType compute(std::size_t index) const
    {
        return state_[(index_ + index + r) % r];
    }

    UIntType state_[r];
    std::size_t index_;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
template<class UIntType, std::size_t w, std::size_t r, std::size_t p,
    std::size_t m1, std::size_t m2, std::size_t m3, class T0, class T1,
    class T2, class T3, class T4, class T5, class T6, class T7, class Tempering>
const std::size_t Well<UIntType, w, r, p, m1, m2, m3, T0, T1, T2, T3, T4, T5,
      T6, T7, Tempering>::word_size;
template<class UIntType, std::size_t w, std::size_t r, std::size_t p,
    std::size_t m1, std::size_t m2, std::size_t m3, class T0, class T1,
    class T2, class T3, class T4, class T5, class T6, class T7, class Tempering>
const std::size_t Well<UIntType, w, r, p, m1, m2, m3, T0, T1, T2, T3, T4, T5,
      T6, T7, Tempering>::state_size;
template<class UIntType, std::size_t w, std::size_t r, std::size_t p,
    std::size_t m1, std::size_t m2, std::size_t m3, class T0, class T1,
    class T2, class T3, class T4, class T5, class T6, class T7, class Tempering>
const std::size_t Well<UIntType, w, r, p, m1, m2, m3, T0, T1, T2, T3, T4, T5,
      T6, T7, Tempering>::mask_bits;
template<class UIntType, std::size_t w, std::size_t r, std::size_t p,
    std::size_t m1, std::size_t m2, std::size_t m3, class T0, class T1,
    class T2, class T3, class T4, class T5, class T6, class T7, class Tempering>
const UIntType Well<UIntType, w, r, p, m1, m2, m3, T0, T1, T2, T3, T4, T5, T6,
      T7, Tempering>::default_seed;
#endif // BOOST_NO_INCLASS_MEMBER_INITIALIZATION

namespace Detail {

/**
 * @name Base definitions with pluggable tempering method
 * @{
 */

template
<
    class UIntType,
    std::size_t w,
    std::size_t r,
    std::size_t p,
    std::size_t m1,
    std::size_t m2,
    std::size_t m3,
    class T0,
    class T1,
    class T2,
    class T3,
    class T4,
    class T5,
    class T6,
    class T7
>
struct Well_quoted
{
    template<class T>
    struct apply
    {
        typedef Well<UIntType, w, r, p, m1, m2, m3, T0, T1, T2, T3, T4, T5, T6,
            T7, T> type;
    };
};

typedef Well_quoted<boost::uint32_t, 32, 16, 0, 13, 9, 5,
    M3<-16>, M3<-15>, M3<11>, M0, M3<-2>, M3<-18>, M2<-28>,
    M5<-5, 0xda442d24> > Well512a_base;

typedef Well_quoted<boost::uint32_t, 32, 17, 23, 13, 11, 10,
    M3<-13>, M3<-15>, M1, M2<-21>, M3<-13>, M2<1>, M0, M3<11> >
    Well521a_base;

typedef Well_quoted<boost::uint32_t, 32, 17, 23, 11, 10, 7,
    M3<-21>, M3<6>, M0, M3<-13>, M3<13>, M2<-10>, M2<-5>, M3<13> >
    Well521b_base;

typedef Well_quoted<boost::uint32_t, 32, 19, 1, 16, 15, 14,
    M3<19>, M3<11>, M3<-14>, M1, M3<18>, M1, M0, M3<-5> > Well607a_base;

typedef Well_quoted<boost::uint32_t, 32, 19, 1, 16, 18, 13,
    M3<-18>, M3<-14>, M0, M3<18>, M3<-24>, M3<5>, M3<-1>, M0>
    Well607b_base;

typedef Well_quoted<boost::uint32_t, 32, 25, 0, 14, 18, 17,
    M1, M3<-15>, M3<10>, M3<-11>, M3<16>, M2<20>, M1, M3<-28> >
    Well800a_base;

typedef Well_quoted<boost::uint32_t, 32, 25, 0, 9, 4, 22,
    M3<-29>, M2<-14>, M1, M2<19>, M1, M3<10>, M4<0xd3e43ffd>, M3<-25> >
    Well800b_base;

typedef Well_quoted<boost::uint32_t, 32, 32, 0, 3, 24, 10,
    M1, M3<8>, M3<-19>, M3<-14>, M3<-11>, M3<-7>, M3<-13>, M0>
    Well1024a_base;

typedef Well_quoted<boost::uint32_t, 32, 32, 0, 22, 25, 26,
    M3<-21>, M3<17>, M4<0x8bdcb91e>, M3<15>, M3<-14>, M3<-21>, M1, M0>
    Well1024b_base;

typedef Well_quoted<boost::uint32_t, 32, 624, 31, 70, 179, 449,
    M3<-25>, M3<27>, M2<9>, M3<1>, M1, M3<-9>, M3<-21>, M3<21> >
    Well19937a_base;

typedef Well_quoted<boost::uint32_t, 32, 624, 31, 203, 613, 123,
    M3<7>, M1, M3<12>, M3<-10>, M3<-19>, M2<-11>, M3<4>, M3<-10> >
    Well19937b_base;

typedef Well_quoted<boost::uint32_t, 32, 679, 27, 151, 327, 84,
    M1, M3<-26>, M3<19>, M0, M3<27>, M3<-11>,
    M6<32, 15, 0x86a9d87e, 0xffffffef, 0x00200000>, M3<-16> >
    Well21701a_base;

typedef Well_quoted<boost::uint32_t, 32, 726, 23, 667, 43, 462,
    M3<28>, M1, M3<18>, M3<3>, M3<21>, M3<-17>, M3<-28>, M3<-1> >
    Well23209a_base;

typedef Well_quoted<boost::uint32_t, 32, 726, 23, 610, 175, 662,
    M4<0xa8c296d1>, M1, M6<32, 15, 0x5d6b45cc, 0xfffeffff, 0x00000002>,
    M3<-24>, M3<-26>, M1, M0, M3<16> > Well23209b_base;

typedef Well_quoted<boost::uint32_t, 32, 1391, 15, 23, 481, 229,
    M3<-24>, M3<30>, M3<-10>, M2<-26>, M1, M3<20>,
    M6<32, 9, 0xb729fcec, 0xfbffffff, 0x00020000>, M1> Well44497a_base;

//! @}

} // namespace Detail

typedef boost::mpl::apply1<Detail::Well512a_base,
    Detail::NoTempering>::type Well512a;
typedef boost::mpl::apply1<Detail::Well521a_base,
    Detail::NoTempering>::type Well521a;
typedef boost::mpl::apply1<Detail::Well521b_base,
    Detail::NoTempering>::type Well521b;
typedef boost::mpl::apply1<Detail::Well607a_base,
    Detail::NoTempering>::type Well607a;
typedef boost::mpl::apply1<Detail::Well607b_base,
    Detail::NoTempering>::type Well607b;
typedef boost::mpl::apply1<Detail::Well800a_base,
    Detail::NoTempering>::type Well800a;
typedef boost::mpl::apply1<Detail::Well800b_base,
    Detail::NoTempering>::type Well800b;
typedef boost::mpl::apply1<Detail::Well1024a_base,
    Detail::NoTempering>::type Well1024a;
typedef boost::mpl::apply1<Detail::Well1024b_base,
    Detail::NoTempering>::type Well1024b;
typedef boost::mpl::apply1<Detail::Well19937a_base,
    Detail::NoTempering>::type Well19937a;
typedef boost::mpl::apply1<Detail::Well19937b_base,
    Detail::NoTempering>::type Well19937b;
typedef boost::mpl::apply1<Detail::Well19937a_base,
    Detail::MatsumotoKuritaTempering<0xe46e1700, 0x9b868000> >::type Well19937c;
typedef boost::mpl::apply1<Detail::Well21701a_base,
    Detail::NoTempering>::type Well21701a;
typedef boost::mpl::apply1<Detail::Well23209a_base,
    Detail::NoTempering>::type Well23209a;
typedef boost::mpl::apply1<Detail::Well23209b_base,
    Detail::NoTempering>::type Well23209b;
typedef boost::mpl::apply1<Detail::Well44497a_base,
    Detail::NoTempering>::type Well44497a;
typedef boost::mpl::apply1<Detail::Well44497a_base,
    Detail::MatsumotoKuritaTempering<0x93dd1400, 0xfa118000> >::type Well44497b;

/**
 * @name Maximally equidistributed versions using Harase's tempering method
 * @{
 */

typedef boost::mpl::apply1<Detail::Well800a_base,
    Detail::HaraseTempering<0x4880> >::type Well800a_ME;
typedef boost::mpl::apply1<Detail::Well800b_base,
    Detail::HaraseTempering<0x17030806> >::type Well800b_ME;
typedef boost::mpl::apply1<Detail::Well19937a_base,
    Detail::HaraseTempering<0x4118000> >::type Well19937a_ME;
typedef boost::mpl::apply1<Detail::Well19937b_base,
    Detail::HaraseTempering<0x30200010> >::type Well19937b_ME;
typedef boost::mpl::apply1<Detail::Well21701a_base,
    Detail::HaraseTempering<0x1002> >::type Well21701a_ME;
typedef boost::mpl::apply1<Detail::Well23209a_base,
    Detail::HaraseTempering<0x5100000> >::type Well23209a_ME;
typedef boost::mpl::apply1<Detail::Well23209b_base,
    Detail::HaraseTempering<0x34000300> >::type Well23209b_ME;
typedef boost::mpl::apply1<Detail::Well44497a_base,
    Detail::HaraseTempering<0x48000000> >::type Well44497a_ME;

//! @}

#endif // WELL_HPP
