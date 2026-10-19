# CS5211 Config Tool

A clean-room, open-source reimplementation of the proprietary Capstone CS5211 EEPROM configuration tool, translated to English.

The CS5211 is an eDP-to-LVDS bridge chip. It is configured by flashing an 8 KiB binary image to an external I²C EEPROM (typically 24C02/24C04). The original vendor tool is a Windows-only application with a Chinese-language UI. This tool replicates its functionality, runs on Linux and Windows, and is written in C++17 with Qt 5.

---

## Features

- Import and export the 8 KiB EEPROM binary (`.bin`)
- Edit the **EDID block** for each of the 16 panel slots (GPIO[3:0] index): resolution presets, timing parameters, manufacturer info, sync polarity, colour depth, gamma
- Edit **per-panel LVDS output config**: port count, colour depth, data format (SPWG/OpenLDI), channel mapping, P/N swap, power-down per channel, signal polarity inversions, output swing and VCOM
- Edit **chip-global config** shared by all 16 panels: eDP link speed and lane count, PWM mode and duty cycle, power sequencing timings (T2/T3/T4/T5/T7), spread spectrum clock (SSC), LVDS slew rate, clock source
- "All panels identical" mode — replicates the selected panel's config to all 16 slots on save
- Optional MCU firmware import — splices new firmware into the blob while preserving the per-panel config table
- CRC-32 recomputed on save (IEEE 802.3 reflected poly with final bit-reversal, matching the chip's firmware verification)
- Loads `Template-CS5211.bin` from the executable directory on startup if present

---

## EEPROM Binary Layout

| Region | Offset | Size | Description |
|---|---|---|---|
| EDID blocks | 0x0000 | 16 × 128 B | One 128-byte EDID per panel slot |
| OCM metadata | 0x0FFA–0x0FFF | 6 B | CRC-32 (4 B) + OCM length (2 B) |
| OCM preamble | 0x1000 | 16 B | Copied verbatim from firmware |
| Panel config table | 0x1010 | 16 × 16 B | Per-panel LVDS register values |
| Chip-global config | 0x1110 | 32 B | Link speed, PWM, timing, SSC, slew, firmware version |
| MCU firmware | 0x1130 + | variable | Remaining OCM firmware code |

---

## Building

### Prerequisites

- CMake ≥ 3.16
- Qt 5 (Widgets module)
- A C++17-capable compiler

### Linux

**Build dependencies**

```sh
sudo apt install cmake qtbase5-dev
```

**Runtime dependency**

```sh
sudo apt-get install libqt5widgets5
```

**Build**

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Windows

**Build dependencies**

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install qt5[essentials]:x64-windows-static
```

**Build**

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build . --config Release
```

### Template

No template binary is included in this repository. The CS5211 firmware is the intellectual property of Capstone Semiconductor and cannot be redistributed as part of a clean-room release.

If you own a device with a CS5211 chip, you can dump the existing EEPROM contents using any I²C EEPROM reader and use that dump as your starting point. Save it as `Template-CS5211.bin` next to the executable and it will be loaded automatically on startup.

---

## License

MIT-0 — see [LICENSE](LICENSE).
