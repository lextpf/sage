/*  ============================================================================================  *
 *                                                            ⠀⣠⡤⠀⢀⣀⣀⡀⠀⠀⠀⠀⣦⡀⠀⠀⠀⠀⠀⠀
 *                                                            ⠀⠘⠃⠈⢿⡏⠉⠉⠀⢀⣀⣰⣿⣿⡄⠀⠀⠀⠀⢀
 *           ::::::::      :::      ::::::::  ::::::::::      ⠀⠀⠀⠀⠀⢹⠀⠀⠀⣸⣿⡿⠉⠿⣿⡆⠀⠰⠿⣿
 *          :+:    :+:   :+: :+:   :+:    :+: :+:             ⠀⠀⠀⠀⠀⢀⣠⠾⠿⠿⠿⠀⢰⣄⠘⢿⠀⠀⠀⠞
 *          +:+         +:+   +:+  +:+        +:+             ⢲⣶⣶⡂⠐⢉⣀⣤⣶⣶⡦⠀⠈⣿⣦⠈⠀⣾⡆⠀
 *          +#++:++#++ +#++:++#++: :#:        +#++:++#        ⠀⠀⠿⣿⡇⠀⠀⠀⠙⢿⣧⠀⠳⣿⣿⡀⠸⣿⣿⠀
 *                 +#+ +#+     +#+ +#+   +#+# +#+             ⠀⠀⠐⡟⠁⠀⠀⢀⣴⣿⠛⠓⠀⣉⣿⣿⢠⡈⢻⡇
 *          #+#    #+# #+#     #+# #+#    #+# #+#             ⠀⠀⠀⠀⠀⠀⠀⣾⣿⣿⣆⠀⢹⣿⣿⣷⡀⠁⢸⡇
 *           ########  ###     ###  ########  ##########      ⠀⠀⠀⠀⠀⠀⠘⠛⠛⠉⠀⠀⠈⠙⠛⠿⢿⣶⣼⠃
 *                                                            ⠀⠀⠀⢰⣧⣤⠤⠖⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
 *
 *                                  << P A S S   M A N A G E R >>
 *
 *  ============================================================================================  *
 *
 *      A Windows AES-256-GCM encryption utility with Qt6/QML GUI and CLI
 *      providing on-demand credential management, directory encryption,
 *      webcam OCR authentication, and global auto-fill.
 *
 *    ----------------------------------------------------------------------
 *
 *      Repository:   https://github.com/lextpf/sage
 *      License:      MIT
 */
#include "Cryptography.h"
#include "Utils.h"
#include "Clipboard.h"
#include "Console.h"
#include "FileOperations.h"

#ifdef USE_QT_UI
#include <QtCore/QString>
#include "Vault.h"
#endif

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

// Forward declaration - implementation is in qmlmain.cpp
#ifdef USE_QT_UI
int RunQMLMode(int argc, char* argv[]);
#endif

/**
 * @brief Program entry point for sage.
 *
 * @details
 * Main execution flow:
 *
 * **Command-line Arguments:**
 * - `-e` or `--encrypt`: Stream encryption mode (stdin -> stdout)
 * - `-d` or `--decrypt`: Stream decryption mode (stdin -> stdout)
 * - `-u` or `--ui`: Launch graphical user interface (default if no mode specified)
 * - `--cli`: Launch command-line interactive mode
 * - `-h` or `--help`: Display usage information
 * - No arguments: GUI mode (default)
 *
 * **Security Initialization:**
 *    - Enable Windows process mitigations (dynamic code prohibition, etc.)
 *    - Abort if running under Remote Desktop (security risk)
 *    - Initialize OpenSSL error strings
 *    - Harden heap (termination on corruption)
 *    - Attempt to enable SeLockMemoryPrivilege (warns if unavailable)
 *
 * **Password Acquisition:**
 *    - Prompt for master password using Windows Credentials UI
 *    - Password stored in locked memory with guard pages
 *
 * **Streaming Mode (if -e or -d specified):**
 *    - Read from stdin, process, write to stdout
 *    - Binary mode for both encryption and decryption
 *    - Errors written to stderr
 *    - Exit after single operation
 *
 * **Interactive Mode (default):**
 *    - Optional file processing from "sage" file
 *    - Display welcome banner
 *    - Enter loop reading user input
 *    - Process batches of files/text until '?' (masked) or '!' (uncensored)
 *    - Support commands: '.', ':clip', ':open', ':none'
 *    - Press Esc to exit
 *
 * **Cleanup:**
 *    - Securely scrub password from memory
 *    - Wipe console buffer (interactive mode only)
 *    - Release all resources
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return
 * - `0` on normal exit
 * - `-1` on early security abort (mitigations failed or Remote Desktop detected)
 * - `1` on usage error or operation failure
 *
 * @note The application will continue even if SeLockMemoryPrivilege cannot be enabled,
 * but will display a security warning. Optimal security requires this privilege.
 *
 * @warning Do not run under Remote Desktop sessions. The application will detect
 * and abort to prevent security risks.
 *
 * @example
 * Encrypt a file:
 * ```
 * sage -e < input.txt > output.sage
 * ```
 *
 * Decrypt a file:
 * ```
 * sage -d < output.sage > decrypted.txt
 * ```
 *
 * Pipe data:
 * ```
 * echo "Hello World" | sage -e | sage -d
 * ```
 *
 * @see FileOperations::processBatch
 * @see FileOperations::streamEncrypt
 * @see FileOperations::streamDecrypt
 * @see readPasswordSecureDesktop
 * @see Cryptography::setSecureProcessMitigations
 * @see Cryptography::tryEnableLockPrivilege
 */
