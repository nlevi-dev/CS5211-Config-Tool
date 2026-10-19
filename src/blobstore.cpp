#include "blobstore.h"

BlobStore::Blob &BlobStore::data() {
    static Blob instance{};
    return instance;
}

int &BlobStore::currentPanel() {
    static int idx = 0;
    return idx;
}
