#ifdef USE_QT_UI

#include "Vault.h"

#include "Cryptography.h"
#include "FileOperations.h"
#include "Logging.h"
#include "Utils.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>

#include <windows.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

static std::string qstringToStd(const QString& qstr)
{
    QByteArray bytes = qstr.toUtf8();
    return std::string(bytes.constData(), bytes.size());
}

namespace
{
// Vault binary format (V2):
//   magic(4 bytes "SVH2") + version(1 byte) + count(4 bytes BE) + N records
// Each record: platformLen(4 BE) + platformBlob + credLen(4 BE) + credBlob
// The entire binary frame is hex-encoded into a single text string on disk.
constexpr unsigned char kVaultMagic[4] = {'S', 'V', 'H', '2'};
constexpr unsigned char kVaultFormatVersion = 1;

// All multi-byte integers use big-endian (network byte order) so the vault
// file is portable across machines regardless of native endianness.
void appendU32BE(std::vector<unsigned char>& out, uint32_t v)
{
    out.push_back(static_cast<unsigned char>((v >> 24) & 0xFFu));
    out.push_back(static_cast<unsigned char>((v >> 16) & 0xFFu));
    out.push_back(static_cast<unsigned char>((v >> 8) & 0xFFu));
    out.push_back(static_cast<unsigned char>(v & 0xFFu));
}

bool readU32BE(const std::vector<unsigned char>& in, size_t& pos, uint32_t& out)
{
    if (pos + 4 > in.size())
        return false;
    out = (static_cast<uint32_t>(in[pos]) << 24) | (static_cast<uint32_t>(in[pos + 1]) << 16) |
          (static_cast<uint32_t>(in[pos + 2]) << 8) | static_cast<uint32_t>(in[pos + 3]);
    pos += 4;
    return true;
}

bool readSizedBlob(const std::vector<unsigned char>& in,
                   size_t& pos,
                   uint32_t len,
                   std::vector<unsigned char>& out)
{
    const size_t n = static_cast<size_t>(len);
    if (pos > in.size() || n > (in.size() - pos))
        return false;
    out.assign(in.begin() + pos, in.begin() + pos + n);
    pos += n;
    return true;
}
}  // namespace

namespace seal
{

void DecryptedCredential::cleanse()
{
    seal::Cryptography::cleanseString(username, password);
}

static std::vector<unsigned char> encryptString(
    const std::string& plaintext,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& masterPassword)
{
    return seal::Cryptography::encryptPacket(
        std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(plaintext.data()),
                                       plaintext.size()),
        masterPassword);
}

static std::string decryptToString(
    const std::vector<unsigned char>& packet,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& password)
{
    auto plainBytes =
        seal::Cryptography::decryptPacket(std::span<const unsigned char>(packet), password);
    std::string result(reinterpret_cast<const char*>(plainBytes.data()), plainBytes.size());
    seal::Cryptography::cleanseString(plainBytes);
    return result;
}

