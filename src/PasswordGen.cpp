#include "PasswordGen.h"

#include <openssl/rand.h>

#include <windows.h>

#include <algorithm>
#include <string>

namespace seal
{

std::string GeneratePassword(int length)
{
    length = std::clamp(length, 8, 128);

    static constexpr char charset[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+";
    static constexpr int charsetLen = sizeof(charset) - 1;

    // Rejection sampling: Integer division truncates so `limit` is
    // the largest multiple of charsetLen that fits in a byte.
    static constexpr unsigned char limit =
        static_cast<unsigned char>((256 / charsetLen) * charsetLen);

    std::string password;
    password.reserve(static_cast<size_t>(length));

    unsigned char rndBuf[128];
    int filled = 0;
    while (filled < length)
    {
        int need = length - filled;
        // Over-request to reduce the number of RAND_bytes round-trips;
        // each byte has a ~70% acceptance rate (196/256 for charsetLen=76).
        int request = std::min(need * 2, static_cast<int>(sizeof(rndBuf)));
        RAND_bytes(rndBuf, request);
        for (int i = 0; i < request && filled < length; ++i)
        {
            if (rndBuf[i] < limit)
            {
                password.push_back(charset[rndBuf[i] % charsetLen]);
                ++filled;
            }
        }
    }
    SecureZeroMemory(rndBuf, sizeof(rndBuf));

    return password;
}

}  // namespace seal
