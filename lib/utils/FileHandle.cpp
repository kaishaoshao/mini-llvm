#include "mini-llvm/utils/FileHandle.h"

#include <cstdio>

using namespace mini_llvm;

FileHandle::FileHandle(const char *path, const char *mode) {
    open(path, mode);
}

FileHandle::~FileHandle() {
    close();
}

void FileHandle::open(const char *path, const char *mode) {
    handle_ = fopen(path, mode);
}

void FileHandle::close() {
    if (handle_ == nullptr) return;
    fclose(handle_);
    handle_ = nullptr;
}
