#pragma once

#include <string>

namespace seal
{

/**
 * @brief Generate a cryptographically uniform random password.
 * @author Alex (https://github.com/lextpf)
 *
 * Uses OpenSSL `RAND_bytes` with rejection sampling to eliminate
 * modular bias. The charset includes uppercase, lowercase, digits,
 * and common symbols (76 characters total). Length is clamped to
 * [8, 128].
 *
 * @param length Desired password length (clamped to 8..128).
 * @return The generated password. Caller must cleanse after use.
 */
[[nodiscard]] std::string GeneratePassword(int length);

}  // namespace seal
