#include "crc32util.h"

namespace Crc32Util {

static uint32_t reverseBits(uint32_t v) {
    uint32_t out = 0;
    for (int i = 0; i < 32; ++i) {
        out = ((v >> i) & 1) | out;
        if (i < 31) out <<= 1;
    }
    return out;
}

static uint32_t tableEntry(int idx) {
    uint64_t v = static_cast<uint64_t>(idx);
    for (int b = 8; b > 0; --b)
        v = (v & 1) ? ((v >> 1) ^ 0xEDB88320u) : (v >> 1);
    return static_cast<uint32_t>(v);
}

uint32_t compute(const uint8_t *data, int length) {
    uint64_t crc = 0x00FFFFFFuLL;
    for (int i = 0; i < length; ++i)
        crc = (crc >> 8) ^ tableEntry(static_cast<int>((crc & 0xFF) ^ data[i]));
    return reverseBits(static_cast<uint32_t>(crc));
}

} // namespace Crc32Util
