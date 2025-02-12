#ifdef _WIN32
#include "PlatformWin32.h"
#include "Platform.h"

#include <atomic>
#include <format>
#include <mbctype.h>

#include "Utils/StringMisc.hpp"

MAA_PLATFORM_NS_BEGIN

static size_t get_page_size()
{
    SYSTEM_INFO sysInfo {};
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
}

const size_t page_size = get_page_size();

void* aligned_alloc(size_t len, size_t align)
{
    return _aligned_malloc(len, align);
}

void aligned_free(void* ptr)
{
    _aligned_free(ptr);
}

static std::string get_ansi_short_path(const std::filesystem::path& path)
{
    wchar_t short_path[MAX_PATH] {};
    auto& osstr = path.native();
    auto shortlen = GetShortPathNameW(osstr.c_str(), short_path, MAX_PATH);
    if (shortlen == 0) return {};
    BOOL failed = FALSE;
    auto ansilen = WideCharToMultiByte(CP_ACP, 0, short_path, shortlen, nullptr, 0, nullptr, &failed);
    if (failed) return {};
    std::string result(ansilen, 0);
    WideCharToMultiByte(CP_ACP, 0, short_path, shortlen, result.data(), ansilen, nullptr, nullptr);
    return result;
}

std::string path_to_crt_string(const std::filesystem::path& path)
{
    // UCRT may use UTF-8 encoding while ANSI code page is still some other MBCS encoding
    // so we use CRT wcstombs instead of WideCharToMultiByte
    size_t mbsize = 0;
    auto& osstr = path.native();
    auto err = wcstombs_s(&mbsize, nullptr, 0, osstr.c_str(), osstr.size());
    if (err == 0) {
        std::string result(mbsize, 0);
        err = wcstombs_s(&mbsize, result.data(), mbsize, osstr.c_str(), osstr.size());
        if (err != 0) return {};
        return result.substr(0, mbsize - 1);
    }
    else {
        // cannot convert (CRT is not using UTF-8), fallback to short path name in ACP
        return get_ansi_short_path(path);
    }
}

std::string path_to_ansi_string(const std::filesystem::path& path)
{
    // UCRT may use UTF-8 encoding while ANSI code page is still some other MBCS encoding
    // so we use CRT wcstombs instead of WideCharToMultiByte
    BOOL failed = FALSE;
    auto& osstr = path.native();
    auto ansilen = WideCharToMultiByte(CP_ACP, 0, osstr.c_str(), (int)osstr.size(), nullptr, 0, nullptr, &failed);
    if (!failed) {
        std::string result(ansilen, 0);
        WideCharToMultiByte(CP_ACP, 0, osstr.c_str(), (int)osstr.size(), result.data(), ansilen, nullptr, &failed);
        return result;
    }
    else {
        // contains character that cannot be converted, fallback to short path name in ACP
        return get_ansi_short_path(path);
    }
}

os_string to_osstring(const std::string_view& utf8_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), (int)utf8_str.size(), nullptr, 0);
    os_string result(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), (int)utf8_str.size(), result.data(), len);
    return result;
}

std::string from_osstring(const os_string_view& os_str)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, os_str.data(), (int)os_str.size(), nullptr, 0, nullptr, nullptr);
    std::string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, os_str.data(), (int)os_str.size(), result.data(), len, nullptr, nullptr);
    return result;
}

MAA_PLATFORM_NS_END

#endif
