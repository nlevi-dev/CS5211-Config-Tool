#pragma once
#include <cstdint>
#include <array>
#include "romdata.h"

// Singleton holding the in-memory copy of the 8 KiB E2PROM blob.
// All dialogs read/write through this store; the main window is responsible
// for loading from and saving to disk.
class BlobStore {
public:
    using Blob = std::array<uint8_t, RomLayout::kBlobSize>;

    static Blob &data();
    static int  &currentPanel();  // index 0-15 of the panel being edited

private:
    BlobStore() = delete;
};
