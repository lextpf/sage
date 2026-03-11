/*  ============================================================================================  *
 *                                                            ⠀⣠⡤⠀⢀⣀⣀⡀⠀⠀⠀⠀⣦⡀⠀⠀⠀⠀⠀⠀
 *                                                            ⠀⠘⠃⠈⢿⡏⠉⠉⠀⢀⣀⣰⣿⣿⡄⠀⠀⠀⠀⢀
 *           ::::::::  ::::::::::     :::     :::             ⠀⠀⠀⠀⠀⢹⠀⠀⠀⣸⣿⡿⠉⠿⣿⡆⠀⠰⠿⣿
 *          :+:    :+: :+:          :+: :+:   :+:             ⠀⠀⠀⠀⠀⢀⣠⠾⠿⠿⠿⠀⢰⣄⠘⢿⠀⠀⠀⠞
 *          +:+        +:+         +:+   +:+  +:+             ⢲⣶⣶⡂⠐⢉⣀⣤⣶⣶⡦⠀⠈⣿⣦⠈⠀⣾⡆⠀
 *          +#++:++#++ +#++:++#   +#++:++#++: +#+             ⠀⠀⠿⣿⡇⠀⠀⠀⠙⢿⣧⠀⠳⣿⣿⡀⠸⣿⣿⠀
 *                 +#+ +#+        +#+     +#+ +#+             ⠀⠀⠐⡟⠁⠀⠀⢀⣴⣿⠛⠓⠀⣉⣿⣿⢠⡈⢻⡇
 *          #+#    #+# #+#        #+#     #+# #+#             ⠀⠀⠀⠀⠀⠀⠀⣾⣿⣿⣆⠀⢹⣿⣿⣷⡀⠁⢸⡇
 *           ########  ########## ###     ### ##########      ⠀⠀⠀⠀⠀⠀⠘⠛⠛⠉⠀⠀⠈⠙⠛⠿⢿⣶⣼⠃
 *                                                            ⠀⠀⠀⢰⣧⣤⠤⠖⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
 *
 *                                  << P A S S   M A N A G E R >>
 *
 *  ============================================================================================  *
 *
 *      A Windows AES-256-GCM encryption utility with Qt6/QML GUI and CLI
 *      providing on-demand credential management, directory encryption,
 *      webcam QR authentication, and global auto-fill.
 *
 *    ----------------------------------------------------------------------
 *
 *      Repository:   https://github.com/lextpf/seal
 *      License:      MIT
 */
#include "Clipboard.h"
#include "Console.h"
#include "Cryptography.h"
#include "FileOperations.h"
#include "ScopedDpapiUnprotect.h"
#include "Utils.h"

#ifdef USE_QT_UI
#include <QtCore/QString>
#include "Vault.h"
#endif

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

// Forward declaration - implementation is in qmlmain.cpp
#ifdef USE_QT_UI
int RunQMLMode(int argc, char* argv[]);
#endif

// Parsed command-line options for mode dispatch.
struct ProgramOptions
{
    bool streamMode = false;
    bool encryptMode = false;
    bool decryptMode = false;
    bool uiMode = false;
    bool cliMode = false;
    bool importMode = false;
    std::string importData;
    std::string importOutputPath;
    bool exportMode = false;
    std::string exportInputPath;
    std::string exportOutputPath;
};

// Alias for backwards compatibility with existing call sites in this file.
template <class GuardT>
using ScopedUnprotect = seal::ScopedDpapiUnprotect<GuardT>;

