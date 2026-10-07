#include "Utility.hpp"

#if defined(ADH_WINDOWS)
#    include <Windows.h>
#endif

namespace adh {
#if defined(ADH_DEBUG)
    bool ShowThrowDialog(const std::string& text) noexcept {
        std::cerr << text << std::endl;
#    if defined(ADH_WINDOWS)
        const int size{ MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0) };
        std::wstring wide(static_cast<std::size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), size);
        return MessageBoxW(nullptr, wide.c_str(), L"AdHoc error: ignore?", MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2) == IDYES;
#    else
        return false;
#    endif
    }
#endif
} // namespace adh
