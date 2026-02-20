#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cwctype>
#include <set>
#include <utility>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <unordered_map>
#include <cmath>
#include <climits>
#include <future>
#include <filesystem>
#include <opencv2/opencv.hpp>

#include "webcam_ocr_runner.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dshow.h>
#include <comdef.h>

#pragma comment(lib, "strmiids.lib")

namespace {

std::wstring ToLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](wchar_t ch) { return (wchar_t)std::towlower(ch); });
    return s;
}

std::vector<std::wstring> EnumerateVideoDeviceNamesDShow() {
    std::vector<std::wstring> names;

    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool coInitialized = SUCCEEDED(hrCo);

    ICreateDevEnum* devEnum = nullptr;
    IEnumMoniker* enumMoniker = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum, (void**)&devEnum
    );
    if (SUCCEEDED(hr) && devEnum) {
        hr = devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumMoniker, 0);
        if (hr == S_OK && enumMoniker) {
            IMoniker* moniker = nullptr;
            while (enumMoniker->Next(1, &moniker, nullptr) == S_OK) {
                IPropertyBag* bag = nullptr;
                if (SUCCEEDED(moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&bag)) && bag) {
                    VARIANT varName;
                    VariantInit(&varName);
                    if (SUCCEEDED(bag->Read(L"FriendlyName", &varName, 0)) && varName.vt == VT_BSTR) {
                        names.emplace_back(varName.bstrVal);
                    } else {
                        names.emplace_back(L"");
                    }
                    VariantClear(&varName);
                    bag->Release();
                } else {
                    names.emplace_back(L"");
                }
                moniker->Release();
            }
        }
    }

    if (enumMoniker) enumMoniker->Release();
    if (devEnum) devEnum->Release();
    if (coInitialized) CoUninitialize();

    return names;
}

int ChooseCameraIndexFromNames(const std::vector<std::wstring>& names, bool log) {
    if (const char* forced = std::getenv("TESS_CAMERA_INDEX")) {
        char* end = nullptr;
        long idx = std::strtol(forced, &end, 10);
        if (end != forced && *end == '\0' && idx >= 0 && idx <= 99) {
            if (log) std::cerr << "Using TESS_CAMERA_INDEX=" << idx << "\n";
            return (int)idx;
        }
    }

    if (names.empty()) {
        return 0;
    }

    if (log) {
        std::cerr << "Detected cameras:\n";
        for (size_t i = 0; i < names.size(); ++i) {
            std::wcerr << L"  [" << i << L"] " << names[i] << L"\n";
        }
    }

    const std::vector<std::wstring> preferredKeywords = {
        L"razer kiyo", L"razer"
    };
    for (size_t i = 0; i < names.size(); ++i) {
        const std::wstring nameLower = ToLower(names[i]);
        for (const auto& kw : preferredKeywords) {
                if (nameLower.find(kw) != std::wstring::npos) {
                if (log) {
                    std::wcerr << L"Selecting preferred webcam: " << names[i]
                               << L" (index " << i << L")\n";
                }
                return (int)i;
            }
        }
    }

    // If Razer is not found, avoid obvious virtual cameras when possible.
    const std::vector<std::wstring> avoidKeywords = {
        L"camo", L"virtual", L"obs", L"droidcam", L"ndi"
    };
    for (size_t i = 0; i < names.size(); ++i) {
        const std::wstring nameLower = ToLower(names[i]);
        bool avoid = false;
        for (const auto& kw : avoidKeywords) {
            if (nameLower.find(kw) != std::wstring::npos) {
                avoid = true;
                break;
            }
        }
        if (!avoid) {
            if (log) {
                std::wcerr << L"Selecting first non-virtual camera: " << names[i]
                           << L" (index " << i << L")\n";
            }
            return (int)i;
        }
    }

    return 0;
}

int ChooseCameraIndex() {
    return ChooseCameraIndexFromNames(EnumerateVideoDeviceNamesDShow(), true);
}