static void printHelp()
{
    std::cout << "seal - AES-256-GCM Encryption Utility\n\n";
    std::cout << "Usage:\n";
    std::cout << "  seal [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -e, --encrypt           Stream encryption mode (stdin -> stdout)\n";
    std::cout << "  -d, --decrypt           Stream decryption mode (stdin -> stdout)\n";
    std::cout << "  -u, --ui                Launch graphical user interface\n";
    std::cout << "  --cli                   Launch command-line interactive mode\n";
    std::cout << "  --import DATA [OUTPUT]  Import credentials into a vault file\n"
              << "  --import - [OUTPUT]     Read import entries from stdin (pipe or paste)\n";
    std::cout << "  --export INPUT [OUTPUT] Export vault to plaintext (re-importable)\n";
    std::cout << "  -h, --help              Display this help message\n";
    std::cout << "  (no args)               GUI mode (default)\n\n";
    std::cout << "Import format:\n";
    std::cout << "  DATA is comma-separated entries: plat:user:pass, plat:user:pass,...\n";
    std::cout << "  DATA can also be a path to a text file containing entries\n";
    std::cout << "  OUTPUT is the vault file path (default: .seal)\n";
    std::cout << "  Vault files are saved as a single framed hex blob.\n\n";
    std::cout << "Export format:\n";
    std::cout << "  INPUT is the vault file path (e.g. vault.seal)\n";
    std::cout << "  OUTPUT is the plaintext output path (default: stdout)\n";
    std::cout << "  Single string: plat:user:pass,plat:user:pass,... (directly re-importable)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  seal -e < input.txt > output.seal\n";
    std::cout << "  seal -d < output.seal > decrypted.txt\n";
    std::cout << "  echo \"Hello\" | seal -e | seal -d\n";
    std::cout << "  seal        (Launch GUI mode - default)\n";
    std::cout << "  seal --ui   (Launch GUI mode)\n";
    std::cout << "  seal --cli  (Launch CLI interactive mode)\n";
    std::cout << "  seal --import \"github:alice:pw123\"              (saves to .seal)\n";
    std::cout << "  seal --import entries.txt vault.seal            (saves to vault)\n";
    std::cout << "  seal --import - vault.seal < entries.txt        (read from stdin)\n";
    std::cout << "  echo \"github:alice:pw123\" | seal --import -     (pipe into stdin)\n";
    std::cout << "  seal --export vault.seal                        (print to stdout)\n";
    std::cout << "  seal --export vault.seal export.txt             (save to file)\n";
}

static bool isOptionToken(const char* token)
{
    return token && token[0] == '-' && token[1] != '\0';
}

// Returns: -1 = parsed OK (continue), 0 = help shown (exit 0), 1 = error (exit 1)
static int parseArguments(int argc, char* argv[], ProgramOptions& opts)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-e" || arg == "--encrypt")
        {
            opts.streamMode = true;
            opts.encryptMode = true;
        }
        else if (arg == "-d" || arg == "--decrypt")
        {
            opts.streamMode = true;
            opts.decryptMode = true;
        }
        else if (arg == "-u" || arg == "--ui")
        {
            opts.uiMode = true;
        }
        else if (arg == "--cli")
        {
            opts.cliMode = true;
        }
        else if (arg == "--import")
        {
            opts.importMode = true;
            if (i + 1 < argc && !isOptionToken(argv[i + 1]))
            {
                opts.importData = argv[++i];
                if (i + 1 < argc && !isOptionToken(argv[i + 1]))
                    opts.importOutputPath = argv[++i];
                else
                    opts.importOutputPath = ".seal";
            }
            else
            {
                std::cerr << "Error: --import requires at least one argument\n";
                std::cerr << "Usage: seal --import \"plat:user:pass,...\" [output.seal]\n";
                return 1;
            }
        }
        else if (arg == "--export")
        {
            opts.exportMode = true;
            if (i + 1 < argc && !isOptionToken(argv[i + 1]))
            {
                opts.exportInputPath = argv[++i];
                if (i + 1 < argc && !isOptionToken(argv[i + 1]))
                    opts.exportOutputPath = argv[++i];
            }
            else
            {
                std::cerr << "Error: --export requires a vault file argument\n";
                std::cerr << "Usage: seal --export vault.seal [output.txt]\n";
                return 1;
            }
        }
        else if (arg == "-h" || arg == "--help")
        {
            printHelp();
            return 0;
        }
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Use -h or --help for usage information.\n";
            return 1;
        }
    }

    if (opts.uiMode && opts.cliMode)
    {
        std::cerr << "Error: Cannot specify both --ui and --cli\n";
        return 1;
    }
    if (opts.encryptMode && opts.decryptMode)
    {
        std::cerr << "Error: Cannot specify both -e and -d\n";
        return 1;
    }
    if (opts.importMode && opts.streamMode)
    {
        std::cerr << "Error: Cannot combine --import with -e/-d\n";
        return 1;
    }
    if (opts.exportMode && opts.streamMode)
    {
        std::cerr << "Error: Cannot combine --export with -e/-d\n";
        return 1;
    }
    if (opts.exportMode && opts.importMode)
    {
        std::cerr << "Error: Cannot combine --export with --import\n";
        return 1;
    }
    return -1;
}

