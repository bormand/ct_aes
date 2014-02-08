#include <iostream>
#include <iomanip>
#include "aes.h"


int main() {

    constexpr auto enc = aes::ctr_encrypt(
        std::array<uint8_t, 16> {
            0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
            0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c },
        std::array<uint8_t, 16> {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        std::array<uint8_t, 14> { "Hello, world!" });

    for (size_t i = 0; i < enc.size(); ++i) {
        if (i > 0 && i % 16 == 0)
            std::cout << std::endl;
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)enc[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}