bool IsVirtualCameraName(const std::wstring& name) {
    const std::wstring nameLower = ToLower(name);
    const std::vector<std::wstring> avoidKeywords = {
        L"camo", L"virtual", L"obs", L"droidcam", L"ndi"
    };
    for (const auto& kw : avoidKeywords) {
        if (nameLower.find(kw) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

bool IsObsCameraName(const std::wstring& name) {
    const std::wstring nameLower = ToLower(name);
    return nameLower.find(L"obs") != std::wstring::npos;
}

bool TryGetEnvIndex(const char* key, int& out) {
    if (const char* raw = std::getenv(key)) {
        char* end = nullptr;
        long idx = std::strtol(raw, &end, 10);
        if (end != raw && *end == '\0' && idx >= 0 && idx <= 99) {
            out = (int)idx;
            return true;
        }
    }
    return false;
}

bool EnvFlagEnabled(const char* key) {
    if (const char* raw = std::getenv(key)) {
        const std::string v = raw;
        return v == "1" || v == "true" || v == "TRUE" || v == "yes" || v == "YES";
    }
    return false;
}

int EnvIntOrDefault(const char* key, int defaultValue, int minValue, int maxValue) {
    if (const char* raw = std::getenv(key)) {
        char* end = nullptr;
        long value = std::strtol(raw, &end, 10);
        if (end != raw && *end == '\0') {
            if (value < minValue) value = minValue;
            if (value > maxValue) value = maxValue;
            return (int)value;
        }
    }
    return defaultValue;
}

double EnvDoubleOrDefault(const char* key, double defaultValue, double minValue, double maxValue) {
    if (const char* raw = std::getenv(key)) {
        char* end = nullptr;
        double value = std::strtod(raw, &end);
        if (end != raw && *end == '\0') {
            if (value < minValue) value = minValue;
            if (value > maxValue) value = maxValue;
            return value;
        }
    }
    return defaultValue;
}

DWORD PriorityClassFromLevel(int level) {
    switch (level) {
        case 2: return HIGH_PRIORITY_CLASS;
        case 1: return ABOVE_NORMAL_PRIORITY_CLASS;
        default: return NORMAL_PRIORITY_CLASS;
    }
}

const char* PriorityLevelName(int level) {
    switch (level) {
        case 2: return "high";
        case 1: return "above-normal";
        default: return "normal";
    }
}

bool HasTextLikePrepass(
    const cv::Mat& frame,
    int minHits,
    double minCoverage,
    double minWidthRatio,
    int& outHits,
    double& outCoverage
) {
    outHits = 0;
    outCoverage = 0.0;
    if (frame.empty()) return false;

    cv::Mat img = frame;
    const int maxSide = std::max(frame.cols, frame.rows);
    if (maxSide > 960) {
        const double s = 960.0 / (double)maxSide;
        cv::resize(frame, img, cv::Size(), s, s, cv::INTER_AREA);
    }

    const int w = img.cols;
    const int h = img.rows;
    if (w < 40 || h < 40) return false;

    // Focus on the central area where phone text is expected.
    const int x0 = (int)(w * 0.20);
    const int y0 = (int)(h * 0.10);
    const int x1 = (int)(w * 0.80);
    const int y1 = (int)(h * 0.92);
    const cv::Rect roiRect(x0, y0, std::max(1, x1 - x0), std::max(1, y1 - y0));
    const cv::Mat roi = img(roiRect);

    cv::Mat gray;
    cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);

    cv::Mat maskGlobal, maskLocal, mask;
    cv::threshold(gray, maskGlobal, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::adaptiveThreshold(
        gray, maskLocal, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 31, -2
    );
    cv::bitwise_and(maskGlobal, maskLocal, mask);
    cv::morphologyEx(
        mask, mask, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)), cv::Point(-1, -1), 1
    );

    cv::Mat linked;
    cv::morphologyEx(
        mask, linked, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(17, 5)), cv::Point(-1, -1), 1
    );

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(linked, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return false;

    const double areaAll = (double)roi.rows * (double)roi.cols;
    struct Box {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        double area = 0.0;
        double cy = 0.0;
    };
    std::vector<Box> accepted;
    accepted.reserve(contours.size());

    for (const auto& cnt : contours) {
        const cv::Rect b = cv::boundingRect(cnt);
        const double area = (double)b.width * (double)b.height;
        if (area < areaAll * 0.00025) continue;
        if (area > areaAll * 0.35) continue;
        if (b.height < 4 || b.height > roi.rows * 0.30) continue;
        if (b.x <= 1 || b.y <= 1 || (b.x + b.width) >= (roi.cols - 1) || (b.y + b.height) >= (roi.rows - 1)) continue;

        const cv::Mat box = mask(b);
        const double inkRatio = (double)cv::countNonZero(box) / std::max(1, b.width * b.height);
        if (inkRatio < 0.03 || inkRatio > 0.78) continue;

        Box bx;
        bx.x = b.x;
        bx.y = b.y;
        bx.w = b.width;
        bx.h = b.height;
        bx.area = area;
        bx.cy = b.y + b.height * 0.5;
        accepted.push_back(bx);
    }

    if (accepted.empty()) {
        outHits = 0;
        outCoverage = 0.0;
        return false;
    }

    int bestHits = 0;
    double bestCoverage = 0.0;
    double bestWidthRatio = 0.0;

    // Require at least one line-like cluster with enough width/coverage.
    for (size_t i = 0; i < accepted.size(); ++i) {
        const auto& a = accepted[i];
        const double rowTol = std::max(12.0, (double)a.h * 1.4);
        int hits = 0;
        int ux0 = INT_MAX;
        int ux1 = INT_MIN;
        double clusterArea = 0.0;

        for (const auto& b : accepted) {
            if (std::fabs(b.cy - a.cy) > rowTol) continue;
            hits++;
            ux0 = std::min(ux0, b.x);
            ux1 = std::max(ux1, b.x + b.w);
            clusterArea += b.area;
        }

        if (hits <= 0 || ux0 >= ux1) continue;
        const double widthRatio = (double)(ux1 - ux0) / std::max(1, roi.cols);
        const double coverage = clusterArea / std::max(1.0, areaAll);
        const double cyNorm = a.cy / std::max(1.0, (double)roi.rows);
        if (cyNorm < 0.08 || cyNorm > 0.98) continue;

        if (hits > bestHits ||
            (hits == bestHits && widthRatio > bestWidthRatio) ||
            (hits == bestHits && std::abs(widthRatio - bestWidthRatio) < 1e-6 && coverage > bestCoverage)) {
            bestHits = hits;
            bestWidthRatio = widthRatio;
            bestCoverage = coverage;
        }
    }

    outHits = bestHits;
    outCoverage = bestCoverage;
    return bestHits >= minHits && bestCoverage >= minCoverage && bestWidthRatio >= minWidthRatio;
}

std::vector<int> BuildCameraPriorityList(const std::vector<std::wstring>& names, int preferredFromNames) {
    std::vector<int> priority;
    std::set<int> seen;
    auto addUnique = [&](int idx) {
        if (idx < 0) return;
        if (seen.insert(idx).second) priority.push_back(idx);
    };

    int forcedIndex = -1;
    if (TryGetEnvIndex("TESS_CAMERA_INDEX", forcedIndex)) {
        addUnique(forcedIndex);
    }

    if (!names.empty()) {
        addUnique(preferredFromNames);

        for (size_t i = 0; i < names.size(); ++i) {
            if (!IsVirtualCameraName(names[i])) {
                addUnique((int)i);
            }
        }
        for (size_t i = 0; i < names.size(); ++i) {
            addUnique((int)i);
        }
    } else {
        addUnique(0);
        addUnique(1);
        addUnique(2);
        addUnique(3);
    }
    return priority;
}

bool ProbeFrame(cv::VideoCapture& cap, cv::Mat& frame) {
    for (int i = 0; i < 4; ++i) {
        if (cap.read(frame) && !frame.empty()) {
            return true;
        }
        Sleep(5);
    }
    return false;
}

bool TryOpenCamera(cv::VideoCapture& cap, int cameraIndex, int api, const char* apiName, cv::Mat& probeFrame, bool requestHighRes) {
    cap.release();

    const bool opened = (api == cv::CAP_ANY) ? cap.open(cameraIndex) : cap.open(cameraIndex, api);
    if (!opened) {
        std::cerr << "Camera open failed: index " << cameraIndex << " via " << apiName << "\n";
        return false;
    }

    if (!ProbeFrame(cap, probeFrame)) {
        std::cerr << "Camera opened but no frames: index " << cameraIndex << " via " << apiName << "\n";
        cap.release();
        return false;
    }

    // Set desired resolution; read one frame to confirm it took effect.
    if (requestHighRes) {
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    } else {
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    }

    // Quick single-read check — the camera already proved it can produce frames.
    if (!cap.read(probeFrame) || probeFrame.empty()) {
        if (requestHighRes) {
            std::cerr << "1080p unstable on index " << cameraIndex << " via " << apiName
                      << ", falling back to 1280x720\n";
            cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
            if (!cap.read(probeFrame) || probeFrame.empty()) {
                std::cerr << "Camera stream failed after resolution fallback: index " << cameraIndex
                          << " via " << apiName << "\n";
                cap.release();
                return false;
            }
        } else {
            std::cerr << "Camera stream failed at 720p: index " << cameraIndex
                      << " via " << apiName << "\n";
            cap.release();
            return false;
        }
    }

    std::cerr << "Camera ready: index " << cameraIndex << " via " << apiName << "\n";
    return true;
}

void ResetCameraToAuto(cv::VideoCapture& cap) {
    // 0.75 = auto exposure in DirectShow convention (0.25 = manual).
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.75);
    cap.set(cv::CAP_PROP_AUTOFOCUS, 1);
    cap.set(cv::CAP_PROP_AUTO_WB, 1);
    std::cerr << "Camera reset to auto (exposure/focus/WB)\n";
}

struct CameraCandidate {
    int index = -1;
    int api = cv::CAP_ANY;
    const char* backend = "";
    int width = 0;
    int height = 0;
    double score = -1.0;
    bool preferredByName = false;
    bool knownByName = false;
    bool virtualByName = false;
    bool valid = false;
};

double ScoreCandidate(int index, int w, int h, int preferredIndex, bool backendDshow) {
    double score = 0.0;
    score += (double)w * (double)h / 1000.0; // prefer higher native resolution
    if (w >= 1900 && h >= 1000) score += 5000.0;
    if (backendDshow) score += 500.0;
    if (index == preferredIndex) score += 200.0;
    return score;
}

std::string NormalizeText(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    bool prevSpace = false;
    for (unsigned char ch : s) {
        if (std::isspace(ch)) {
            if (!out.empty() && !prevSpace) {
                out.push_back(' ');
            }
            prevSpace = true;
        } else {
            out.push_back((char)ch);
            prevSpace = false;
        }
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    if (!out.empty() && out.front() == ' ') out.erase(out.begin());
    return out;
}

int TextScore(const std::string& s) {
    int score = 0;
    for (unsigned char ch : s) {
        if (!std::isspace(ch)) score++;
    }
    return score;
}

// Bounded Levenshtein distance. Returns maxDist+1 if distance exceeds maxDist.
int LevenshteinBounded(const std::string& a, const std::string& b, int maxDist) {
    const int la = (int)a.size(), lb = (int)b.size();
    if (std::abs(la - lb) > maxDist) return maxDist + 1;
    // Two-row DP
    std::vector<int> prev(lb + 1), curr(lb + 1);
    for (int j = 0; j <= lb; ++j) prev[j] = j;
    for (int i = 1; i <= la; ++i) {
        curr[0] = i;
        int rowMin = curr[0];
        for (int j = 1; j <= lb; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            curr[j] = std::min({prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost});
            if (curr[j] < rowMin) rowMin = curr[j];
        }
        if (rowMin > maxDist) return maxDist + 1;
        std::swap(prev, curr);
    }
    return prev[lb] <= maxDist ? prev[lb] : maxDist + 1;
}

// Character-position voting across cluster variants. Picks the most common char
// at each position, weighted by OCR confidence. Only considers variants whose
// length matches the most frequent length in the cluster.
std::string PositionVote(const std::vector<std::pair<std::string, float>>& variants) {
    if (variants.size() < 2) {
        return variants.empty() ? std::string{} : variants[0].first;
    }
    // Find most common length
    std::unordered_map<size_t, int> lenCounts;
    for (const auto& [s, c] : variants) lenCounts[s.size()]++;
    size_t targetLen = 0;
    int maxCount = 0;
    for (const auto& [len, cnt] : lenCounts) {
        if (cnt > maxCount || (cnt == maxCount && len > targetLen)) {
            maxCount = cnt;
            targetLen = len;
        }
    }
    // Filter to that length
    std::vector<const std::pair<std::string, float>*> filtered;
    for (const auto& v : variants) {
        if (v.first.size() == targetLen) filtered.push_back(&v);
    }
    if (filtered.empty()) return variants[0].first;
    if (filtered.size() == 1) return filtered[0]->first;
    // Vote per position
    std::string result(targetLen, ' ');
    for (size_t pos = 0; pos < targetLen; ++pos) {
        std::unordered_map<char, float> votes;
        for (const auto* v : filtered) {
            votes[(unsigned char)v->first[pos]] += v->second;
        }
        float bestW = -1.0f;
        char bestCh = ' ';
        for (const auto& [ch, w] : votes) {
            if (w > bestW) { bestW = w; bestCh = ch; }
        }
        result[pos] = bestCh;
    }
    return result;
}

} // namespace