// Apply all process-wide security mitigations in dependency order.
// Returns 0 on success, 1 if a critical mitigation fails.
static int initializeSecurity(bool allowDynamicCode)
{
    // Order matters: debugger check first (fail fast before any secrets load),
    // then process mitigations (CFG, DEP, ASLR, dynamic-code policy), then
    // heap/access hardening, and finally memory-lock privilege which is needed
    // before any secure_string allocations touch VirtualLock.
    seal::Cryptography::detectDebugger();

    if (!seal::Cryptography::setSecureProcessMitigations(allowDynamicCode))
        return 1;
    if (seal::Cryptography::isRemoteSession())
        return 1;

    seal::Cryptography::hardenHeap();
    seal::Cryptography::hardenProcessAccess();
    seal::Cryptography::disableCrashDumps();
    if (!seal::Cryptography::tryEnableLockPrivilege())
    {
        const char* username = std::getenv("USERNAME");

        std::cerr << "\n!!! SECURITY WARNING !!!\n\n"
                  << "Failed to enable memory lock privilege (SE_LOCK_MEMORY_NAME).\n"
                  << "This application cannot securely protect sensitive data in memory.\n\n"
                  << "To fix this issue:\n"
                  << "  1. Open Group Policy Editor (gpedit.msc)\n"
                  << "  2. Go to \"Local Policies\" then \"User Rights Assignment\"\n"
                  << "  3. Add your account to \"Lock pages in memory\"\n"
                  << "  4. Reboot your system\n\n"
                  << "Current user: " << (username ? username : "Unknown") << "\n";
    }
    return 0;
}

#ifdef USE_QT_UI
static void loadImportDataFromFile(std::string& importData)
{
    std::string fileContent;

    if (importData == "-")
    {
        // Read from stdin - supports piping and paste (Ctrl+Z to end on Windows).
        fileContent.assign(std::istreambuf_iterator<char>(std::cin),
                           std::istreambuf_iterator<char>());
        std::cout << "Reading entries from stdin...\n";
    }
    else
    {
        std::ifstream testFile(importData);
        if (!testFile.good())
            return;

        fileContent.assign(std::istreambuf_iterator<char>(testFile),
                           std::istreambuf_iterator<char>());
        testFile.close();
        std::cout << "Reading entries from file...\n";
    }

    // Replace newlines with commas so entries can be one-per-line or comma-separated.
    std::replace_if(
        fileContent.begin(), fileContent.end(), [](char c) { return c == '\n' || c == '\r'; }, ',');
    importData = fileContent;
}

static int parseImportEntries(
    const std::string& importData,
    std::vector<std::tuple<std::string, std::string, std::string>>& entries)
{
    std::string remaining = importData;
    while (!remaining.empty())
    {
        size_t commaPos = remaining.find(',');
        std::string token =
            (commaPos != std::string::npos) ? remaining.substr(0, commaPos) : remaining;
        remaining = (commaPos != std::string::npos) ? remaining.substr(commaPos + 1) : "";

        token = seal::utils::trim(token);
        if (token.empty())
            continue;

        size_t firstColon = token.find(':');
        if (firstColon == std::string::npos)
        {
            std::cerr << "Error: Invalid entry (missing colon): " << token << "\n";
            std::cerr << "Expected format: platform:username:password\n";
            return 1;
        }
        size_t secondColon = token.find(':', firstColon + 1);
        if (secondColon == std::string::npos)
        {
            std::cerr << "Error: Invalid entry (missing second colon): " << token << "\n";
            std::cerr << "Expected format: platform:username:password\n";
            return 1;
        }

        std::string platform = seal::utils::trim(token.substr(0, firstColon));
        std::string user =
            seal::utils::trim(token.substr(firstColon + 1, secondColon - firstColon - 1));
        std::string pass = token.substr(secondColon + 1);

        if (platform.empty() || user.empty() || pass.empty())
        {
            std::cerr << "Error: Empty field in entry: " << token << "\n";
            return 1;
        }
        entries.emplace_back(platform, user, pass);
    }
    return 0;
}

