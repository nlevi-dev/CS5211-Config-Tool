#pragma once
#include <cstdint>

// E2PROM memory layout constants for the CS5211 configuration blob.
// The blob is 8 KiB.  The first 4 KiB holds 16 x 128-byte EDID blocks
// (one per GPIO[3:0] panel index).  The second 4 KiB is the OCM firmware
// region, which starts with a small per-panel config table followed by the
// actual MCU code.

namespace RomLayout {

constexpr int kBlobSize        = 8192;

// EDID region
constexpr int kEdidBlockSize   = 128;   // bytes per EDID block
constexpr int kEdidPanelCount  = 16;    // number of panel slots

// OCM region
constexpr int kOcmBase         = 4096;  // byte offset of OCM region
constexpr int kCfgTableOffset  = 16;    // offset inside OCM to per-panel cfg table
constexpr int kCfgEntrySize    = 16;    // bytes per panel config entry
constexpr int kCfgTableSize    = 288;   // total size of per-panel cfg table

// Metadata stored just before the OCM region
constexpr int kOcmCrcAddr      = 4090;
constexpr int kOcmLenAddr      = 4094;

// Absolute addresses of chip-global config fields (inside OCM cfg table)
constexpr int kAddrLinkSpeed   = 4368;
constexpr int kAddrLaneCnt     = 4369;
constexpr int kAddrPwmMode     = 4370;
constexpr int kAddrT2          = 4371;
constexpr int kAddrT3H         = 4372;
constexpr int kAddrT3L         = 4373;
constexpr int kAddrT4          = 4374;
constexpr int kAddrT5H         = 4375;
constexpr int kAddrT5L         = 4376;
constexpr int kAddrT7H         = 4377;
constexpr int kAddrT7L         = 4378;
constexpr int kAddrPwmDefault  = 4379;
constexpr int kAddrPwmMin      = 4380;
constexpr int kAddrPwmMax      = 4381;
constexpr int kAddrClkSel      = 4382;
constexpr int kAddrSscStep     = 4383;
constexpr int kAddrSscDiv      = 4384;
constexpr int kAddrSscMode     = 4385;
constexpr int kAddrSlewCtrl    = 4386;
constexpr int kAddrFwVersion   = 4399;

} // namespace RomLayout
