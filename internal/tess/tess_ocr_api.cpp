#include "tess_ocr_api.h"
#if defined(TESS_OCR_HAVE_INPROC_WEBCAM)
#include "webcam_ocr_runner.h"
#endif

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {

std::string GetEnvOrEmpty(const char* key) {
    char* value = nullptr;
    size_t len = 0;
    _dupenv_s(&value, &len, key);
    std::string out = (value && len > 0) ? std::string(value) : std::string();
    if (value) free(value);
    return out;
}

bool EnvFlagEnabled(const char* key) {
    const std::string raw = GetEnvOrEmpty(key);
    if (raw.empty()) return false;
    return raw == "1" || raw == "true" || raw == "TRUE" || raw == "yes" || raw == "YES";
}

std::string QuoteArg(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('"');
    for (char ch : s) {
        if (ch == '"') out.push_back('\\');
        out.push_back(ch);
    }
    out.push_back('"');
    return out;
}

std::string ModuleDir() {
    HMODULE hm = nullptr;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&tess_ocr_capture_from_webcam),
            &hm)) {
        return std::string();
    }

    std::array<char, MAX_PATH> path{};
    const DWORD n = GetModuleFileNameA(hm, path.data(), (DWORD)path.size());
    if (n == 0 || n >= path.size()) {
        return std::string();
    }
    return std::filesystem::path(path.data()).parent_path().string();
}

std::string ResolveWebcamExePath(const char* exe_path) {
    if (exe_path && *exe_path) return std::string(exe_path);
    const std::string from_env = GetEnvOrEmpty("TESS_OCR_WEBCAM_EXE");
    if (!from_env.empty()) return from_env;

    const std::string mod_dir = ModuleDir();
    if (!mod_dir.empty()) {
        const std::filesystem::path candidate = std::filesystem::path(mod_dir) / "webcam_ocr.exe";
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }
    return "webcam_ocr.exe";
}

bool CopyToOut(const std::string& value, char* out, unsigned int out_cap) {
    if (!out || out_cap == 0) return false;
    const size_t need = value.size() + 1;
    if (need > out_cap) return false;
    memcpy(out, value.c_str(), need);
    return true;
}

std::string Trim(std::string s) {
    size_t b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) b++;
    size_t e = s.size();
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) e--;
    return s.substr(b, e - b);
}

std::string UnescapeQuoted(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    bool esc = false;
    for (char ch : s) {
        if (esc) {
            out.push_back(ch);
            esc = false;
            continue;
        }
        if (ch == '\\') {
            esc = true;
            continue;
        }
        out.push_back(ch);
    }
    if (esc) out.push_back('\\');
    return out;
}

bool CopyWithCheck(const std::string& text, char* out_text, unsigned int out_cap) {
    if (!out_text || out_cap == 0) return false;
    if (!CopyToOut(text, out_text, out_cap)) {
        out_text[0] = '\0';
        return false;
    }
    return true;
}

std::string ParseFinalTextFromOutput(const std::string& stdout_text) {
    std::string final;
    std::string last_non_empty;
    size_t pos = 0;
    while (pos < stdout_text.size()) {
        size_t nl = stdout_text.find('\n', pos);
        std::string line = (nl == std::string::npos) ? stdout_text.substr(pos) : stdout_text.substr(pos, nl - pos);
        pos = (nl == std::string::npos) ? stdout_text.size() : nl + 1;
        line = Trim(line);
        if (line.empty()) continue;
        last_non_empty = line;

        const std::string prefix = "Final string: \"";
        if (line.rfind(prefix, 0) == 0 && line.size() >= prefix.size() + 1 && line.back() == '"') {
            final = UnescapeQuoted(line.substr(prefix.size(), line.size() - prefix.size() - 1));
            break;
        }
    }

    if (final.empty() && !last_non_empty.empty()) {
        final = last_non_empty;
    }
    return final;
}