static int handleImportMode(std::string& importData, const std::string& importOutputPath)
{
    loadImportDataFromFile(importData);

    std::vector<std::tuple<std::string, std::string, std::string>> entries;
    int rc = parseImportEntries(importData, entries);
    if (rc != 0)
        return rc;

    if (entries.empty())
    {
        std::cerr << "Error: No valid entries found in import data\n";
        return 1;
    }

    std::cout << "Importing " << entries.size() << " credential(s)...\n";

    seal::basic_secure_string<wchar_t> masterPassword;
    try
    {
        masterPassword = seal::readPasswordConsole();
    }
    catch (...)
    {
        std::cerr << "Error: Failed to read master password\n";
        return 1;
    }
    // DPAPIGuard wraps the master password with CryptProtectMemory while idle.
    // unprotect() decrypts it in-place for the encryption loop; reprotect()
    // (or destructor) re-encrypts it so the plaintext key is short-lived.
    seal::DPAPIGuard<seal::basic_secure_string<wchar_t>> importDpapi(&masterPassword);

    std::vector<seal::VaultRecord> records;
    records.reserve(entries.size());
    try
    {
        ScopedUnprotect dpapiScope(importDpapi);
        for (const auto& [platform, user, pass] : entries)
        {
            auto secUser = seal::utils::utf8ToSecureWide(user);
            auto secPass = seal::utils::utf8ToSecureWide(pass);
            records.push_back(seal::encryptCredential(platform, secUser, secPass, masterPassword));
            // Wipe the wide copies immediately; the encrypted VaultRecord now owns the data.
            seal::Cryptography::cleanseString(secUser, secPass);
        }

        QString outputPath = QString::fromUtf8(importOutputPath.c_str());
        if (!outputPath.endsWith(".seal", Qt::CaseInsensitive))
            outputPath += ".seal";

        // Cleanse the original plaintext entry strings now that every credential
        // has been encrypted into a VaultRecord. This limits the plaintext lifetime.
        for (auto& [p, u, pw] : entries)
            seal::Cryptography::cleanseString(p, u, pw);

        if (seal::saveVaultV2(outputPath, records, masterPassword))
        {
            std::cout << "Successfully saved " << records.size() << " credential(s) to "
                      << outputPath.toStdString() << "\n";
            seal::Cryptography::cleanseString(masterPassword);
            return 0;
        }
        std::cerr << "Error: Failed to save vault file\n";
        seal::Cryptography::cleanseString(masterPassword);
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        seal::Cryptography::cleanseString(masterPassword);
    }
    return 1;
}

