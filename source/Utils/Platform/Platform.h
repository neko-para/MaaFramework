#pragma once

#include <cstddef>
#include <filesystem>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

#ifdef _WIN32
#include "PlatformWin32.h"
#include "SafeWindows.h"
#else
#include "PlatformPosix.h"
#endif //  _WIN32

#include "Common/MaaConf.h"

#define MAA_PLATFORM_NS MAA_NS::platform_ns
#define MAA_PLATFORM_NS_BEGIN \
    namespace MAA_PLATFORM_NS \
    {
#define MAA_PLATFORM_NS_END }

MAA_PLATFORM_NS_BEGIN

// TODO: 也许应该从os_string计算出os_string_view, 但是我不清楚怎么算
using os_string = std::filesystem::path::string_type;
using os_string_view = std::basic_string_view<std::filesystem::path::value_type>;

inline std::filesystem::path path(const os_string_view& os_str)
{
    return std::filesystem::path(os_str);
}

#ifdef _WIN32
std::string path_to_crt_string(const std::filesystem::path&);
std::string path_to_ansi_string(const std::filesystem::path&);
os_string to_osstring(const std::string_view& utf8_str);
std::string from_osstring(const os_string_view&);

// Allow construct a path from utf8-string in win32; string_view ver.
inline std::filesystem::path path(const std::string_view& utf8_str)
{
    return std::filesystem::path(to_osstring(utf8_str));
}

inline std::string path_to_utf8_string(const std::filesystem::path& path)
{
    return from_osstring(path.native());
}

inline std::string path_to_crt_string(const std::string_view& utf8_path)
{
    return path_to_crt_string(path(utf8_path));
}

inline std::string path_to_ansi_string(const std::string_view& utf8_path)
{
    return path_to_crt_string(path(utf8_path));
}

#else

inline os_string_view to_osstring(const std::string_view& utf8_str)
{
    return utf8_str;
}

inline std::string_view from_osstring(const os_string_view& os_str)
{
    return os_str;
}

inline std::string path_to_utf8_string(const std::filesystem::path& path)
{
    return path.native();
}

inline std::string path_to_ansi_string(const std::filesystem::path& path)
{
    return path.native();
}

inline std::string path_to_crt_string(const std::filesystem::path& path)
{
    return path.native();
}

#endif

// --------- detail ------------

extern const size_t page_size;

void* aligned_alloc(size_t len, size_t align);
void aligned_free(void* ptr);

template <typename TElem>
requires std::is_trivial_v<TElem>
class single_page_buffer
{
    TElem* _ptr = nullptr;

public:
    single_page_buffer()
    {
        _ptr = reinterpret_cast<TElem*>(aligned_alloc(page_size, page_size));
        if (!_ptr) throw std::bad_alloc();
    }

    explicit single_page_buffer(std::nullptr_t) {}

    ~single_page_buffer()
    {
        if (_ptr) aligned_free(reinterpret_cast<void*>(_ptr));
    }

    // disable copy construct
    single_page_buffer(const single_page_buffer&) = delete;
    single_page_buffer& operator=(const single_page_buffer&) = delete;

    inline single_page_buffer(single_page_buffer&& other) noexcept { std::swap(_ptr, other._ptr); }
    inline single_page_buffer& operator=(single_page_buffer&& other) noexcept
    {
        if (_ptr) {
            aligned_free(reinterpret_cast<void*>(_ptr));
            _ptr = nullptr;
        }
        std::swap(_ptr, other._ptr);
        return *this;
    }

    inline TElem* get() const { return _ptr; }
    inline size_t size() const { return _ptr ? (page_size / sizeof(TElem)) : 0; }
};

MAA_PLATFORM_NS_END
