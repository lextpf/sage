#ifdef USE_QT_UI

#include "CliDispatch.h"

#include "Clipboard.h"
#include "FileOperations.h"
#include "Utils.h"

#include <QtCore/QString>

#include <windows.h>

#include <span>
#include <string>

namespace seal
{

void CliDispatchFile(const std::string& stripped, const CliDispatchCallbacks& cb)
{
    std::string target = stripped;

    std::string base = seal::utils::basenameA(target);
    if (seal::utils::endsWithCi(base, ".exe") || _stricmp(base.c_str(), "seal") == 0)
    {
        cb.output(QString("(skipped) %1").arg(QString::fromStdString(target)));
        return;
    }

    bool isSeal = seal::utils::endsWithCi(target, ".seal");
    if (isSeal)
    {
        std::string newName = seal::utils::strip_ext_ci(target, std::string_view{".seal"});
        bool ok = seal::FileOperations::decryptFileTo(target, newName, cb.password);
        if (ok)
        {
            DeleteFileA(target.c_str());
            cb.output(QString("(decrypted) %1 -> %2")
                          .arg(QString::fromStdString(target), QString::fromStdString(newName)));
        }
        else
        {
            cb.output(QString("(decrypt failed) %1").arg(QString::fromStdString(target)));
        }
    }
    else
    {
        std::string newName = seal::utils::add_ext(target, std::string_view{".seal"});
        bool ok = seal::FileOperations::encryptFileTo(target, newName, cb.password);
        if (ok)
        {
            DeleteFileA(target.c_str());
            cb.output(QString("(encrypted) %1 -> %2")
                          .arg(QString::fromStdString(target), QString::fromStdString(newName)));
        }
        else
        {
            cb.output(QString("(encrypt failed) %1").arg(QString::fromStdString(target)));
        }
    }
}

void CliDispatchHexTokens(const std::string& input, const CliDispatchCallbacks& cb)
{
    auto hexTokens = seal::utils::extractHexTokens(input);
    for (const auto& tok : hexTokens)
    {
        try
        {
            auto plain = seal::FileOperations::decryptLine(tok, cb.password);
            (void)seal::Clipboard::copyWithTTL(plain.view());
            // Mask plaintext in the output so it doesn't accumulate in the
            // QML string buffer. The value is on the clipboard (TTL-scrubbed).
            cb.output(
                QString("%1  [copied]").arg(QString(static_cast<int>(plain.size()), QChar('*'))));
            seal::Cryptography::cleanseString(plain);
        }
        catch (const std::exception& ex)
        {
            cb.output(QString("(decrypt failed: %1)").arg(QString::fromUtf8(ex.what())));
        }
    }
}

bool CliDispatchBase64(const std::string& input, const CliDispatchCallbacks& cb)
{
    try
    {
        auto bytes = seal::utils::fromBase64(input);
        if (!bytes.empty())
        {
            auto plain = seal::Cryptography::decryptPacket(std::span<const unsigned char>(bytes),
                                                           cb.password);
            (void)seal::Clipboard::copyWithTTL(reinterpret_cast<const char*>(plain.data()),
                                               plain.size());
            // Mask plaintext in the output - value is on the clipboard.
            cb.output(
                QString("%1  [copied]").arg(QString(static_cast<int>(plain.size()), QChar('*'))));
            seal::Cryptography::cleanseString(plain);
            return true;
        }
    }
    catch (...)
    {
        // Not valid base64 ciphertext, fall through to encrypt
    }
    return false;
}

void CliDispatchEncrypt(const std::string& input, const CliDispatchCallbacks& cb)
{
    std::string hex = seal::FileOperations::encryptLine(input, cb.password);
    // Convert hex back to raw bytes for base64 encoding
    std::vector<unsigned char> raw;
    seal::utils::from_hex(std::string_view{hex}, raw);
    std::string b64 = seal::utils::toBase64(std::span<const unsigned char>(raw));
    cb.output(QString("(hex) %1").arg(QString::fromStdString(hex)));
    cb.output(QString("(b64) %1").arg(QString::fromStdString(b64)));
}

}  // namespace seal

#endif  // USE_QT_UI