static int handleExportMode(const std::string& inputPath, const std::string& outputPath)
{
    QString vaultPath = QString::fromUtf8(inputPath.c_str());
    if (!vaultPath.endsWith(".seal", Qt::CaseInsensitive))
        vaultPath += ".seal";

    seal::basic_secure_string<wchar_t> masterPassword;
    try
    {
        masterPassword = seal::readPasswordConsole();
    }
    catch (...)
    {
        std::cerr << "Error: Failed to read master password\n";
        return 1;
    }
    seal::DPAPIGuard<seal::basic_secure_string<wchar_t>> exportDpapi(&masterPassword);

    std::vector<seal::VaultRecord> records;
    try
    {
        ScopedUnprotect dpapiScope(exportDpapi);
        records = seal::loadVaultIndex(vaultPath, masterPassword);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        seal::Cryptography::cleanseString(masterPassword);
        return 1;
    }

    if (records.empty())
    {
        std::cerr << "Vault is empty, nothing to export.\n";
        seal::Cryptography::cleanseString(masterPassword);
        return 0;
    }

    // Build one comma-separated export string in the same format that --import accepts.
    // SECURITY: fail immediately on the first decryption error. If we kept going,
    // the number of successful decryptions before the error would leak how many
    // records the vault contains (side-channel on wrong password).
    // Pre-reserve to reduce intermediate reallocation copies of plaintext on
    // the heap. Each record is roughly ~80 chars (platform:user:pass + comma).
    std::string exportData;
    exportData.reserve(records.size() * 80);
    size_t exportedCount = 0;
    try
    {
        ScopedUnprotect dpapiScope(exportDpapi);
        for (const auto& rec : records)
        {
            if (rec.deleted)
                continue;
            auto cred = seal::decryptCredentialOnDemand(rec, masterPassword);
            std::string user = seal::utils::secureWideToUtf8(cred.username);
            std::string pass = seal::utils::secureWideToUtf8(cred.password);
            std::string entry = rec.platform + ":" + user + ":" + pass;
            if (!exportData.empty())
                exportData.push_back(',');
            exportData += entry;
            ++exportedCount;
            seal::Cryptography::cleanseString(entry);
            seal::Cryptography::cleanseString(user, pass);
            cred.cleanse();
        }
    }
    catch (const std::exception&)
    {
        seal::Cryptography::cleanseString(exportData);
        seal::Cryptography::cleanseString(masterPassword);
        std::cerr << "Error: Decryption failed - wrong password or corrupted vault.\n";
        return 1;
    }
    seal::Cryptography::cleanseString(masterPassword);

    // Write to file or stdout.
    if (outputPath.empty())
    {
        std::cout << exportData;
    }
    else
    {
        std::ofstream out(outputPath);
        if (!out.good())
        {
            std::cerr << "Error: Could not open output file: " << outputPath << "\n";
            return 1;
        }
        out << exportData;
        out.close();
        std::cout << "Exported " << exportedCount << " credential(s) to " << outputPath << "\n";
    }

    seal::Cryptography::cleanseString(exportData);
    return 0;
}
#endif  // USE_QT_UI

static int handleStreamMode(bool encryptMode,
                            bool decryptMode,
                            seal::basic_secure_string<wchar_t>& password,
                            seal::DPAPIGuard<seal::basic_secure_string<wchar_t>>& dpapi)
{
    ScopedUnprotect dpapiScope(dpapi);
    bool success = false;
    if (encryptMode)
        success = seal::FileOperations::streamEncrypt(password);
    else if (decryptMode)
        success = seal::FileOperations::streamDecrypt(password);
    seal::Cryptography::cleanseString(password);
    return success ? 0 : 1;
}

// Process the "seal" input file (a text file named literally "seal" in the cwd).
// It contains paths/hex tokens, one per line, terminated by '?' or '!'.
// This runs once at startup as a batch before the interactive loop begins.
static void processSageFileBatch(seal::DPAPIGuard<seal::basic_secure_string<wchar_t>>& dpapi,
                                 seal::basic_secure_string<wchar_t>& password)
{
    std::ifstream fin("seal");
    if (!fin)
        return;

    auto fileBatch = seal::readBulkLinesDualFrom(fin);
    const auto& flines = fileBatch.first;
    if (flines.empty())
        return;

    ScopedUnprotect dpapiScope(dpapi);
    seal::FileOperations::processBatch(flines, fileBatch.second, password);
    std::cout << "\n";
}

// Re-read the "seal" input file on Esc press. Only processes the file if it
// contains actual file/directory paths (not raw hex), acting as a quick
// re-encrypt/re-decrypt shortcut before exiting the interactive loop.
static bool handleEscSageFile(seal::DPAPIGuard<seal::basic_secure_string<wchar_t>>& dpapi,
                              seal::basic_secure_string<wchar_t>& password)
{
    std::ifstream fin("seal");
    if (!fin)
        return false;

    auto fileBatch = seal::readBulkLinesDualFrom(fin);
    const auto& flines = fileBatch.first;
    if (flines.empty())
        return false;

    bool hasFiles = false;
    for (const auto& line : flines)
    {
        if (_stricmp(line.c_str(), ".") == 0 || seal::utils::isDirectoryA(line.c_str()) ||
            seal::utils::fileExistsA(line.c_str()))
        {
            hasFiles = true;
            break;
        }
    }
    if (hasFiles)
    {
        ScopedUnprotect dpapiScope(dpapi);
        seal::FileOperations::processBatch(flines, fileBatch.second, password);
    }
    return true;
}