int main(int argc, char* argv[]) {
    // Parse command-line arguments
    bool streamMode = false;
    bool encryptMode = false;
    bool decryptMode = false;
    bool uiMode = false;
    bool cliMode = false;
    bool importMode = false;
    std::string importData;
    std::string importOutputPath;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-e" || arg == "--encrypt") {
            streamMode = true;
            encryptMode = true;
        }
        else if (arg == "-d" || arg == "--decrypt") {
            streamMode = true;
            decryptMode = true;
        }
        else if (arg == "-u" || arg == "--ui") {
            uiMode = true;
        }
        else if (arg == "--cli") {
            cliMode = true;
        }
        else if (arg == "--import") {
            importMode = true;
            if (i + 2 < argc) {
                importData = argv[++i];
                importOutputPath = argv[++i];
            } else {
                std::cerr << "Error: --import requires two arguments\n";
                std::cerr << "Usage: sage --import \"plat:user:pass,...\" output.sage\n";
                return 1;
            }
        }
        else if (arg == "-h" || arg == "--help") {
            std::cout << "sage - AES-256-GCM Encryption Utility\n\n";
            std::cout << "Usage:\n";
            std::cout << "  sage [OPTIONS]\n\n";
            std::cout << "Options:\n";
            std::cout << "  -e, --encrypt    Stream encryption mode (stdin -> stdout)\n";
            std::cout << "  -d, --decrypt    Stream decryption mode (stdin -> stdout)\n";
            std::cout << "  -u, --ui         Launch graphical user interface\n";
            std::cout << "  --cli            Launch command-line interactive mode\n";
            std::cout << "  --import DATA OUTPUT  Import credentials into a vault file\n";
            std::cout << "  -h, --help       Display this help message\n";
            std::cout << "  (no args)        GUI mode (default)\n\n";
            std::cout << "Import format:\n";
            std::cout << "  DATA is comma-separated entries: plat:user:pass, plat:user:pass, ...\n";
            std::cout << "  DATA can also be a path to a text file containing entries\n";
            std::cout << "  (one per line or comma-separated, spaces around commas are OK)\n";
            std::cout << "  OUTPUT is the vault file path (e.g. myvault.sage)\n\n";
            std::cout << "Examples:\n";
            std::cout << "  sage -e < input.txt > output.sage\n";
            std::cout << "  sage -d < output.sage > decrypted.txt\n";
            std::cout << "  echo \"Hello\" | sage -e | sage -d\n";
            std::cout << "  sage        (Launch GUI mode - default)\n";
            std::cout << "  sage --ui   (Launch GUI mode)\n";
            std::cout << "  sage --cli  (Launch CLI interactive mode)\n";
            std::cout << "  sage --import \"github:alice:pw123, aws:bob:secret\" myvault.sage\n";
            std::cout << "  sage --import entries.txt myvault.sage\n";
            return 0;
        }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Use -h or --help for usage information.\n";
            return 1;
        }
    }

    // Check for conflicting mode flags
    if (uiMode && cliMode) {
        std::cerr << "Error: Cannot specify both --ui and --cli\n";
        return 1;
    }

    // Determine mode early so security mitigations match runtime requirements.
    const bool useUIMode = uiMode || (!cliMode && !streamMode);
    const bool allowDynamicCode = useUIMode && !importMode;

    sage::Cryptography::detectDebugger();

    if (!sage::Cryptography::setSecureProcessMitigations(allowDynamicCode)) return -1;
    if (sage::Cryptography::isRemoteSession()) return -1;

    sage::Cryptography::hardenHeap();
    sage::Cryptography::hardenProcessAccess();
    sage::Cryptography::disableCrashDumps();
    if (!sage::Cryptography::tryEnableLockPrivilege()) {
        char* username = nullptr;
        size_t len = 0;
        _dupenv_s(&username, &len, "USERNAME");

        std::cerr << "\n!!! SECURITY WARNING !!!\n\n"
            << "Failed to enable memory lock privilege (SE_LOCK_MEMORY_NAME).\n"
            << "This application cannot securely protect sensitive data in memory.\n\n"
            << "To fix this issue:\n"
            << "  1. Open Group Policy Editor (gpedit.msc)\n"
            << "  2. Go to \"Local Policies\" then \"User Rights Assignment\"\n"
            << "  3. Add your account to \"Lock pages in memory\"\n"
            << "  4. Reboot your system\n\n"
            << "Current user: " << (username ? username : "Unknown") << "\n";

        free(username);  // Must free the memory allocated by _dupenv_s
    }

    // --import handler: parse credentials and write vault file
    if (importMode) {
#ifdef USE_QT_UI
        // Helper: convert UTF-8 std::string to secure wide string
        auto utf8ToSecureWide = [](const std::string& utf8)
            -> sage::basic_secure_string<wchar_t, sage::locked_allocator<wchar_t>> {
            sage::basic_secure_string<wchar_t, sage::locked_allocator<wchar_t>> result;
            if (utf8.empty()) return result;
            int need = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
            if (need > 0) {
                result.s.resize(need);
                MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), result.s.data(), need);
            }
            return result;
        };

        // If importData is a file path, read entries from the file
        {
            std::ifstream testFile(importData);
            if (testFile.good()) {
                std::string fileContent((std::istreambuf_iterator<char>(testFile)),
                                         std::istreambuf_iterator<char>());
                testFile.close();
                // Replace newlines with commas so entries can be one-per-line or comma-separated
                for (char& c : fileContent) {
                    if (c == '\n' || c == '\r') c = ',';
                }
                importData = fileContent;
                std::cout << "Reading entries from file...\n";
            }
        }

        // Helper: trim whitespace from both ends
        auto trim = [](const std::string& s) -> std::string {
            size_t start = s.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return "";
            size_t end = s.find_last_not_of(" \t\r\n");
            return s.substr(start, end - start + 1);
        };

        // Parse comma-separated plat:user:pass entries
        std::vector<std::tuple<std::string, std::string, std::string>> entries;
        {
            std::string remaining = importData;
            while (!remaining.empty()) {
                size_t commaPos = remaining.find(',');
                std::string token = (commaPos != std::string::npos)
                    ? remaining.substr(0, commaPos)
                    : remaining;
                remaining = (commaPos != std::string::npos)
                    ? remaining.substr(commaPos + 1)
                    : "";

                token = trim(token);
                if (token.empty()) continue;

                // Split token by ':' - first two colons separate plat:user:pass
                // (password may contain colons)
                size_t firstColon = token.find(':');
                if (firstColon == std::string::npos) {
                    std::cerr << "Error: Invalid entry (missing colon): " << token << "\n";
                    std::cerr << "Expected format: platform:username:password\n";
                    return 1;
                }
                size_t secondColon = token.find(':', firstColon + 1);
                if (secondColon == std::string::npos) {
                    std::cerr << "Error: Invalid entry (missing second colon): " << token << "\n";
                    std::cerr << "Expected format: platform:username:password\n";
                    return 1;
                }

                std::string platform = trim(token.substr(0, firstColon));
                std::string user     = trim(token.substr(firstColon + 1, secondColon - firstColon - 1));
                std::string pass     = token.substr(secondColon + 1);

                if (platform.empty() || user.empty() || pass.empty()) {
                    std::cerr << "Error: Empty field in entry: " << token << "\n";
                    return 1;
                }
                entries.emplace_back(platform, user, pass);
            }
        }

        if (entries.empty()) {
            std::cerr << "Error: No valid entries found in import data\n";
            return 1;
        }

        std::cout << "Importing " << entries.size() << " credential(s)...\n";

        // Prompt for master password
        sage::basic_secure_string<wchar_t> masterPassword;
        try {
            masterPassword = sage::readPasswordSecureDesktop();
        } catch (...) {
            std::cerr << "Error: Failed to read master password\n";
            return 1;
        }
        sage::DPAPIGuard<sage::basic_secure_string<wchar_t>> importDpapi(&masterPassword);

        // Encrypt each entry - unprotect for the duration of encryption
        importDpapi.unprotect();
        std::vector<sage::VaultRecord> records;
        records.reserve(entries.size());
        for (const auto& [platform, user, pass] : entries) {
            auto secUser = utf8ToSecureWide(user);
            auto secPass = utf8ToSecureWide(pass);

            records.push_back(sage::encryptCredential(
                platform, secUser, secPass, masterPassword));

            sage::Cryptography::cleanseString(secUser, secPass);
        }

        // Save vault
        QString outputPath = QString::fromUtf8(importOutputPath.c_str());
        if (!outputPath.endsWith(".sage", Qt::CaseInsensitive)) {
            outputPath += ".sage";
        }

        if (sage::saveVaultV2(outputPath, records, masterPassword)) {
            std::cout << "Successfully saved " << records.size()
                      << " credential(s) to " << outputPath.toStdString() << "\n";
            sage::Cryptography::cleanseString(masterPassword);
            return 0;
        } else {
            std::cerr << "Error: Failed to save vault file\n";
            sage::Cryptography::cleanseString(masterPassword);
            return 1;
        }
