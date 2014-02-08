#ifndef UTILS_H
#define UTILS_H

#include <array>


namespace util
{

    // sequence generation stuff
    template <size_t... I>
    struct seq {};

    template <size_t N, size_t... I>
    struct gen_seq : gen_seq<N-1, N-1, I...> {};

    template <size_t... I>
    struct gen_seq<0, I...> : seq<I...> {};

    // array shifting

    namespace impl
    {
        template <class T, size_t N, size_t K, size_t... I>
        constexpr std::array<T, N> shift(const std::array<T, N> &a, const std::array<T, K> &b, seq<I...>)
        {
            return std::array<T, N> { (I < (N-K) ? a[I+K] : b[I-N+K])... };
        }
    }

    template <class T, size_t N, size_t K>
    constexpr std::array<T, N> shift(const std::array<T, N> &a, const std::array<T, K> &b)
    {
        return impl::shift(a, b, gen_seq<N>());
    }

}

#endif