static int handleCliMode(bool streamMode, bool encryptMode, bool decryptMode)
{
    if (encryptMode && decryptMode)
    {
        std::cerr << "Error: Cannot specify both -e and -d\n";
        return 1;
    }

    try
    {
        seal::basic_secure_string<wchar_t> password = seal::readPasswordConsole();
        seal::DPAPIGuard<seal::basic_secure_string<wchar_t>> dpapi(&password);

        if (streamMode)
            return handleStreamMode(encryptMode, decryptMode, password, dpapi);

        // CLI startup sequence:
        // 1. Process the "seal" input file as an automatic batch (if present).
        // 2. Enter the interactive loop where the user types/pastes lines
        //    terminated by '?' (masked output) or '!' (uncensored output).
        // 3. On Esc, re-read the "seal" file one more time, then exit cleanly.
        processSageFileBatch(dpapi, password);

        std::cout << "+----------------------------------------- seal - Interactive Mode "
                     "-----------------------------------------+\n";
        std::cout << "|              Paste/type and finish with '?' (MASKED) or '!' (UNCENSORED) "
                     "Press Esc to exit.               |\n";
        std::cout << "|    Commands '.'= current dir | ':clip'= copy seal | ':open'= edit seal | "
                     "':none'= clear clipboard         |\n";
        std::cout << "+----------------------------------------------------------------------------"
                     "-------------------------------+\n";

        for (;;)
        {
            std::pair<std::vector<std::string>, bool> batch;
            if (!seal::readBulkLinesDualOrEsc(batch))
            {
                (void)handleEscSageFile(dpapi, password);
                return 0;
            }
            if (batch.first.empty())
                break;

            ScopedUnprotect dpapiScope(dpapi);
            seal::FileOperations::processBatch(batch.first, batch.second, password);
        }
        seal::Cryptography::cleanseString(password);
        seal::wipeConsoleBuffer();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

// Program entry point. Dispatches to one of several modes based on CLI args:
//   -e/--encrypt, -d/--decrypt  : stream encryption/decryption (stdin->stdout)
//   -u/--ui / no args           : Qt GUI mode (default)
//   --cli                       : interactive console mode
//   --import / --export          : vault import/export
//   -h/--help                   : usage information
int main(int argc, char* argv[])
{
    ProgramOptions opts;
    int rc = parseArguments(argc, argv, opts);
    if (rc >= 0)
        return rc;

    const bool useUIMode = opts.uiMode || (!opts.cliMode && !opts.streamMode);
    // Qt Quick's QML engine uses a JIT compiler (V4) that needs dynamic code
    // generation. CLI / import / export never load QML, so they can keep the
    // stricter PROCESS_MITIGATION_DYNAMIC_CODE_POLICY that blocks RWX pages.
    const bool allowDynamicCode = useUIMode && !opts.importMode && !opts.exportMode;

    rc = initializeSecurity(allowDynamicCode);
    if (rc != 0)
        return rc;

    if (opts.importMode)
    {
#ifdef USE_QT_UI
        return handleImportMode(opts.importData, opts.importOutputPath);
#else
        std::cerr << "Error: --import requires Qt UI support (USE_QT_UI).\n";
        std::cerr << "Please rebuild with USE_QT_UI enabled.\n";
        return 1;
#endif
    }

    if (opts.exportMode)
    {
#ifdef USE_QT_UI
        return handleExportMode(opts.exportInputPath, opts.exportOutputPath);
#else
        std::cerr << "Error: --export requires Qt UI support (USE_QT_UI).\n";
        std::cerr << "Please rebuild with USE_QT_UI enabled.\n";
        return 1;
#endif
    }

    if (useUIMode)
    {
#ifdef USE_QT_UI
        return RunQMLMode(argc, argv);
#else
        std::cerr << "Error: GUI mode requested but Qt UI support is not compiled in.\n";
        std::cerr << "Please rebuild with USE_QT_UI enabled or use --cli for CLI mode.\n";
        return 1;
#endif
    }

    return handleCliMode(opts.streamMode, opts.encryptMode, opts.decryptMode);
}