struct OcrProcess {
    struct Candidate {
        std::string text;
        float conf = 0.0f;
    };

    HANDLE stdinWr = INVALID_HANDLE_VALUE;
    HANDLE stdoutRd = INVALID_HANDLE_VALUE;
    PROCESS_INFORMATION pi{};

    static std::string resolveOcrScript() {
        // 1. Check TESS_OCR_SCRIPT env var
        const char* envScript = std::getenv("TESS_OCR_SCRIPT");
        if (envScript && envScript[0]) return std::string(envScript);

        // 2. Look next to the running executable
        char exePath[MAX_PATH] = {};
        DWORD n = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        if (n > 0 && n < MAX_PATH) {
            std::filesystem::path candidate = std::filesystem::path(exePath).parent_path() / "ocr.py";
            if (std::filesystem::exists(candidate)) return candidate.string();
        }

        // 3. Fallback to current directory
        return "ocr.py";
    }

    bool start(DWORD priorityClass) {
        SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};

        HANDLE stdinRd, stdoutWr;
        CreatePipe(&stdinRd, &stdinWr, &sa, 0);
        CreatePipe(&stdoutRd, &stdoutWr, &sa, 0);
        SetHandleInformation(stdinWr, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(stdoutRd, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = stdinRd;
        si.hStdOutput = stdoutWr;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        const std::string pythonExe = []() -> std::string {
            const char* env = std::getenv("TESS_OCR_PYTHON");
            return (env && env[0]) ? std::string(env) : "python";
        }();
        std::string cmdStr = pythonExe + " \"" + resolveOcrScript() + "\"";
        std::vector<char> cmd(cmdStr.begin(), cmdStr.end());
        cmd.push_back('\0');

        if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, TRUE, priorityClass, nullptr, nullptr, &si, &pi)) {
            std::cerr << "Failed to launch python ocr.py\n";
            CloseHandle(stdinRd); CloseHandle(stdinWr);
            CloseHandle(stdoutRd); CloseHandle(stdoutWr);
            stdinWr = INVALID_HANDLE_VALUE;
            return false;
        }

        CloseHandle(stdinRd);
        CloseHandle(stdoutWr);
        return true;
    }

    Candidate recognize(const std::vector<uchar>& pngBuf) {
        // Send 4-byte size + PNG data
        uint32_t size = (uint32_t)pngBuf.size();
        DWORD written;
        if (!WriteFile(stdinWr, &size, 4, &written, nullptr) || written != 4) {
            return Candidate{};
        }

        const uchar* ptr = pngBuf.data();
        DWORD remaining = size;
        while (remaining > 0) {
            DWORD chunk = remaining > 65536 ? 65536 : remaining;
            if (!WriteFile(stdinWr, ptr, chunk, &written, nullptr) || written == 0) {
                return Candidate{};
            }
            ptr += written;
            remaining -= written;
        }

        // Read lines until empty line
        Candidate best;
        double bestScore = -1.0;
        std::string line;
        char ch;
        DWORD bytesRead;
        while (ReadFile(stdoutRd, &ch, 1, &bytesRead, nullptr) && bytesRead > 0) {
            if (ch == '\n') {
                if (line.empty()) break;  // empty line = end of result

                Candidate cand;
                size_t prefixLen = 0;
                if (line.rfind("FINAL\t", 0) == 0) {
                    prefixLen = 6;
                } else if (line.rfind("CAND\t", 0) == 0) {
                    // Backward compatibility with older ocr.py.
                    prefixLen = 5;
                }

                if (prefixLen > 0) {
                    size_t p1 = line.find('\t', prefixLen);
                    if (p1 != std::string::npos) {
                        const std::string confStr = line.substr(prefixLen, p1 - prefixLen);
                        cand.text = line.substr(p1 + 1);
                        try { cand.conf = std::stof(confStr); }
                        catch (...) { cand.conf = 0.0f; }
                    } else {
                        cand.text = line.substr(prefixLen);
                        cand.conf = 0.0f;
                    }
                } else {
                    cand.text = line;
                    cand.conf = 0.0f;
                }

                const int lenScore = TextScore(cand.text);
                const double score = (double)lenScore * 1.8 + (double)cand.conf * 8.0;
                if (score > bestScore) {
                    bestScore = score;
                    best = cand;
                }
                line.clear();
            } else if (ch != '\r') {
                line += ch;
            }
        }

        return best;
    }

    void stop(bool forceTerminate = false) {
        if (stdinWr != INVALID_HANDLE_VALUE) {
            CloseHandle(stdinWr);
            stdinWr = INVALID_HANDLE_VALUE;
        }
        if (stdoutRd != INVALID_HANDLE_VALUE) {
            CloseHandle(stdoutRd);
            stdoutRd = INVALID_HANDLE_VALUE;
        }
        if (pi.hProcess) {
            DWORD waitMs = forceTerminate ? 200 : 5000;
            DWORD wr = WaitForSingleObject(pi.hProcess, waitMs);
            if (wr == WAIT_TIMEOUT && forceTerminate) {
                TerminateProcess(pi.hProcess, 1);
                WaitForSingleObject(pi.hProcess, 500);
            }
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            pi = {};
        }
    }
};

struct AsyncOcr {
    using Candidate = OcrProcess::Candidate;

    std::vector<OcrProcess> procs;
    std::vector<std::thread> workers;
    std::vector<unsigned char> workerBusy;
    std::mutex mu;
    std::condition_variable cv;

    bool stopRequested = false;
    bool preserveOrder = false;
    std::deque<std::vector<uchar>> queue;
    std::deque<Candidate> results;
    static constexpr size_t kMaxQueueRealtime = 5;
    static constexpr size_t kMaxQueueBurst = 128;
    static constexpr int kMaxWorkers = 8;
    DWORD workerPriorityClass = NORMAL_PRIORITY_CLASS;

    bool warmupModel(OcrProcess& proc, const std::vector<uchar>& pngBuf) {
        // Prime OCR model/runtime without consuming a real webcam frame.
        if (pngBuf.empty()) return false;
        (void)proc.recognize(pngBuf);
        return true;
    }

    bool start(int workerCount, int preloadWorkers, DWORD priorityClass) {
        if (workerCount < 1) workerCount = 1;
        if (workerCount > kMaxWorkers) workerCount = kMaxWorkers;
        if (preloadWorkers < 0) preloadWorkers = 0;
        if (preloadWorkers > workerCount) preloadWorkers = workerCount;
        workerPriorityClass = priorityClass;

        procs.reserve(workerCount);
        workers.reserve(workerCount);
        workerBusy.assign((size_t)workerCount, 0);
        stopRequested = false;

        cv::Mat warmup(96, 96, CV_8UC3, cv::Scalar(0, 0, 0));
        std::vector<uchar> warmupPng;
        if (preloadWorkers > 0 && !cv::imencode(".png", warmup, warmupPng)) {
            std::cerr << "OCR warmup frame encode failed; skipping preload.\n";
            preloadWorkers = 0;
        }

        for (int i = 0; i < workerCount; ++i) {
            procs.emplace_back();
            if (!procs.back().start(workerPriorityClass)) {
                std::cerr << "OCR worker " << i << " failed to start.\n";
                shutdown();
                return false;
            }
            if (i < preloadWorkers && !warmupModel(procs.back(), warmupPng)) {
                std::cerr << "OCR worker " << i << " warmup frame encode failed; continuing.\n";
            }
        }
        std::cerr << "OCR workers ready: workers=" << workerCount
                  << " preload=" << preloadWorkers << "/" << workerCount
                  << " priorityClass=" << workerPriorityClass << "\n";

        for (int i = 0; i < workerCount; ++i) {
            workers.emplace_back([this, i]() { runWorker(i); });
        }
        return true;
    }

    void runWorker(int workerIndex) {
        for (;;) {
            std::vector<uchar> localPng;
            {
                std::unique_lock<std::mutex> lock(mu);
                cv.wait(lock, [&]() { return stopRequested || !queue.empty(); });
                if (stopRequested && queue.empty()) break;

                if (preserveOrder) {
                    localPng = std::move(queue.front());
                    queue.pop_front();
                } else {
                    // Keep newest frame; drop stale backlog.
                    localPng = std::move(queue.back());
                    queue.clear();
                }
                workerBusy[(size_t)workerIndex] = 1;
            }

            Candidate cand = procs[(size_t)workerIndex].recognize(localPng);

            {
                std::lock_guard<std::mutex> lock(mu);
                results.push_back(std::move(cand));
                workerBusy[(size_t)workerIndex] = 0;
            }
        }
    }

