#pragma once
#include <cstdint>
#include <vector>

// CRC-32 as used by the CS5211 firmware blob.
// The polynomial is the standard IEEE 802.3 reflected poly (0xEDB88320).
// After the standard CRC the firmware applies a bit-reversal of the 32-bit
// result before storing it.

namespace Crc32Util {

uint32_t compute(const uint8_t *data, int length);

} // namespace Crc32Util
