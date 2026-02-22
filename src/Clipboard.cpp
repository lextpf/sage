#include "Clipboard.h"
#include "Utils.h"

#include <shellapi.h>

#include <cstring>
#include <thread>

namespace
{

// RAII guard for OpenClipboard / CloseClipboard.
// Does NOT empty the clipboard on construction - callers that need to
// write must call EmptyClipboard() explicitly after acquiring the lock.
struct ClipboardLock
{
    bool ok = false;

    ClipboardLock()
    {
        ok = !!OpenClipboard(nullptr);
    }

    ~ClipboardLock()
    {
        if (ok)
        {
            CloseClipboard();
        }
    }

    ClipboardLock(const ClipboardLock&) = delete;
    ClipboardLock& operator=(const ClipboardLock&) = delete;
};

} // namespace

namespace sage {

bool Clipboard::setText(const std::string& text)
{
    ClipboardLock lock;
    if (!lock.ok)
    {
        return false;
    }

    // Must empty before SetClipboardData
    EmptyClipboard();

    if (text.size() > static_cast<size_t>(INT_MAX))
    {
        return false;
    }

    int wlen = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        text.data(), static_cast<int>(text.size()),
        nullptr, 0);
    if (wlen <= 0)
    {
        return false;
    }

    SIZE_T bytes = (static_cast<SIZE_T>(wlen) + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem)
    {
        return false;
    }

    wchar_t* p = static_cast<wchar_t*>(GlobalLock(hMem));
    if (!p)
    {
        GlobalFree(hMem);
        return false;
    }

    int written = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        text.data(), static_cast<int>(text.size()),
        p, wlen);
    if (written <= 0)
    {
        GlobalUnlock(hMem);
        GlobalFree(hMem);
        return false;
    }

    p[written] = L'\0';
    GlobalUnlock(hMem);

    // SetClipboardData takes ownership of hMem on success
    if (!SetClipboardData(CF_UNICODETEXT, hMem))
    {
        GlobalFree(hMem);
        return false;
    }
    return true;
}

bool Clipboard::copyWithTTL(const char* data, size_t n, DWORD ttl_ms)
{
    std::string val(data, n);
    bool ok = setText(val);
    if (!ok)
    {
        return false;
    }

    // Detached thread: sleeps, then scrubs the clipboard if content is unchanged
    std::thread([val = std::move(val), ttl_ms]() mutable
    {
        Sleep(ttl_ms);

        // Open clipboard without emptying - we only want to read-compare
        ClipboardLock lock;
        if (lock.ok)
        {
            HANDLE h = GetClipboardData(CF_UNICODETEXT);
            bool same = false;
            if (h)
            {
                if (wchar_t* w = static_cast<wchar_t*>(GlobalLock(h)))
                {
                    // Round-trip current clipboard UTF-16 back to UTF-8 for comparison
                    int need = WideCharToMultiByte(
                        CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
                    std::string cur(need ? static_cast<size_t>(need) - 1 : 0, '\0');
                    if (need)
                    {
                        WideCharToMultiByte(
                            CP_UTF8, 0, w, -1, cur.data(), need, nullptr, nullptr);
                    }
                    GlobalUnlock(h);

                    // Constant-time compare to avoid timing leaks
                    same = sage::Cryptography::ctEqualAny(cur, val);
                }
            }

            // Only clear if nobody else has changed the clipboard
            if (same)
            {
                EmptyClipboard();
            }
        }
        sage::Cryptography::cleanseString(val);
    }).detach();

    return true;
}

bool Clipboard::copyWithTTL(const char* s, DWORD ttl_ms)
{
    if (!s)
        return copyWithTTL("", 0, ttl_ms);
    return copyWithTTL(s, std::strlen(s), ttl_ms);
}

bool Clipboard::copyInputFile()
{
    std::string buf;
    if (!utils::read_bin("sage", buf))
    {
        return false;
    }
    return copyWithTTL(buf);
}

bool typeSecret(const wchar_t* bytes, int len, DWORD delay_ms)
{
    if (!bytes)
    {
        return false;
    }

    std::wstring w;
    if (len < 0)
    {
        w = std::wstring(bytes);
        if (w.empty())
        {
            return false;
        }
    }
    else
    {
        if (len <= 0)
        {
            return false;
        }
        w.assign(bytes, bytes + static_cast<size_t>(len));
        // Strip trailing null if the caller included one in the length
        if (!w.empty() && w.back() == L'\0')
        {
            w.pop_back();
        }
    }

    // Give the user time to switch focus to the target window
    Sleep(delay_ms);

    // Build key-down / key-up pairs for each UTF-16 code unit
    std::vector<INPUT> seq;
    seq.reserve(static_cast<size_t>(w.size()) * 2);
    for (wchar_t ch : w)
    {
        INPUT down{};
        down.type = INPUT_KEYBOARD;
        down.ki.wScan = ch;
        down.ki.dwFlags = KEYEVENTF_UNICODE;
        INPUT up = down;
        up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        seq.push_back(down);
        seq.push_back(up);
    }

    // Send one event at a time with a small randomised delay after each pair
    // to avoid tripping input-rate limiters in target applications
    for (size_t i = 0; i < seq.size(); ++i)
    {
        SendInput(1, &seq[i], sizeof(INPUT));
        if ((i & 1) == 1)
        {
            Sleep(5 + (GetTickCount64() & 7));
        }
    }

    // Scrub sensitive keystroke data before returning
    SecureZeroMemory(seq.data(), seq.size() * sizeof(INPUT));
    SecureZeroMemory(w.data(), w.size() * sizeof(wchar_t));
    return true;
}

bool openInputInNotepad()
{
    const char* file = "sage";
    HINSTANCE h = ShellExecuteA(
        nullptr, "open", "notepad.exe", file, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(h) <= 32)
    {
        // Fallback: ShellExecuteA can fail on restricted accounts
        int ret = system("cmd /c start \"\" notepad.exe sage");
        return (ret == 0);
    }
    return true;
}

void wipeConsoleBuffer()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(hOut, &info))
    {
        return;
    }

    DWORD cells = static_cast<DWORD>(info.dwSize.X) * static_cast<DWORD>(info.dwSize.Y);
    COORD home{ 0, 0 };
    DWORD written = 0;

    // Overwrite every character cell, then reset attributes and cursor
    FillConsoleOutputCharacterA(hOut, ' ', cells, home, &written);
    FillConsoleOutputAttribute(hOut, info.wAttributes, cells, home, &written);
    SetConsoleCursorPosition(hOut, home);
}

} // namespace sage