#else
        std::cerr << "Error: --import requires Qt UI support (USE_QT_UI).\n";
        std::cerr << "Please rebuild with USE_QT_UI enabled.\n";
        return 1;
#endif
    }

    // GUI mode (default or explicitly requested)
    if (useUIMode) {
#ifdef USE_QT_UI
        return RunQMLMode(argc, argv);
#else
        std::cerr << "Error: GUI mode requested but Qt UI support is not compiled in.\n";
        std::cerr << "Please rebuild with USE_QT_UI enabled or use --cli for CLI mode.\n";
        return 1;
#endif
    }

    if (encryptMode && decryptMode) {
        std::cerr << "Error: Cannot specify both -e and -d\n";
        return 1;
    }

    try {
        sage::basic_secure_string<wchar_t> password = sage::readPasswordSecureDesktop();
        sage::DPAPIGuard<sage::basic_secure_string<wchar_t>> dpapi(&password);

        // Streaming mode: process stdin -> stdout
        if (streamMode) {
            dpapi.unprotect();
            bool success = false;
            if (encryptMode) {
                success = sage::FileOperations::streamEncrypt(password);
            }
            else if (decryptMode) {
                success = sage::FileOperations::streamDecrypt(password);
            }

            sage::Cryptography::cleanseString(password);
            return success ? 0 : 1;
        }

        // 1) One-off batch from sage (optional)
        {
            std::ifstream fin("sage");
            if (fin) {
                auto fileBatch = sage::readBulkLinesDualFrom(fin);
                const auto& flines = fileBatch.first;
                bool funcensored = fileBatch.second;
                if (!flines.empty()) {
                    dpapi.unprotect();
                    sage::FileOperations::processBatch(flines, funcensored, password);
                    dpapi.reprotect();
                    std::cout << "\n";
                }
            }
        }

        // 2) Interactive console input
        std::cout << "+----------------------------------------- sage - Interactive Mode -----------------------------------------+\n";
        std::cout << "|              Paste/type and finish with '?' (MASKED) or '!' (UNCENSORED) Press Esc to exit.               |\n";
        std::cout << "|    Commands '.'= current dir | ':clip'= copy sage | ':open'= edit sage | ':none'= clear clipboard         |\n";
        std::cout << "+-----------------------------------------------------------------------------------------------------------+\n";

        for (;;) {
            std::pair<std::vector<std::string>, bool> batch;
            if (!sage::readBulkLinesDualOrEsc(batch)) {
                // 1) One-off batch from sage (optional)
                {
                    std::ifstream fin("sage");
                    if (fin) {
                        auto fileBatch = sage::readBulkLinesDualFrom(fin);
                        const auto& flines = fileBatch.first;
                        bool funcensored = fileBatch.second;
                        bool yes = false;
                        if (!flines.empty()) {
                            for (auto& line : flines) {
                                if (_stricmp(line.c_str(), ".") == 0 || sage::utils::isDirectoryA(line.c_str()) || sage::utils::fileExistsA(line.c_str()))
                                    yes = true;
                            }
                            if (yes) {
                                dpapi.unprotect();
                                sage::FileOperations::processBatch(flines, funcensored, password);
                                dpapi.reprotect();
                            }
                            return 0;
                        }
                    }
                }
                return 0; // Esc pressed outside the masked UI
            }
            const auto& lines = batch.first;
            bool uncensored = batch.second;

            if (lines.empty()) break;

            dpapi.unprotect();
            sage::FileOperations::processBatch(lines, uncensored, password);
            dpapi.reprotect();
        }
        sage::Cryptography::cleanseString(password);
        if (!streamMode) {
            sage::wipeConsoleBuffer();
        }
    }
    catch (const std::exception& e) {
        // In streaming mode, print errors; in interactive mode, exit quietly
        if (streamMode) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
        // Interactive mode: interrupted or EOF; exit quietly
    }
    return 0;
}
