#ifndef AES_H
#define AES_H
#include <array>
#include "utils.h"
#include "gf256.h"

// compile-time aes, lol

namespace aes
{

    namespace impl
    {

        typedef std::array<uint8_t, 16> block;
        typedef std::array<uint8_t, 4> row;

        // sbox generation stuff

        constexpr uint8_t affine(uint8_t r, uint8_t s, int n)
        {
            return n > 0 ? affine(r << 1 | r >> 7, r ^ s, n - 1) : s;
        }

        constexpr uint8_t affine(uint8_t r)
        {
            return affine(r, 0, 5);
        }

        constexpr uint8_t sbox(uint8_t a)
        {
            return affine(gf256::inv(a)) ^ 0x63;
        }


        // aes steps

        template <size_t... I>
        constexpr block sub_bytes(const block &b, util::seq<I...>)
        {
            return block { sbox(b[I])... };
        }

        constexpr block sub_bytes(const block &b) {
            return sub_bytes(b, util::gen_seq<16>());
        }

        template <size_t... I>
        constexpr block shift_rows(const block &b, util::seq<I...>)
        {
            return block { b[(I/4 + I % 4) % 4 * 4 + I % 4]... };
        }

        constexpr block shift_rows(const block &b) {
            return shift_rows(b, util::gen_seq<16>());
        }

        constexpr uint8_t mix(size_t row, size_t col, uint8_t val)
        {
            return gf256::mul(row == col ? 2 : col == (row+1) % 4 ? 3 : 1, val);
        }

        template <size_t... I>
        constexpr block mix_columns(const block &b, util::seq<I...>)
        {
            return block { ((uint8_t)(mix(I%4, 0, b[(I/4)*4 + 0]) ^  mix(I%4, 1, b[(I/4)*4 + 1]) ^
                mix(I%4, 2, b[(I/4)*4 + 2]) ^ mix(I%4, 3, b[(I/4)*4 + 3])))... };
        }

        constexpr block mix_columns(const block &b)
        {
            return mix_columns(b, util::gen_seq<16>());
        }

        template <size_t N, size_t... I>
        constexpr block add_round_key(const block &b, const std::array<uint8_t, N> &key,
             size_t offs, util::seq<I...>)
        {
            return block { ((uint8_t)(b[I] ^ key[I + offs]))... };
        }

        template <size_t N>
        constexpr block add_round_key(const block &b, const std::array<uint8_t, N> &key, size_t offs)
        {
            return add_round_key(b, key, offs, util::gen_seq<16>());
        }

        // key schedule (i'm too lazy to optimize this shit
        // so enjoy tons of copy & paste and lispish smiles ))))))))

        constexpr uint8_t rcon(size_t i)
        {
            return i == 1 ? 1 : gf256::mul(rcon(i-1), 2);
        }

        template <size_t KS, size_t I, size_t N>
        constexpr std::array<uint8_t, N> schedule_phase1(const std::array<uint8_t, N> &a)
        {
            return util::shift(a, row {
                (uint8_t)(a[N-KS+0] ^ sbox(a[N-3]) ^ rcon(I)),
                (uint8_t)(a[N-KS+1] ^ sbox(a[N-2])),
                (uint8_t)(a[N-KS+2] ^ sbox(a[N-1])),
                (uint8_t)(a[N-KS+3] ^ sbox(a[N-4]))
            });
        }

        template <size_t KS, size_t N>
        constexpr std::array<uint8_t, N> schedule_phase2(const std::array<uint8_t, N> &a)
        {
            return util::shift(a, row {
                (uint8_t)(a[N-KS+0] ^ a[N-4]),
                (uint8_t)(a[N-KS+1] ^ a[N-3]),
                (uint8_t)(a[N-KS+2] ^ a[N-2]),
                (uint8_t)(a[N-KS+3] ^ a[N-1])
            });
        }

        template <size_t KS, size_t N>
        constexpr std::array<uint8_t, N> schedule_phase3(const std::array<uint8_t, N> &a)
        {
            return util::shift(a, row {
                (uint8_t)(a[N-KS+0] ^ sbox(a[N-4])),
                (uint8_t)(a[N-KS+1] ^ sbox(a[N-3])),
                (uint8_t)(a[N-KS+2] ^ sbox(a[N-2])),
                (uint8_t)(a[N-KS+3] ^ sbox(a[N-1]))
            });
        }

        template <size_t KS, size_t I, size_t N>
        constexpr std::array<uint8_t, N> schedule_row1(const std::array<uint8_t, N> &a)
        {
            return schedule_phase2<KS>(schedule_phase2<KS>(
                schedule_phase2<KS>(schedule_phase1<KS, I>(a))));

        }

        template <size_t KS, size_t N>
        constexpr std::array<uint8_t, N> schedule_row2(const std::array<uint8_t, N> &a)
        {
            return schedule_phase2<KS>(schedule_phase2<KS>(
                schedule_phase2<KS>(schedule_phase3<KS>(a))));

        }

