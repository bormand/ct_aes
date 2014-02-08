#ifndef GF256_H
#define GF256_H

// compile-time ops in GF(2^8) with 0x11b irreducible polynomial

namespace gf256
{

    namespace impl
    {

        constexpr uint8_t mul_step(uint8_t a, uint8_t b, uint8_t p, int c)
        {
            return c > 0 ? mul_step((a << 1) ^ (a & 0x80 ? 0x1b : 0x00),
                b >> 1, b & 0x01 ? a ^ p : p, c - 1) : p;
        }

        constexpr int degree(int x)
        {
            return x != 0 ? degree(x >> 1) + 1 : 0;
        }

        constexpr int inv_step(int u, int v, int g1, int g2);
        constexpr int inv_step(int u, int v, int g1, int g2, int j)
        {
            return j > 0 ? inv_step(u ^ (v << j), v, g1 ^ (g2 << j), g2) :
                           inv_step(v ^ (u << (-j)), u, g2 ^ (g1 << (-j)), g1);
        }

        constexpr int inv_step(int u, int v, int g1, int g2)
        {
            return u == 1 ? g1 : inv_step(u, v, g1, g2, degree(u) - degree(v));
        }

    }

    constexpr uint8_t mul(uint8_t a, uint8_t b)
    {
        return impl::mul_step(a, b, 0, 8);
    }

    constexpr uint8_t inv(uint8_t a)
    {
        return a == 0 ? 0 : impl::inv_step(a, 0x11b, 1, 0);
    }

}


#endif
