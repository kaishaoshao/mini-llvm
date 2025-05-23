#pragma once

#include <cstdio>
#include <utility>

namespace mini_llvm {

class FileHandle {
public:
    FileHandle() = default;

    explicit FileHandle(FILE *handle) : handle_(handle) {}

    FileHandle(const char *path, const char *mode);
    ~FileHandle();

    FileHandle(const FileHandle &) = delete;

    FileHandle(FileHandle &&other) noexcept {
        swap(other);
    }

    FileHandle &operator=(FileHandle other) noexcept {
        swap(other);
        return *this;
    }

    explicit operator bool() const {
        return handle_;
    }

    FILE *get() {
        return handle_;
    }

    const FILE *get() const {
        return handle_;
    }

    void open(const char *path, const char *mode);
    void close();

    void swap(FileHandle &other) noexcept {
        std::swap(handle_, other.handle_);
    }

private:
    FILE *handle_{};
};

inline void swap(FileHandle &lhs, FileHandle &rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace mini_llvm