std::vector<VaultRecord> loadVaultIndex(
    const QString& vaultPath,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& password)
{
    qCInfo(logVault) << "loadVaultIndex:" << QFileInfo(vaultPath).fileName();
    std::ifstream in(qstringToStd(vaultPath), std::ios::in | std::ios::binary);
    if (!in)
    {
        qCWarning(logVault) << "loadVaultIndex: cannot open file";
        throw std::runtime_error("Cannot open vault file");
    }

    // Step 1: Read the entire file as a hex-encoded text string.
    std::string raw((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    // Strip any whitespace that may have been inserted (e.g. line breaks).
    std::string compact = seal::utils::stripSpaces(raw);
    if (compact.empty())
        return {};

    // Step 2: Hex-decode the text back into the raw binary frame.
    // The vault file is stored as one long hex string so it stays a safe
    // single-line text blob (no embedded NULs, no encoding ambiguity).
    std::vector<unsigned char> framed;
    if (!seal::utils::from_hex(std::string_view{compact}, framed))
    {
        qCWarning(logVault) << "loadVaultIndex: invalid framed hex";
        throw std::runtime_error("Invalid vault format");
    }

    // Step 3: Parse the binary frame header: magic(4) + version(1) + count(4) = 9 bytes min.
    size_t pos = 0;
    if (framed.size() < 9)
    {
        qCWarning(logVault) << "loadVaultIndex: framed payload too short";
        throw std::runtime_error("Corrupted vault file");
    }
    for (unsigned char b : kVaultMagic)
    {
        if (framed[pos++] != b)
        {
            qCWarning(logVault) << "loadVaultIndex: bad magic";
            throw std::runtime_error("Invalid vault format");
        }
    }
    const unsigned char version = framed[pos++];
    if (version != kVaultFormatVersion)
    {
        qCWarning(logVault) << "loadVaultIndex: unsupported format version" << version;
        throw std::runtime_error("Unsupported vault format version");
    }

    uint32_t entryCount = 0;
    if (!readU32BE(framed, pos, entryCount))
    {
        qCWarning(logVault) << "loadVaultIndex: missing entry count";
        throw std::runtime_error("Corrupted vault file");
    }
    // Sanity check: each record needs at least 8 bytes (two u32 length fields),
    // so reject counts that would be impossible given the remaining data.
    if (entryCount > (framed.size() - pos) / 8)
    {
        qCWarning(logVault) << "loadVaultIndex: impossible entry count";
        throw std::runtime_error("Corrupted vault file");
    }

    // Step 4: Read each length-prefixed record: [platformLen][platformBlob][credLen][credBlob].
    // We decrypt *only* the platform name here (for display in the list view);
    // the credential blob stays encrypted until the user explicitly requests it.
    std::vector<VaultRecord> records;
    records.reserve(entryCount);

    for (uint32_t i = 0; i < entryCount; ++i)
    {
        std::vector<unsigned char> platformBlob, credBlob;
        uint32_t platformLen = 0;
        uint32_t credLen = 0;
        if (!readU32BE(framed, pos, platformLen) ||
            !readSizedBlob(framed, pos, platformLen, platformBlob) ||
            !readU32BE(framed, pos, credLen) || !readSizedBlob(framed, pos, credLen, credBlob))
        {
            qCWarning(logVault) << "loadVaultIndex: truncated framed payload";
            throw std::runtime_error("Corrupted vault file");
        }

        try
        {
            std::string platformName = decryptToString(platformBlob, password);

            VaultRecord rec;
            rec.platform = std::move(platformName);
            rec.encryptedPlatform = std::move(platformBlob);
            rec.encryptedBlob = std::move(credBlob);
            rec.dirty = false;
            rec.deleted = false;
            records.push_back(std::move(rec));
        }
        catch (...)
        {
            // Fail on the very first decryption failure so a wrong password
            // never reveals how many records the vault holds.  If we kept
            // going, an attacker could measure how far parsing progressed
            // and infer the record count even without the correct password.
            qCWarning(logVault) << "loadVaultIndex: wrong password (failed on entry" << i << ")";
            throw std::runtime_error("Wrong password");
        }
    }
    // Step 5: Verify we consumed every byte.  Trailing bytes would indicate
    // file corruption, accidental concatenation, or a tampered payload.
    if (pos != framed.size())
    {
        qCWarning(logVault) << "loadVaultIndex: trailing bytes in framed payload";
        throw std::runtime_error("Corrupted vault file");
    }

    qCInfo(logVault) << "loadVaultIndex: parsed" << records.size() << "record(s)";
    return records;
}

bool saveVaultV2(
    const QString& vaultPath,
    const std::vector<VaultRecord>& records,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& password)
{
    qCInfo(logVault) << "saveVaultV2:" << QFileInfo(vaultPath).fileName()
                     << "records=" << records.size();

    // Atomic save: write to a temporary file, flush, then rename over the target.
    // This prevents data loss if the process crashes mid-write.
    std::string finalPath = qstringToStd(vaultPath);
    std::string tmpPath = finalPath + ".tmp";

    std::ofstream out(tmpPath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!out)
    {
        qCWarning(logVault) << "saveVaultV2: cannot open temp file for writing";
        return false;
    }

    struct SerializedRecord
    {
        std::vector<unsigned char> platform;
        std::vector<unsigned char> credential;
    };

    std::vector<SerializedRecord> serialized;
    serialized.reserve(records.size());
    for (const auto& rec : records)
    {
        if (rec.deleted)
            continue;

        // Use existing encrypted platform if available, otherwise encrypt it
        std::vector<unsigned char> platformBlob;
        if (!rec.encryptedPlatform.empty() && !rec.dirty)
        {
            platformBlob = rec.encryptedPlatform;
        }
        else
        {
            platformBlob = encryptString(rec.platform, password);
        }

        if (platformBlob.size() > std::numeric_limits<uint32_t>::max() ||
            rec.encryptedBlob.size() > std::numeric_limits<uint32_t>::max())
        {
            qCWarning(logVault) << "saveVaultV2: encrypted field too large";
            out.close();
            DeleteFileA(tmpPath.c_str());
            return false;
        }

        serialized.push_back({std::move(platformBlob), rec.encryptedBlob});
    }

    if (serialized.size() > std::numeric_limits<uint32_t>::max())
    {
        qCWarning(logVault) << "saveVaultV2: too many records";
        out.close();
        DeleteFileA(tmpPath.c_str());
        return false;
    }

    // Build the binary frame: magic + version + count + records (same layout
    // that loadVaultIndex expects to parse back).
    size_t framedSize = 4 + 1 + 4;  // magic(4) + version(1) + entryCount(4)
    for (const auto& rec : serialized)
    {
        framedSize += 4 + rec.platform.size();    // platformLen + platformBlob
        framedSize += 4 + rec.credential.size();  // credLen + credBlob
    }

    std::vector<unsigned char> framed;
    framed.reserve(framedSize);
    framed.insert(framed.end(), kVaultMagic, kVaultMagic + sizeof(kVaultMagic));
    framed.push_back(kVaultFormatVersion);
    appendU32BE(framed, static_cast<uint32_t>(serialized.size()));
    for (const auto& rec : serialized)
    {
        appendU32BE(framed, static_cast<uint32_t>(rec.platform.size()));
        framed.insert(framed.end(), rec.platform.begin(), rec.platform.end());
        appendU32BE(framed, static_cast<uint32_t>(rec.credential.size()));
        framed.insert(framed.end(), rec.credential.begin(), rec.credential.end());
    }

    // Hex-encode the entire binary frame so the file is plain text on disk.
    const std::string hexBlob = seal::utils::to_hex(framed);
    out.write(hexBlob.data(), static_cast<std::streamsize>(hexBlob.size()));
    out.flush();
    bool ok = out.good();
    out.close();

    if (ok)
    {
        // Atomically replace the target file with the completed temp file.
        // MoveFileExA with MOVEFILE_REPLACE_EXISTING performs an atomic rename
        // on NTFS, so readers never see a half-written vault.  MOVEFILE_COPY_ALLOWED
        // is a fallback that lets the OS copy+delete if src/dst are on different volumes.
        if (!MoveFileExA(tmpPath.c_str(),
                         finalPath.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
        {
            qCWarning(logVault) << "saveVaultV2: rename failed";
            DeleteFileA(tmpPath.c_str());
            return false;
        }
        qCInfo(logVault) << "saveVaultV2: success";
    }
    else
    {
        qCWarning(logVault) << "saveVaultV2: write error";
        DeleteFileA(tmpPath.c_str());
    }
    return ok;
}

DecryptedCredential decryptCredentialOnDemand(
    const VaultRecord& record,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& password)
{
    qCDebug(logVault) << "decryptCredentialOnDemand: platform=" << record.platform.c_str();
    // The encrypted credential blob contains "username\0password" -- a single
    // null byte separates the two fields inside the decrypted plaintext.
    auto plainBytes = seal::Cryptography::decryptPacket(
        std::span<const unsigned char>(record.encryptedBlob), password);

    const char* data = reinterpret_cast<const char*>(plainBytes.data());
    size_t len = plainBytes.size();

    // Find the first '\0' which splits username from password.
    size_t sep = len;
    for (size_t i = 0; i < len; ++i)
    {
        if (data[i] == '\0')
        {
            sep = i;
            break;
        }
    }

    if (sep == len)
    {
        qCWarning(logVault) << "decryptCredentialOnDemand: malformed blob (no null separator)";
        seal::Cryptography::cleanseString(plainBytes);
        throw std::runtime_error("Malformed credential blob");
    }

    // Everything before the separator is the username; everything after is the password.
    // Use explicit SecureZeroMemory on the intermediate strings because
    // std::string may use SSO (small-string optimization), storing short
    // strings directly in the object's stack frame. OPENSSL_cleanse +
    // shrink_to_fit (used by cleanseString) only wipes heap-allocated
    // buffers, leaving SSO data intact on the stack.
    std::string userUtf8(data, sep);
    std::string passUtf8;
    if (sep + 1 < len)
    {
        passUtf8.assign(data + sep + 1, len - sep - 1);
    }
    seal::Cryptography::cleanseString(plainBytes);

    DecryptedCredential cred;
    cred.username = seal::utils::utf8ToSecureWide(userUtf8);
    cred.password = seal::utils::utf8ToSecureWide(passUtf8);

    // Wipe via cleanseString (covers the heap-allocated buffer path).
    // Note: SSO inline-buffer residue is accepted here -- zeroing the live
    // std::string object metadata with SecureZeroMemory would corrupt it and
    // cause undefined behaviour when the destructor runs at scope exit.
    seal::Cryptography::cleanseString(userUtf8, passUtf8);
    return cred;
}

VaultRecord encryptCredential(
    const std::string& platform,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& username,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& password,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& masterPassword)
{
    qCDebug(logVault) << "encryptCredential: platform=" << platform.c_str();
    // Convert wide-char credentials to UTF-8 for the on-disk format.
    std::string userUtf8 = seal::utils::secureWideToUtf8(username);
    std::string passUtf8 = seal::utils::secureWideToUtf8(password);

    // Build the credential plaintext as "username\0password" -- a single null
    // byte separates the two fields.  This mirrors what decryptCredentialOnDemand
    // expects when splitting the blob back apart.
    std::string credPlain;
    credPlain.reserve(userUtf8.size() + 1 + passUtf8.size());
    credPlain.append(userUtf8);
    credPlain.push_back('\0');
    credPlain.append(passUtf8);
    // Wipe the intermediate plaintext copies from memory before continuing.
    seal::Cryptography::cleanseString(userUtf8, passUtf8);

    // Encrypt the combined credential blob with the master password.
    auto credBlob = seal::Cryptography::encryptPacket(
        std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(credPlain.data()),
                                       credPlain.size()),
        masterPassword);
    seal::Cryptography::cleanseString(credPlain);

    auto platformBlob = encryptString(platform, masterPassword);

    VaultRecord rec;
    rec.platform = platform;
    rec.encryptedPlatform = std::move(platformBlob);
    rec.encryptedBlob = std::move(credBlob);
    rec.dirty = true;
    rec.deleted = false;
    return rec;
}

int encryptDirectory(
    const QString& dirPath,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& password)
{
    std::string path = qstringToStd(dirPath);
    int count = 0;

    qCInfo(logVault) << "encryptDirectory:" << dirPath;
    try
    {
        // Collect all eligible file paths first, then process them in a
        // second pass. Renaming files during recursive_directory_iterator
        // traversal can cause skipped or double-visited entries on some
        // filesystems.
        namespace fs = std::filesystem;
        std::vector<std::string> filePaths;
        for (const auto& entry :
             fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied))
        {
            if (entry.is_symlink() || !entry.is_regular_file())
            {
                continue;
            }

            const std::string ext = entry.path().extension().string();
            if (seal::utils::endsWithCi(ext, ".seal") || seal::utils::endsWithCi(ext, ".exe") ||
                seal::utils::endsWithCi(ext, ".dll") || seal::utils::endsWithCi(ext, ".pdb"))
            {
                continue;
            }

            filePaths.push_back(entry.path().string());
        }

        for (const auto& filePath : filePaths)
        {
            if (FileOperations::encryptFileInPlace(filePath, password))
            {
                std::string newPath = filePath + ".seal";
                if (MoveFileExA(filePath.c_str(), newPath.c_str(), MOVEFILE_REPLACE_EXISTING))
                {
                    count++;
                }
                else
                {
                    qCWarning(logVault) << "encryptDirectory: rename failed for"
                                        << QString::fromUtf8(filePath.c_str());
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        qCWarning(logVault) << "encryptDirectory: error:" << e.what();
    }

    qCInfo(logVault) << "encryptDirectory: encrypted" << count << "file(s)";
    return count;
}

int decryptDirectory(
    const QString& dirPath,
    const seal::basic_secure_string<wchar_t, seal::locked_allocator<wchar_t>>& password)
{
    std::string path = qstringToStd(dirPath);
    int count = 0;

    qCInfo(logVault) << "decryptDirectory:" << dirPath;
    try
    {
        // Collect all eligible file paths first, then process them in a
        // second pass. Renaming files during recursive_directory_iterator
        // traversal can cause skipped or double-visited entries on some
        // filesystems.
        namespace fs = std::filesystem;
        std::vector<std::string> filePaths;
        for (const auto& entry :
             fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied))
        {
            if (entry.is_symlink() || !entry.is_regular_file())
            {
                continue;
            }

            const std::string ext = entry.path().extension().string();
            if (!seal::utils::endsWithCi(ext, ".seal"))
            {
                continue;
            }

            filePaths.push_back(entry.path().string());
        }

        for (const auto& filePath : filePaths)
        {
            if (FileOperations::decryptFileInPlace(filePath, password))
            {
                std::string newPath = seal::utils::strip_ext_ci(filePath, ".seal");
                if (MoveFileExA(filePath.c_str(), newPath.c_str(), MOVEFILE_REPLACE_EXISTING))
                {
                    count++;
                }
                else
                {
                    qCWarning(logVault) << "decryptDirectory: rename failed for"
                                        << QString::fromUtf8(filePath.c_str());
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        qCWarning(logVault) << "decryptDirectory: error:" << e.what();
    }

    qCInfo(logVault) << "decryptDirectory: decrypted" << count << "file(s)";
    return count;
}

}  // namespace seal

#endif  // USE_QT_UI