    bool submit(std::vector<uchar>&& pngBuf) {
        std::lock_guard<std::mutex> lock(mu);
        const size_t maxQueue = preserveOrder ? kMaxQueueBurst : kMaxQueueRealtime;
        if (queue.size() >= maxQueue) {
            queue.pop_front();
        }
        queue.push_back(std::move(pngBuf));
        cv.notify_one();
        return true;
    }

    bool isBusy() {
        std::lock_guard<std::mutex> lock(mu);
        if (!queue.empty()) return true;
        for (unsigned char b : workerBusy) {
            if (b) return true;
        }
        return false;
    }

    bool hasPendingWork() {
        std::lock_guard<std::mutex> lock(mu);
        if (!queue.empty() || !results.empty()) return true;
        for (unsigned char b : workerBusy) {
            if (b) return true;
        }
        return false;
    }

    void setPreserveOrder(bool value) {
        std::lock_guard<std::mutex> lock(mu);
        preserveOrder = value;
    }

    bool pollResult(Candidate& out) {
        std::lock_guard<std::mutex> lock(mu);
        if (results.empty()) return false;
        out = std::move(results.front());
        results.pop_front();
        return true;
    }

    void shutdown(bool forceTerminate = false) {
        {
            std::lock_guard<std::mutex> lock(mu);
            stopRequested = true;
            cv.notify_all();
        }
        for (auto& w : workers) {
            if (w.joinable()) w.join();
        }
        workers.clear();
        for (auto& p : procs) {
            p.stop(forceTerminate);
        }
        procs.clear();
        workerBusy.clear();
        queue.clear();
        results.clear();
    }
};