        constexpr std::array<uint8_t, 240> schedule_key(const std::array<uint8_t, 32> &key)
        {
            return
                schedule_row1<32, 7>(
                schedule_row2<32>(schedule_row1<32, 6>(
                schedule_row2<32>(schedule_row1<32, 5>(
                schedule_row2<32>(schedule_row1<32, 4>(
                schedule_row2<32>(schedule_row1<32, 3>(
                schedule_row2<32>(schedule_row1<32, 2>(
                schedule_row2<32>(schedule_row1<32, 1>(
                util::shift(std::array<uint8_t, 240> {0}, key))))))))))))));
        }

        constexpr std::array<uint8_t, 208> schedule_key(const std::array<uint8_t, 24> &key)
        {
            return
                schedule_row1<24, 8>(
                schedule_phase2<24>(schedule_phase2<24>(schedule_row1<24, 7>(
                schedule_phase2<24>(schedule_phase2<24>(schedule_row1<24, 6>(
                schedule_phase2<24>(schedule_phase2<24>(schedule_row1<24, 5>(
                schedule_phase2<24>(schedule_phase2<24>(schedule_row1<24, 4>(
                schedule_phase2<24>(schedule_phase2<24>(schedule_row1<24, 3>(
                schedule_phase2<24>(schedule_phase2<24>(schedule_row1<24, 2>(
                schedule_phase2<24>(schedule_phase2<24>(schedule_row1<24, 1>(
                util::shift(std::array<uint8_t, 208> {0}, key)))))))))))))))))))))));
        }

        constexpr std::array<uint8_t, 176> schedule_key(const std::array<uint8_t, 16> &key)
        {
            return
                schedule_row1<16, 10>(schedule_row1<16, 9>(
                schedule_row1<16, 8>(schedule_row1<16, 7>(
                schedule_row1<16, 6>(schedule_row1<16, 5>(
                schedule_row1<16, 4>(schedule_row1<16, 3>(
                schedule_row1<16, 2>(schedule_row1<16, 1>(
                util::shift(std::array<uint8_t, 176> {0}, key)))))))))));
        }

        // encryption

        template <size_t N>
        constexpr block encrypt_step(const block &blk, const std::array<uint8_t, N> &key, size_t i)
        {
            return i == 0 ? encrypt_step(add_round_key(blk, key, i), key, i+16) :
                i + 16 < N ? encrypt_step(add_round_key(mix_columns(shift_rows(sub_bytes(blk))), key, i), key, i + 16) :
                add_round_key(shift_rows(sub_bytes(blk)), key, i);
        }

        template <size_t N>
        constexpr block encrypt(const block &blk, const std::array<uint8_t, N> &key)
        {
            return encrypt_step(blk, key, 0);
        }

        // CTR-mode keystream generator
        template <size_t... I>
        constexpr block ctr_to_block(uint64_t ctr, util::seq<I...>)
        {
            return block { ((uint8_t)(I < 8 ? 0 : (ctr >> ((15-I)*8)) & 0xFF))... };
        }

        constexpr block ctr_to_block(uint64_t ctr)
        {
            return ctr_to_block(ctr, util::gen_seq<16>());
        }

        template <size_t N, size_t... I>
        constexpr std::array<uint8_t, N> add(const std::array<uint8_t, N> &a,
            const std::array<uint8_t, N> &b, int carry, size_t i, util::seq<I...>)
        {
            return i == 0 ? block { (I == 0 ? (uint8_t)(a[0] + b[0] + carry) : a[I] ) ... } :
                add(block { (I != i ? a[I] : (uint8_t)(carry + a[i] + b[i]) ) ...  },
                    b, (carry + a[i] + b[i]) / 256, i - 1, util::gen_seq<16>());
        }

        template <size_t N>
        constexpr std::array<uint8_t, N> add(const std::array<uint8_t, N> &a,
            const std::array<uint8_t, N> &b)
        {
            return add(a, b, 0, 15, util::gen_seq<N>());
        }

        template <size_t N, size_t K, size_t... I>
        constexpr std::array<block, N> ctr_keystream(const std::array<uint8_t, K> &key, const block &iv, util::seq<I...>)
        {
            return std::array<block, N> { encrypt(add(iv, ctr_to_block(I)), key)... };
        }

        template <size_t N, size_t K>
        constexpr std::array<block, N> ctr_keystream(const std::array<uint8_t, K> &key, const block &iv)
        {
            return ctr_keystream<N>(key, iv, util::gen_seq<N>());
        }

        template <size_t N, size_t M, size_t... I>
        constexpr std::array<uint8_t, N> ctr_encrypt(const std::array<block, M> &keystream,
            const std::array<uint8_t, N> &data, util::seq<I...>)
        {
            return std::array<uint8_t, N> { ((uint8_t)(keystream[I/16][I%16] ^ data[I])) ... };
        }

    }

    template <size_t N, size_t K>
    constexpr std::array<uint8_t, N> ctr_encrypt(const std::array<uint8_t, K> &key,
        const std::array<uint8_t, 16> &iv, const std::array<uint8_t, N> &data)
    {
        return impl::ctr_encrypt<N, (N-1)/16+1>(impl::ctr_keystream<(N-1)/16+1>(impl::schedule_key(key), iv),
            data, util::gen_seq<N>());
    }

}


#endif