int CaptureFromWebcamProcess(
    const std::string& exe_path,
    unsigned int timeout_ms,
    std::string& out_text,
    std::string& out_error
) {
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE stdout_rd = INVALID_HANDLE_VALUE;
    HANDLE stdout_wr = INVALID_HANDLE_VALUE;
    if (!CreatePipe(&stdout_rd, &stdout_wr, &sa, 0)) {
        out_error = "CreatePipe(stdout) failed";
        return TESS_OCR_ERR_START_FAILED;
    }
    SetHandleInformation(stdout_rd, HANDLE_FLAG_INHERIT, 0);

    HANDLE child_stderr = GetStdHandle(STD_ERROR_HANDLE);
    HANDLE nul_handle = INVALID_HANDLE_VALUE;
    if (child_stderr == nullptr || child_stderr == INVALID_HANDLE_VALUE) {
        nul_handle = CreateFileA(
            "NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
        );
        child_stderr = (nul_handle == INVALID_HANDLE_VALUE) ? stdout_wr : nul_handle;
    }

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = stdout_wr;
    si.hStdError = child_stderr;

    const std::string cmd = QuoteArg(exe_path) + " --no-prompt";
    std::vector<char> cmd_mut(cmd.begin(), cmd.end());
    cmd_mut.push_back('\0');

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(
        nullptr,
        cmd_mut.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    CloseHandle(stdout_wr);
    if (nul_handle != INVALID_HANDLE_VALUE) CloseHandle(nul_handle);

    if (!ok) {
        CloseHandle(stdout_rd);
        out_error = "Failed to launch webcam_ocr.exe";
        return TESS_OCR_ERR_START_FAILED;
    }

    std::string stdout_text;
    auto start = std::chrono::steady_clock::now();
    bool timed_out = false;

    for (;;) {
        DWORD avail = 0;
        if (!PeekNamedPipe(stdout_rd, nullptr, 0, nullptr, &avail, nullptr)) {
            break;
        }
        if (avail > 0) {
            std::vector<char> buf(std::min<DWORD>(avail, 4096u));
            DWORD rd = 0;
            if (!ReadFile(stdout_rd, buf.data(), (DWORD)buf.size(), &rd, nullptr)) {
                break;
            }
            if (rd > 0) stdout_text.append(buf.data(), buf.data() + rd);
        }

        DWORD wait = WaitForSingleObject(pi.hProcess, 20);
        if (wait == WAIT_OBJECT_0) {
            for (;;) {
                DWORD avail2 = 0;
                if (!PeekNamedPipe(stdout_rd, nullptr, 0, nullptr, &avail2, nullptr) || avail2 == 0) break;
                std::vector<char> buf2(std::min<DWORD>(avail2, 4096u));
                DWORD rd2 = 0;
                if (!ReadFile(stdout_rd, buf2.data(), (DWORD)buf2.size(), &rd2, nullptr) || rd2 == 0) break;
                stdout_text.append(buf2.data(), buf2.data() + rd2);
            }
            break;
        }

        if (timeout_ms > 0) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
            if (elapsed > (long long)timeout_ms) {
                timed_out = true;
                break;
            }
        }
    }

    if (timed_out) {
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 1000);
    } else {
        WaitForSingleObject(pi.hProcess, 1000);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(stdout_rd);

    if (timed_out) {
        out_error = "webcam capture timed out";
        return TESS_OCR_ERR_TIMEOUT;
    }

    out_text = ParseFinalTextFromOutput(stdout_text);
    return TESS_OCR_OK;
}

#if defined(TESS_OCR_HAVE_INPROC_WEBCAM)
int CaptureFromWebcamInProcess(
    unsigned int timeout_ms,
    std::string& out_text,
    std::string& out_error
) {
    if (timeout_ms > 0) {
        // In-process capture is interactive; timeout control requires process mode.
        out_error = "timeout is only supported in process mode";
        return TESS_OCR_ERR_INVALID_ARGUMENT;
    }

    const char* argv0 = "webcam_ocr";
    const char* argv1 = "--no-prompt";
    char* argv[] = {const_cast<char*>(argv0), const_cast<char*>(argv1)};

    std::ostringstream captured;
    std::streambuf* old_buf = std::cout.rdbuf(captured.rdbuf());
    int rc = 0;
    try {
        rc = tess_webcam_ocr_run(2, argv);
    } catch (...) {
        std::cout.rdbuf(old_buf);
        std::cerr << "[tess] webcam OCR threw an exception\n";
        out_error = "in-process webcam OCR threw an exception";
        return TESS_OCR_ERR_START_FAILED;
    }
    std::cout.rdbuf(old_buf);

    std::cerr << "[tess] webcam OCR finished (rc=" << rc << ")\n";

    const std::string raw = captured.str();
    if (!raw.empty()) {
        std::cerr << "[tess] captured stdout: (redacted)\n";
    } else {
        std::cerr << "[tess] captured stdout: (empty)\n";
    }

    out_text = ParseFinalTextFromOutput(raw);

    std::cerr << "[tess] parsed text: "
              << (out_text.empty() ? "(empty)" : "***") << "\n";

    if (rc != 0 && out_text.empty()) {
        out_error = "in-process webcam OCR failed";
        return TESS_OCR_ERR_START_FAILED;
    }
    return TESS_OCR_OK;
}
#endif

} // namespace

int tess_ocr_capture_from_webcam(
    const char* webcam_ocr_exe_path,
    unsigned int timeout_ms,
    char* out_text,
    unsigned int out_text_capacity
) {
    if (!out_text || out_text_capacity == 0) {
        return TESS_OCR_ERR_INVALID_ARGUMENT;
    }

    std::string text;
    std::string err;

    int rc = TESS_OCR_ERR_START_FAILED;
#if defined(TESS_OCR_HAVE_INPROC_WEBCAM)
    const bool force_process = EnvFlagEnabled("TESS_OCR_CAPTURE_USE_EXE");
    const bool has_exe_override = webcam_ocr_exe_path && webcam_ocr_exe_path[0] != '\0';
    if (!force_process && !has_exe_override) {
        rc = CaptureFromWebcamInProcess(timeout_ms, text, err);
    } else {
        const std::string exe_path = ResolveWebcamExePath(webcam_ocr_exe_path);
        rc = CaptureFromWebcamProcess(exe_path, timeout_ms, text, err);
    }
#else
    const std::string exe_path = ResolveWebcamExePath(webcam_ocr_exe_path);
    rc = CaptureFromWebcamProcess(exe_path, timeout_ms, text, err);
#endif

    if (rc != TESS_OCR_OK) {
        out_text[0] = '\0';
        return rc;
    }

    if (!CopyWithCheck(text, out_text, out_text_capacity)) {
        return TESS_OCR_ERR_BUFFER_TOO_SMALL;
    }
    return TESS_OCR_OK;
}