int tess_webcam_ocr_run(int argc, char** argv) {
    bool noExitPrompt = EnvFlagEnabled("TESS_NO_EXIT_PROMPT");
    for (int i = 1; i < argc; ++i) {
        if (!argv[i]) continue;
        const std::string arg = argv[i];
        if (arg == "--no-prompt") noExitPrompt = true;
    }

    // Avoid a common MSMF startup failure mode on some UVC webcams.
    _putenv("OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS=0");

    const int appPriorityLevel = EnvIntOrDefault("TESS_APP_PRIORITY_LEVEL", 1, 0, 2);
    const DWORD appPriorityClass = PriorityClassFromLevel(appPriorityLevel);
    if (appPriorityClass != NORMAL_PRIORITY_CLASS) {
        if (SetPriorityClass(GetCurrentProcess(), appPriorityClass)) {
            std::cerr << "Process priority: " << PriorityLevelName(appPriorityLevel) << "\n";
        } else {
            std::cerr << "Process priority request failed; continuing at normal priority.\n";
        }
    }

    AsyncOcr ocr;
    const int ocrWorkers = EnvIntOrDefault("TESS_OCR_WORKERS", 1, 1, 8);
    const int ocrPreloadWorkers = EnvIntOrDefault("TESS_OCR_PRELOAD_WORKERS", 1, 0, 8);
    const int ocrPriorityLevel = EnvIntOrDefault("TESS_OCR_PRIORITY_LEVEL", 1, 0, 2);
    const DWORD ocrPriorityClass = PriorityClassFromLevel(ocrPriorityLevel);
    std::cerr << "Starting OCR workers in background: workers=" << ocrWorkers
              << " preload=" << ocrPreloadWorkers
              << " priority=" << PriorityLevelName(ocrPriorityLevel) << "\n";
    auto ocrStartFuture = std::async(std::launch::async, [&]() {
        return ocr.start(ocrWorkers, ocrPreloadWorkers, ocrPriorityClass);
    });
    auto failAndShutdownOcr = [&](int code) {
        if (ocrStartFuture.valid()) {
            bool started = false;
            try {
                started = ocrStartFuture.get();
            } catch (...) {
                started = false;
            }
            if (started) {
                ocr.shutdown(true);
            }
        }
        return code;
    };

    const auto names = EnumerateVideoDeviceNamesDShow();
    const int preferredByName = names.empty() ? -1 : ChooseCameraIndexFromNames(names, true);
    const auto cameraPriority = BuildCameraPriorityList(names, preferredByName);
    const int preferredIndexHint = preferredByName >= 0 ? preferredByName
                                                         : (cameraPriority.empty() ? -1 : cameraPriority.front());
    int forcedIndex = -1;
    const bool hasForcedIndex = TryGetEnvIndex("TESS_CAMERA_INDEX", forcedIndex);
    const bool allowVirtualFallback = EnvFlagEnabled("TESS_ALLOW_VIRTUAL_CAMERA");
    const bool allowObsCamera = EnvFlagEnabled("TESS_ALLOW_OBS_CAMERA");
    const bool quickCameraSelect = !EnvFlagEnabled("TESS_DISABLE_CAMERA_QUICK_SELECT");
    cv::VideoCapture cap;
    cv::Mat frame;

    struct BackendTry {
        int api;
        const char* name;
    };
    const std::vector<BackendTry> backendOrder = {
        {cv::CAP_DSHOW, "DSHOW"},
    };

    std::vector<CameraCandidate> candidates;
    CameraCandidate chosen;
    bool chooseOk = false;
    bool cameraOpenForChosen = false;
    bool quickCameraHit = false;
    for (int idx : cameraPriority) {
        for (const auto& backend : backendOrder) {
            const bool wantHighRes = true;
            if (!TryOpenCamera(cap, idx, backend.api, backend.name, frame, wantHighRes)) {
                continue;
            }

            const int w = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
            const int h = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
            const bool backendDshow = backend.api == cv::CAP_DSHOW;
            const double score = ScoreCandidate(idx, w, h, preferredIndexHint, backendDshow);

            std::cerr << "Candidate camera: index " << idx << " via " << backend.name
                      << " @" << w << "x" << h << " score=" << score << "\n";

            CameraCandidate cand;
            cand.index = idx;
            cand.api = backend.api;
            cand.backend = backend.name;
            cand.width = w;
            cand.height = h;
            cand.score = score;
            cand.knownByName = idx >= 0 && idx < (int)names.size();
            cand.virtualByName = cand.knownByName ? IsVirtualCameraName(names[idx]) : false;
            cand.preferredByName = cand.knownByName && (idx == preferredByName);
            cand.valid = true;
            candidates.push_back(cand);

            const bool quickForcedHit = hasForcedIndex && idx == forcedIndex;
            const bool quickPreferredHit = !hasForcedIndex && preferredByName >= 0 && idx == preferredByName;
            if (quickCameraSelect && (quickForcedHit || quickPreferredHit)) {
                chosen = cand;
                chooseOk = true;
                cameraOpenForChosen = true;
                quickCameraHit = true;
                std::cerr << "Quick camera select: index " << idx
                          << " via " << backend.name << "\n";
                break;
            }

            cap.release();
            frame.release();
        }
        if (quickCameraHit) break;
    }

    if (candidates.empty()) {
        std::cerr << "Could not open webcam\n";
        return failAndShutdownOcr(1);
    }

    auto chooseBest = [&](auto predicate) {
        bool found = false;
        CameraCandidate bestLocal;
        for (const auto& c : candidates) {
            if (!predicate(c)) continue;
            if (!found || c.score > bestLocal.score) {
                bestLocal = c;
                found = true;
            }
        }
        if (found) chosen = bestLocal;
        return found;
    };

    if (!chooseOk) {
        if (hasForcedIndex) {
            chooseOk = chooseBest([&](const CameraCandidate& c) {
                return c.index == forcedIndex;
            });
            if (!chooseOk) {
                std::cerr << "Forced camera index TESS_CAMERA_INDEX=" << forcedIndex
                          << " was not available.\n";
            }
        } else {
            chooseOk = chooseBest([&](const CameraCandidate& c) {
                return c.preferredByName;
            });
            if (!chooseOk) {
                chooseOk = chooseBest([&](const CameraCandidate& c) {
                    return c.knownByName && !c.virtualByName;
                });
            }
            if (!chooseOk) {
                chooseOk = chooseBest([&](const CameraCandidate& c) {
                    return !c.knownByName;
                });
            }
            if (!chooseOk && allowVirtualFallback) {
                chooseOk = chooseBest([&](const CameraCandidate& c) {
                    if (!c.virtualByName) return false;
                    if (!c.knownByName) return false;
                    if (IsObsCameraName(names[c.index]) && !allowObsCamera) return false;
                    return true;
                });
            }
        }
    }

    if (!chooseOk) {
        std::cerr << "Razer Kiyo was detected by name but no non-virtual stream was usable.\n"
                  << "Close/disable Camo and OBS Virtual Camera, then retry.\n"
                  << "If you still want virtual fallback, set TESS_ALLOW_VIRTUAL_CAMERA=1.\n"
                  << "OBS is blocked by default; set TESS_ALLOW_OBS_CAMERA=1 to allow it.\n";
        return failAndShutdownOcr(1);
    }

    if (!cameraOpenForChosen &&
        !TryOpenCamera(cap, chosen.index, chosen.api, chosen.backend, frame, true)) {
        std::cerr << "Selected camera could not be reopened for capture.\n";
        return failAndShutdownOcr(1);
    }

    std::cerr << "Using camera index " << chosen.index << " via " << chosen.backend
              << " @" << chosen.width << "x" << chosen.height;
    if (chosen.knownByName) {
        std::wcerr << L" name=\"" << names[chosen.index] << L"\"";
    }
    std::cerr << "\n";

    // Start the webcam window immediately so the user sees live video while
    // OCR workers are still preloading in the background.
    const int cameraWarmupMs = EnvIntOrDefault("TESS_CAMERA_WARMUP_MS", 250, 0, 5000);
    if (cameraWarmupMs > 0) {
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(cameraWarmupMs)) {
            cap >> frame;
            if (frame.empty()) break;
            cv::imshow("webcam", frame);
            int key = cv::waitKey(1);
            if (key == 27) { frame = cv::Mat(); break; }
        }
    }

    // Auto-focus the webcam window so the user can press Enter immediately.
    {
        HWND hwnd = FindWindowA(nullptr, "webcam");
        if (hwnd) {
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
        }
    }

    // Now wait for OCR workers to finish preloading.
    if (!ocrStartFuture.get()) return 1;

    std::cerr << "OCR ready. Camera stays live.\n";

    std::string bestText;
    int bestScore = 0;
    float bestConf = 0.0f;
    int bestLen = 0;
    std::string bestRawText;
    int bestRawScore = 0;
    float bestRawConf = 0.0f;
    int bestRawLen = 0;
    struct TextAggregate {
        int hits = 0;
        double scoreSum = 0.0;
        float bestConf = 0.0f;
        int bestLen = 0;
        std::string bestText;
        std::vector<std::pair<std::string, float>> variants;
    };
    struct TurboHit {
        std::string text;
        float conf = 0.0f;
        int len = 0;
        std::chrono::steady_clock::time_point t{};
    };
    std::unordered_map<std::string, TextAggregate> textAgg;
    std::deque<TurboHit> turboHistory;
    std::string prevText;
    int stableHits = 0;
    int seenTextFrames = 0;
    bool emittedFinalText = false;
    std::string finalText;
    auto escapeForQuoted = [](const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char ch : s) {
            if (ch == '\\' || ch == '"') out.push_back('\\');
            out.push_back(ch);
        }
        return out;
    };
    auto emitFinal = [&](const std::string& text) {
        finalText = text;
        emittedFinalText = true;
        std::cout << "Final string: \"" << escapeForQuoted(finalText) << "\"\n";
    };
    const int kMinStableHits = EnvIntOrDefault("TESS_MIN_STABLE_HITS", 2, 1, 5);
    const int kMaxSeenTextFrames = EnvIntOrDefault("TESS_MAX_TEXT_FRAMES", 16, 4, 200);
    const int kMinSharpness = EnvIntOrDefault("TESS_MIN_SHARPNESS", 40, 0, 2000);
    const int kMaxRunMs = EnvIntOrDefault("TESS_MAX_RUN_MS", 12000, 1000, 120000);
    const int kMinOutputChars = EnvIntOrDefault("TESS_MIN_OUTPUT_CHARS", 5, 1, 64);
    const bool kRequireTextPrepass = !EnvFlagEnabled("TESS_DISABLE_TEXT_PREPASS");
    const int kPrepassMinHits = EnvIntOrDefault("TESS_PREPASS_MIN_HITS", 3, 1, 16);
    const double kPrepassMinCoverage = EnvDoubleOrDefault("TESS_PREPASS_MIN_COVERAGE", 0.0020, 0.0, 1.0);
    const double kPrepassMinWidthRatio = EnvDoubleOrDefault("TESS_PREPASS_MIN_WIDTH_RATIO", 0.28, 0.0, 1.0);
    const int kPrepassStreak = EnvIntOrDefault("TESS_PREPASS_STREAK", 2, 1, 16);
    const int kPrepassEvalMs = EnvIntOrDefault("TESS_PREPASS_EVAL_MS", 50, 10, 2000);
    const int kTurboDurationMs = EnvIntOrDefault("TESS_TURBO_DURATION_MS", 4000, 0, 60000);
    const int kTurboSubmitMs = EnvIntOrDefault("TESS_TURBO_SUBMIT_MS", 35, 10, 1000);
    const int kTurboPrepassStreak = EnvIntOrDefault("TESS_TURBO_PREPASS_STREAK", 1, 1, 16);
    const int kTurboTriggerLen = EnvIntOrDefault("TESS_TURBO_TRIGGER_LEN", 5, 1, 64);
    const float kTurboTriggerConf = (float)EnvDoubleOrDefault("TESS_TURBO_TRIGGER_CONF", 0.38, 0.0, 1.0);
    const bool kTurboAllowBusySubmit = !EnvFlagEnabled("TESS_DISABLE_TURBO_BUSY_SUBMIT");
    const int kTurboHistoryMs = EnvIntOrDefault("TESS_TURBO_HISTORY_MS", 1000, 200, 10000);
    const int kTurboHistoryCalls = EnvIntOrDefault("TESS_TURBO_HISTORY_CALLS", 5, 1, 64);
    const int kTurboHistoryMaxHits = EnvIntOrDefault("TESS_TURBO_HISTORY_MAX_HITS", 24, 4, 256);
    const bool kEnterTriggerMode = !EnvFlagEnabled("TESS_DISABLE_ENTER_TRIGGER");
    const int kEnterCaptureMs = EnvIntOrDefault("TESS_ENTER_CAPTURE_MS", 800, 200, 20000);
    const int kEnterFrameSubmitMs = EnvIntOrDefault("TESS_ENTER_FRAME_MS", 100, 20, 1000);
    const int kEnterCaptureFrames = EnvIntOrDefault("TESS_ENTER_CAPTURE_FRAMES", 3, 1, 256);
    const int kEnterDrainMinResults = EnvIntOrDefault("TESS_ENTER_DRAIN_MIN_RESULTS", 2, 1, 64);
    const int kEnterFirstResultMaxMs = EnvIntOrDefault("TESS_ENTER_FIRST_RESULT_MAX_MS", 8000, 500, 120000);
    const float kMinAcceptConf = (float)EnvDoubleOrDefault("TESS_MIN_ACCEPT_CONF", 0.45, 0.0, 1.0);
    const int kMinConsensusHits = EnvIntOrDefault("TESS_MIN_CONSENSUS_HITS", 2, 1, 10);
    const int kMaxEditDist = EnvIntOrDefault("TESS_MAX_EDIT_DIST", 2, 0, 5);
    const double kMaxGlareRatio = EnvDoubleOrDefault("TESS_MAX_GLARE_RATIO", 0.15, 0.0, 1.0);
    const int kOcrMaxSide = 1280;
    const float kStableConf = 0.35f;
    const float kOneShotConf = (float)EnvDoubleOrDefault("TESS_ONE_SHOT_CONF", 0.90, 0.0, 1.0);
    const int kOneShotMinLen = 6;
    auto lastSubmit = std::chrono::steady_clock::now() - std::chrono::milliseconds(1000);
    const auto loopStart = std::chrono::steady_clock::now();
    const int submitIntervalMs = EnvIntOrDefault("TESS_OCR_SUBMIT_MS", 80, 25, 1000);
    const int idleSubmitIntervalMs = EnvIntOrDefault("TESS_OCR_IDLE_SUBMIT_MS", 240, 50, 5000);
    const auto kSubmitInterval = std::chrono::milliseconds(submitIntervalMs);
    const auto kIdleSubmitInterval = std::chrono::milliseconds(idleSubmitIntervalMs);
    const auto kTurboSubmitInterval = std::chrono::milliseconds(kTurboSubmitMs);
    const auto kTurboDuration = std::chrono::milliseconds(kTurboDurationMs);
    const auto kTurboHistoryWindow = std::chrono::milliseconds(kTurboHistoryMs);
    const auto kPrepassEvalInterval = std::chrono::milliseconds(kPrepassEvalMs);
    const int kEnterMaxWaitMs = EnvIntOrDefault(
        "TESS_ENTER_MAX_WAIT_MS",
        std::max(kEnterCaptureMs, kEnterFrameSubmitMs * kEnterCaptureFrames * 4),
        500, 120000
    );
    const int kEnterDrainMaxMs = EnvIntOrDefault("TESS_ENTER_DRAIN_MAX_MS", 2500, 200, 60000);
    const auto kEnterMaxWaitWindow = std::chrono::milliseconds(kEnterMaxWaitMs);
    const auto kEnterFirstResultMaxWindow = std::chrono::milliseconds(kEnterFirstResultMaxMs);
    const auto kEnterDrainMaxWindow = std::chrono::milliseconds(kEnterDrainMaxMs);
    const auto kEnterFrameInterval = std::chrono::milliseconds(kEnterFrameSubmitMs);
    auto lastPrepassEval = std::chrono::steady_clock::now() - std::chrono::milliseconds(1000);
    auto turboUntil = std::chrono::steady_clock::time_point::min();
    bool turboBatchActive = false;
    auto turboBatchStart = std::chrono::steady_clock::time_point::min();
    int turboBatchCalls = 0;
    int prepassHitStreak = 0;
    bool cachedTextLike = !kRequireTextPrepass;
    int cachedPrepassHits = 0;
    double cachedPrepassCoverage = 0.0;
    bool manualTriggerActive = !kEnterTriggerMode;
    bool enterBurstCapturing = false;
    auto enterBurstStart = std::chrono::steady_clock::time_point::min();
    auto enterBurstLastSubmit = std::chrono::steady_clock::time_point::min();
    auto enterDrainStart = std::chrono::steady_clock::time_point::min();
    int enterSubmittedFrames = 0;
    int enterAcceptedResults = 0;
    bool enterSawAnyResult = false;
    bool forceTerminateOcr = false;

    std::cerr << "OCR submit interval: " << submitIntervalMs << " ms"
              << " | idleSubmit=" << idleSubmitIntervalMs << " ms"
              << " | sharpness>=" << kMinSharpness
              << " | minChars=" << kMinOutputChars
              << " | prepass=" << (kRequireTextPrepass ? "on" : "off")
              << " | prepassStreak>=" << kPrepassStreak
              << " | prepassEvalMs=" << kPrepassEvalMs
              << " | enterTrigger=" << (kEnterTriggerMode ? "on" : "off")
              << " | enterCaptureMs=" << kEnterCaptureMs
              << " | enterFrameMs=" << kEnterFrameSubmitMs
              << " | enterFrames=" << kEnterCaptureFrames
              << " | enterMaxWaitMs=" << kEnterMaxWaitMs
              << " | enterFirstResultMaxMs=" << kEnterFirstResultMaxMs
              << " | enterDrainMinResults=" << kEnterDrainMinResults
              << " | enterDrainMaxMs=" << kEnterDrainMaxMs
              << " | turboDurationMs=" << kTurboDurationMs
              << " | turboSubmitMs=" << kTurboSubmitMs
              << " | turboHistoryMs=" << kTurboHistoryMs
              << " | turboHistoryCalls>=" << kTurboHistoryCalls
              << " | turboPrepassStreak>=" << kTurboPrepassStreak
              << " | turboTriggerConf=" << kTurboTriggerConf
              << " | prepassWidth>=" << kPrepassMinWidthRatio
              << " | minConf=" << kMinAcceptConf
              << " | oneShotConf=" << kOneShotConf
              << " | consensusHits>=" << kMinConsensusHits
              << " | stableHits>=" << kMinStableHits
              << " | maxTextFrames=" << kMaxSeenTextFrames
              << " | maxRunMs=" << kMaxRunMs << "\n";
    if (kEnterTriggerMode) {
        std::cerr << "Manual trigger enabled: press Enter in webcam window to start turbo capture.\n";
    }

    auto chooseBestFromAgg = [&]() -> std::string {
        std::string best;
        TextAggregate bestAgg{};
        const TextAggregate* bestAggPtr = nullptr;
        bool found = false;
        for (const auto& kv : textAgg) {
            const auto& a = kv.second;
            if (a.bestLen < kMinOutputChars || a.bestConf < kMinAcceptConf) continue;
            if (!found ||
                a.hits > bestAgg.hits ||
                (a.hits == bestAgg.hits && a.bestConf > bestAgg.bestConf) ||
                (a.hits == bestAgg.hits && std::abs(a.bestConf - bestAgg.bestConf) < 1e-6f && a.bestLen > bestAgg.bestLen) ||
                (a.hits == bestAgg.hits && std::abs(a.bestConf - bestAgg.bestConf) < 1e-6f && a.bestLen == bestAgg.bestLen && a.scoreSum > bestAgg.scoreSum)) {
                found = true;
                best = a.bestText.empty() ? kv.first : a.bestText;
                bestAgg = a;
                bestAggPtr = &kv.second;
            }
        }
        if (bestAggPtr && bestAggPtr->variants.size() >= 2) {
            best = PositionVote(bestAggPtr->variants);
        }
        return best;
    };

    // Aggregate a text result into the nearest edit-distance cluster in textAgg.
    auto aggregateIntoCluster = [&](const std::string& text, float conf, int lenScore, int score) {
        std::string clusterKey;
        int closestDist = kMaxEditDist + 1;
        for (const auto& [key, _] : textAgg) {
            int d = LevenshteinBounded(text, key, kMaxEditDist);
            if (d < closestDist) { closestDist = d; clusterKey = key; }
            if (d == 0) break;
        }
        TextAggregate* agg;
        if (closestDist <= kMaxEditDist) {
            agg = &textAgg[clusterKey];
        } else {
            agg = &textAgg[text];
        }
        agg->hits++;
        agg->scoreSum += (double)score;
        if (conf > agg->bestConf) { agg->bestConf = conf; agg->bestText = text; }
        if (lenScore > agg->bestLen) agg->bestLen = lenScore;
        agg->variants.push_back({text, conf});
    };

    auto recordCandidate = [&](const AsyncOcr::Candidate& ocrOut) -> bool {
        const std::string text = NormalizeText(ocrOut.text);
        if (text.empty()) return false;
        std::cerr << "[ocr-worker] text=*** conf=" << ocrOut.conf << "\n";
        enterSawAnyResult = true;
        const int lenScore = TextScore(text);

        if (lenScore >= kMinOutputChars) {
            const int rawScore = (int)(lenScore * 18 + ocrOut.conf * 80.0f);
            if (lenScore > bestRawLen || (lenScore == bestRawLen && ocrOut.conf > bestRawConf) || rawScore > bestRawScore) {
                bestRawText = text;
                bestRawLen = lenScore;
                bestRawConf = ocrOut.conf;
                bestRawScore = rawScore;
            }
        }

        if (lenScore < kMinOutputChars) return false;
        if (ocrOut.conf < kMinAcceptConf) return false;

        const int score = (int)(lenScore * 18 + ocrOut.conf * 80.0f);
        if (lenScore > bestLen || (lenScore == bestLen && ocrOut.conf > bestConf) || score > bestScore) {
            bestText = text;
            bestLen = lenScore;
            bestConf = ocrOut.conf;
            bestScore = score;
        }
        aggregateIntoCluster(text, ocrOut.conf, lenScore, score);
        return true;
    };

    auto handleUiKey = [&](int key, const std::chrono::steady_clock::time_point& now) {
        if (key == 27) return true; // ESC
        if (kEnterTriggerMode && (key == 13 || key == 10) && !manualTriggerActive) {
            manualTriggerActive = true;
            enterBurstCapturing = true;
            enterBurstStart = now;
            enterBurstLastSubmit = now - kEnterFrameInterval;
            enterSubmittedFrames = 0;
            enterAcceptedResults = 0;
            enterSawAnyResult = false;
            enterDrainStart = std::chrono::steady_clock::time_point::min();
            ocr.setPreserveOrder(true);

            // Start a fresh recognition attempt from this explicit user action.
            bestText.clear();
            bestScore = 0;
            bestConf = 0.0f;
            bestLen = 0;
            bestRawText.clear();
            bestRawScore = 0;
            bestRawConf = 0.0f;
            bestRawLen = 0;
            textAgg.clear();
            prevText.clear();
            stableHits = 0;
            seenTextFrames = 0;
            turboHistory.clear();
            turboBatchActive = false;
            turboBatchStart = std::chrono::steady_clock::time_point::min();
            turboBatchCalls = 0;

            // Close the webcam window immediately — frames are captured silently.
            cv::destroyWindow("webcam");

            std::cerr << "\nManual trigger: capturing up to " << kEnterCaptureFrames
                      << " frame(s), " << kEnterFrameSubmitMs << "ms cadence, soft window "
                      << kEnterCaptureMs << "ms, max wait " << kEnterMaxWaitMs << "ms.\n";
        }
        return false;
    };

    while (true) {
        const auto now = std::chrono::steady_clock::now();
        const bool enterDrainPhase = kEnterTriggerMode && manualTriggerActive && !enterBurstCapturing;

        if (enterDrainPhase) {
            AsyncOcr::Candidate drained;
            while (ocr.pollResult(drained)) {
                if (recordCandidate(drained)) {
                    enterAcceptedResults++;
                }
            }

            const bool pending = ocr.hasPendingWork();
            const bool enoughResults =
                enterAcceptedResults >= kEnterDrainMinResults && (!textAgg.empty() || !bestText.empty());
            const bool firstResultTimedOut =
                enterDrainStart != std::chrono::steady_clock::time_point::min() &&
                !enterSawAnyResult &&
                (now - enterDrainStart) >= kEnterFirstResultMaxWindow;
            const bool drainTimedOut =
                enterDrainStart != std::chrono::steady_clock::time_point::min() &&
                enterSawAnyResult &&
                (now - enterDrainStart) >= kEnterDrainMaxWindow;

            if (!pending || enoughResults || drainTimedOut || firstResultTimedOut) {
                finalText = chooseBestFromAgg();
                if (finalText.empty()) finalText = bestText;
                if (finalText.empty()) finalText = bestRawText;
                forceTerminateOcr = pending;
                if (firstResultTimedOut && finalText.empty()) {
                    std::cerr << "OCR wait timed out before first result.\n";
                }
                emitFinal(finalText);
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        cap >> frame;
        if (frame.empty()) break;

        if (kEnterTriggerMode) {
            if (!manualTriggerActive) {
                cv::imshow("webcam", frame);
                int key = cv::waitKey(1);
                if (handleUiKey(key, now)) break;
                continue;
            }

            const bool enterFramesDone = enterSubmittedFrames >= kEnterCaptureFrames;
            const bool enterTimedOut = (now - enterBurstStart) >= kEnterMaxWaitWindow;
            if (enterFramesDone || enterTimedOut) {
                enterBurstCapturing = false;
                enterDrainStart = now;
                ResetCameraToAuto(cap);
                cap.release();
                std::cerr << "\nCapture burst complete: " << enterSubmittedFrames
                          << " frame(s) submitted";
                if (enterTimedOut && !enterFramesDone) {
                    std::cerr << " (timed out after " << kEnterMaxWaitMs << "ms)";
                }
                std::cerr << ". Waiting OCR...\n";
                continue;
            }

            if ((now - enterBurstLastSubmit) >= kEnterFrameInterval) {
                cv::Mat ocrFrame = frame;
                const int frameMaxSide = std::max(frame.cols, frame.rows);
                if (frameMaxSide > kOcrMaxSide) {
                    const double scale = (double)kOcrMaxSide / (double)frameMaxSide;
                    cv::resize(frame, ocrFrame, cv::Size(), scale, scale, cv::INTER_AREA);
                }
                std::vector<uchar> pngBuf;
                cv::imencode(".png", ocrFrame, pngBuf);
                if (ocr.submit(std::move(pngBuf))) {
                    enterBurstLastSubmit = now;
                    enterSubmittedFrames++;
                }
            }

            AsyncOcr::Candidate streamed;
            while (ocr.pollResult(streamed)) {
                (void)recordCandidate(streamed);
            }
            continue;
        }

        // Check sharpness via Laplacian variance — skip blurry frames
        cv::Mat gray, lap;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::Laplacian(gray, lap, CV_64F);
        cv::Scalar mu, sigma;
        cv::meanStdDev(lap, mu, sigma);
        double sharpness = sigma.val[0] * sigma.val[0];

        std::cerr << "sharpness=" << (int)sharpness << "    \r";

        if (sharpness < kMinSharpness) {
            cv::imshow("webcam", frame);
            int key = cv::waitKey(1);
            if (handleUiKey(key, now)) break;
            continue;
        }

        // Reject frames with too much specular glare.
        if (kMaxGlareRatio < 1.0) {
            cv::Mat saturatedMask;
            cv::threshold(gray, saturatedMask, 250, 255, cv::THRESH_BINARY);
            int saturated = cv::countNonZero(saturatedMask);
            double glareRatio = (double)saturated / (double)(gray.rows * gray.cols);
            if (glareRatio > kMaxGlareRatio) {
                cv::imshow("webcam", frame);
                int key = cv::waitKey(1);
                if (handleUiKey(key, now)) break;
                continue;
            }
        }

        const bool turboActive = now < turboUntil;
        if (turboActive && !turboBatchActive) {
            turboBatchActive = true;
            turboBatchStart = now;
            turboBatchCalls = 0;
            turboHistory.clear();
        } else if (!turboActive && turboBatchActive) {
            turboBatchActive = false;
            turboBatchStart = std::chrono::steady_clock::time_point::min();
            turboBatchCalls = 0;
            turboHistory.clear();
        }

        bool textLike = true;
        if (manualTriggerActive && kRequireTextPrepass) {
            if ((now - lastPrepassEval) >= kPrepassEvalInterval) {
                int prepassHits = 0;
                double prepassCoverage = 0.0;
                cachedTextLike = HasTextLikePrepass(
                    frame, kPrepassMinHits, kPrepassMinCoverage, kPrepassMinWidthRatio, prepassHits, prepassCoverage
                );
                cachedPrepassHits = prepassHits;
                cachedPrepassCoverage = prepassCoverage;
                if (cachedTextLike) {
                    prepassHitStreak++;
                } else {
                    prepassHitStreak = 0;
                }
                lastPrepassEval = now;
            }
            textLike = cachedTextLike;
            if (!textLike) {
                std::cerr << "sharpness=" << (int)sharpness
                          << " prepassHits=" << cachedPrepassHits
                          << " prepassCov=" << cachedPrepassCoverage << "    \r";
            }
        }

        const auto activeInterval = turboActive ? kTurboSubmitInterval : (textLike ? kSubmitInterval : kIdleSubmitInterval);
        if (manualTriggerActive && sharpness >= kMinSharpness && (now - lastSubmit) >= activeInterval) {
            const int requiredPrepassStreak = turboActive ? kTurboPrepassStreak : kPrepassStreak;
            const bool prepassReady = !kRequireTextPrepass || prepassHitStreak >= requiredPrepassStreak;
            if (prepassReady) {
                const bool canSubmitWhileBusy = turboActive && kTurboAllowBusySubmit;
                if (canSubmitWhileBusy || !ocr.isBusy()) {
                    cv::Mat ocrFrame = frame;
                    const int frameMaxSide = std::max(frame.cols, frame.rows);
                    if (frameMaxSide > kOcrMaxSide) {
                        const double scale = (double)kOcrMaxSide / (double)frameMaxSide;
                        cv::resize(frame, ocrFrame, cv::Size(), scale, scale, cv::INTER_AREA);
                    }
                    std::vector<uchar> pngBuf;
                    cv::imencode(".png", ocrFrame, pngBuf);
                    if (ocr.submit(std::move(pngBuf))) {
                        lastSubmit = now;
                    }
                } else {
                    // Worker is still processing; back off instead of building queue pressure.
                    lastSubmit = now;
                }
            }
        }

        AsyncOcr::Candidate ocrOut;
        if (ocr.pollResult(ocrOut)) {
            const std::string text = NormalizeText(ocrOut.text);
            if (text.empty()) {
                cv::imshow("webcam", frame);
                int key = cv::waitKey(1);
                if (handleUiKey(key, now)) break;
                continue;
            }

            const int lenScore = TextScore(text);
            if (turboActive && turboBatchActive) {
                turboBatchCalls++;
                TurboHit h;
                h.text = text;
                h.conf = ocrOut.conf;
                h.len = lenScore;
                h.t = now;
                turboHistory.push_back(std::move(h));
                while ((int)turboHistory.size() > kTurboHistoryMaxHits) {
                    turboHistory.pop_front();
                }
                while (!turboHistory.empty() && (now - turboHistory.front().t) > kTurboHistoryWindow) {
                    turboHistory.pop_front();
                }
            }

            if (lenScore < kMinOutputChars) {
                cv::imshow("webcam", frame);
                int key = cv::waitKey(1);
                if (handleUiKey(key, now)) break;
                continue;
            }
            if (ocrOut.conf < kMinAcceptConf) {
                std::cerr << "ocr reject: low conf=" << ocrOut.conf << "    \r";
                cv::imshow("webcam", frame);
                int key = cv::waitKey(1);
                if (handleUiKey(key, now)) break;
                continue;
            }

            seenTextFrames++;
            const int score = (int)(lenScore * 18 + ocrOut.conf * 80.0f);
            if (lenScore > bestLen || (lenScore == bestLen && ocrOut.conf > bestConf) || score > bestScore) {
                bestText = text;
                bestLen = lenScore;
                bestConf = ocrOut.conf;
                bestScore = score;
            }
            aggregateIntoCluster(text, ocrOut.conf, lenScore, score);

            if (lenScore >= kTurboTriggerLen && ocrOut.conf >= kTurboTriggerConf && kTurboDurationMs > 0) {
                turboUntil = now + kTurboDuration;
            }

            if (text == prevText) {
                stableHits++;
            } else {
                prevText = text;
                stableHits = 1;
            }

            std::string consensusText;
            TextAggregate consensusAgg;
            bool hasConsensus = false;
            for (const auto& kv : textAgg) {
                const auto& a = kv.second;
                const std::string& displayText = a.bestText.empty() ? kv.first : a.bestText;
                if (!hasConsensus ||
                    a.hits > consensusAgg.hits ||
                    (a.hits == consensusAgg.hits && a.scoreSum > consensusAgg.scoreSum) ||
                    (a.hits == consensusAgg.hits && std::abs(a.scoreSum - consensusAgg.scoreSum) < 1e-6 && a.bestConf > consensusAgg.bestConf) ||
                    (a.hits == consensusAgg.hits && std::abs(a.scoreSum - consensusAgg.scoreSum) < 1e-6 && std::abs(a.bestConf - consensusAgg.bestConf) < 1e-6f && displayText.size() > consensusText.size())) {
                    hasConsensus = true;
                    consensusText = displayText;
                    consensusAgg = a;
                }
            }

            std::cerr << "ocr conf=" << ocrOut.conf
                      << " stable=" << stableHits
                      << " turbo=" << (turboActive ? 1 : 0)
                      << " turboCalls=" << turboBatchCalls
                      << " consensusHits=" << (hasConsensus ? consensusAgg.hits : 0) << "    \r";

            const int minAcceptLen = std::max(kMinOutputChars, 5);
            const bool oneShotGood =
                (lenScore >= std::max(kOneShotMinLen, kMinOutputChars) && ocrOut.conf >= kOneShotConf);
            const bool stableEnough =
                (stableHits >= kMinStableHits && lenScore >= std::max(minAcceptLen, bestLen - 1) && ocrOut.conf >= kStableConf);
            const bool consensusEnough =
                (hasConsensus && consensusAgg.hits >= kMinConsensusHits &&
                 consensusAgg.bestLen >= kMinOutputChars && consensusAgg.bestConf >= kMinAcceptConf);
            const bool attemptsExceeded = (seenTextFrames >= kMaxSeenTextFrames && !bestText.empty());
            const bool timeExceeded =
                ((now - loopStart) >= std::chrono::milliseconds(kMaxRunMs) && !bestText.empty());

            bool turboHistoryReady = false;
            if (turboActive && turboBatchActive && turboBatchStart != std::chrono::steady_clock::time_point::min()) {
                const bool windowReady = (now - turboBatchStart) >= kTurboHistoryWindow;
                const bool callsReady = turboBatchCalls >= kTurboHistoryCalls;
                turboHistoryReady = windowReady && callsReady && !turboHistory.empty();
            }
            if (turboHistoryReady) {
                TextAggregate histBest{};
                std::string histText;
                bool foundHist = false;
                const TextAggregate* histBestPtr = nullptr;
                std::unordered_map<std::string, TextAggregate> histAgg;
                for (const auto& h : turboHistory) {
                    // Edit-distance-aware clustering for turbo history
                    std::string clusterKey;
                    int closestDist = kMaxEditDist + 1;
                    for (const auto& [key, _] : histAgg) {
                        int d = LevenshteinBounded(h.text, key, kMaxEditDist);
                        if (d < closestDist) { closestDist = d; clusterKey = key; }
                        if (d == 0) break;
                    }
                    TextAggregate* a;
                    if (closestDist <= kMaxEditDist) {
                        a = &histAgg[clusterKey];
                    } else {
                        a = &histAgg[h.text];
                    }
                    a->hits++;
                    a->scoreSum += (double)h.conf;
                    if (h.conf > a->bestConf) { a->bestConf = h.conf; a->bestText = h.text; }
                    if (h.len > a->bestLen) a->bestLen = h.len;
                    a->variants.push_back({h.text, h.conf});
                }
                for (const auto& kv : histAgg) {
                    const auto& a = kv.second;
                    if (a.bestLen < kMinOutputChars) continue;
                    if (!foundHist ||
                        a.hits > histBest.hits ||
                        (a.hits == histBest.hits && a.bestConf > histBest.bestConf) ||
                        (a.hits == histBest.hits && std::abs(a.bestConf - histBest.bestConf) < 1e-6f && a.bestLen > histBest.bestLen) ||
                        (a.hits == histBest.hits && std::abs(a.bestConf - histBest.bestConf) < 1e-6f && a.bestLen == histBest.bestLen && a.scoreSum > histBest.scoreSum)) {
                        foundHist = true;
                        histText = a.bestText.empty() ? kv.first : a.bestText;
                        histBest = a;
                        histBestPtr = &kv.second;
                    }
                }
                if (foundHist) {
                    if (histBestPtr && histBestPtr->variants.size() >= 2) {
                        histText = PositionVote(histBestPtr->variants);
                    }
                    emitFinal(histText);
                    break;
                }
            }

            if (oneShotGood || stableEnough) {
                emitFinal(text);
                break;
            }
            if (consensusEnough) {
                emitFinal(consensusText);
                break;
            }
            if (attemptsExceeded || timeExceeded) {
                if (hasConsensus && consensusAgg.hits >= 1) {
                    emitFinal(consensusText);
                } else {
                    emitFinal(bestText);
                }
                break;
            }
        }

        cv::imshow("webcam", frame);
        int key = cv::waitKey(1);
        if (handleUiKey(key, now)) break;
    }

    ocr.shutdown(forceTerminateOcr || emittedFinalText);

    AsyncOcr::Candidate trailingOcrOut;
    if (ocr.pollResult(trailingOcrOut)) {
        const std::string text = NormalizeText(trailingOcrOut.text);
        if (!text.empty()) {
            const int lenScore = TextScore(text);
            if (lenScore >= kMinOutputChars && trailingOcrOut.conf >= kMinAcceptConf) {
                const int score = (int)(lenScore * 18 + trailingOcrOut.conf * 80.0f);
                if (lenScore > bestLen || (lenScore == bestLen && trailingOcrOut.conf > bestConf) || score > bestScore) {
                    bestText = text;
                    bestLen = lenScore;
                    bestConf = trailingOcrOut.conf;
                    bestScore = score;
                }
            }
        }
    }

    if (!emittedFinalText) {
        std::string consensusText;
        TextAggregate consensusAgg;
        const TextAggregate* consensusAggPtr = nullptr;
        bool hasConsensus = false;
        for (const auto& kv : textAgg) {
            const auto& a = kv.second;
            if (a.hits < kMinConsensusHits) continue;
            const std::string& displayText = a.bestText.empty() ? kv.first : a.bestText;
            if (!hasConsensus ||
                a.hits > consensusAgg.hits ||
                (a.hits == consensusAgg.hits && a.scoreSum > consensusAgg.scoreSum) ||
                (a.hits == consensusAgg.hits && std::abs(a.scoreSum - consensusAgg.scoreSum) < 1e-6 && a.bestConf > consensusAgg.bestConf) ||
                (a.hits == consensusAgg.hits && std::abs(a.scoreSum - consensusAgg.scoreSum) < 1e-6 && std::abs(a.bestConf - consensusAgg.bestConf) < 1e-6f && displayText.size() > consensusText.size())) {
                hasConsensus = true;
                consensusText = displayText;
                consensusAgg = a;
                consensusAggPtr = &kv.second;
            }
        }
        if (hasConsensus && consensusAggPtr && consensusAggPtr->variants.size() >= 2) {
            consensusText = PositionVote(consensusAggPtr->variants);
        }
        if (hasConsensus) {
            emitFinal(consensusText);
        } else {
            emitFinal(bestText);
        }
    }

    // Clean up any remaining OpenCV windows and release the camera.
    cv::destroyAllWindows();
    if (cap.isOpened()) {
        ResetCameraToAuto(cap);
        cap.release();
    }

    if (!noExitPrompt) {
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
    }
    return 0;
}

#ifndef TESS_OCR_NO_MAIN
int main(int argc, char** argv) {
    return tess_webcam_ocr_run(argc, argv);
}
#endif
