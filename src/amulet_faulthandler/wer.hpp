#pragma once

#include <windows.h>

#include <cstdint>
#include <stdexcept>
#include <cwchar>

#define DumpConfigMagic 0x616d6668

class DumpConfig
{
public:
    DWORD magic;
    std::uint16_t dump_index;
    wchar_t path[MAX_PATH];
    wchar_t ext[16];
    bool full_dump;

    DumpConfig() : magic(DumpConfigMagic) {
        path[0] = 0;
        ext[0] = 0;
        full_dump = false;
        dump_index = 0;
    }

    // Check if the magic number is correct.
    bool is_dump_context() const {
        return magic == DumpConfigMagic;
    }

    // Check if the path and extension have been set.
    bool is_set() const {
        return path[0] != 0 && ext[0] != 0;
    }

    // Set the path and extension.
    void set(const wchar_t* path_, const wchar_t* ext_, bool full_dump_) {
        if (!path_ || !ext_){
            throw std::invalid_argument("path or ext is undefined");
        }

        // Get the string lengths
        auto path_len = wcslen(path_);
        auto ext_len = wcslen(ext_);

        // Validate lengths
        if (path_len == 0)
            throw std::invalid_argument("path is undefined");
        if (path_len >= MAX_PATH)
            throw std::invalid_argument("path path is too long");
        if (ext_len >= 16)
            throw std::invalid_argument("ext is too long");
        if (MAX_PATH < path_len + max(ext_len, 4) + 6) {
            throw std::invalid_argument("Path is too long for Windows");
        }

        // Set the values
        wcscpy_s(path, path_);
        if (ext_len){
            wcscpy_s(ext, ext_);
        } else {
            wcscpy_s(ext, L".dmp");
        }
        full_dump = full_dump_;
    }
};
