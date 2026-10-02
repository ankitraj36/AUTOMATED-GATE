#include "auth_module.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <algorithm>

namespace DoorAccess {
namespace Auth {

// ─── Constants ────────────────────────────────────────────────────────────────

static constexpr int SALT_BYTES      = 16;
static constexpr int HASH_BYTES      = 32;
static constexpr int PBKDF2_ITERATIONS = 100000;

// ─── Helper: Bytes to Hex String ──────────────────────────────────────────────

static std::string bytesToHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
    }
    return oss.str();
}

// ─── Helper: Hex String to Bytes ──────────────────────────────────────────────

static std::vector<unsigned char> hexToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        unsigned int byte;
        std::istringstream iss(hex.substr(i, 2));
        iss >> std::hex >> byte;
        bytes.push_back(static_cast<unsigned char>(byte));
    }
    return bytes;
}

// ─── Generate Salt ────────────────────────────────────────────────────────────

std::string generateSalt() {
    unsigned char saltBytes[SALT_BYTES];
    if (RAND_bytes(saltBytes, SALT_BYTES) != 1) {
        // Fallback: use time-based pseudo-random (less secure, but functional)
        srand(static_cast<unsigned>(time(nullptr)));
        for (int i = 0; i < SALT_BYTES; ++i) {
            saltBytes[i] = static_cast<unsigned char>(rand() % 256);
        }
    }
    return bytesToHex(saltBytes, SALT_BYTES);
}

// ─── Hash Password ───────────────────────────────────────────────────────────

std::string hashPassword(const std::string& password, const std::string& salt) {
    auto saltBytes = hexToBytes(salt);
    unsigned char hash[HASH_BYTES];

    PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.size()),
        saltBytes.data(),
        static_cast<int>(saltBytes.size()),
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        HASH_BYTES,
        hash
    );

    return bytesToHex(hash, HASH_BYTES);
}

// ─── Verify Password ─────────────────────────────────────────────────────────

bool verifyPassword(const std::string& password,
                    const std::string& storedHash,
                    const std::string& salt) {
    std::string computedHash = hashPassword(password, salt);
    // Constant-time comparison to prevent timing attacks
    if (computedHash.size() != storedHash.size()) return false;

    unsigned char result = 0;
    for (size_t i = 0; i < computedHash.size(); ++i) {
        result |= static_cast<unsigned char>(computedHash[i] ^ storedHash[i]);
    }
    return result == 0;
}

// ─── Mask Credential ─────────────────────────────────────────────────────────

std::string maskCredential(const std::string& credential) {
    if (credential.empty()) return "***";
    if (credential.size() <= 2) return std::string(credential.size(), '*');

    // Show first character and last character, mask the rest
    size_t visibleChars = std::min<size_t>(2, credential.size() / 2);
    std::string masked = credential.substr(0, visibleChars);
    masked += std::string(credential.size() - visibleChars, '*');
    return masked;
}

} // namespace Auth
} // namespace DoorAccess
