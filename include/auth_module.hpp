#pragma once

#include <string>

/**
 * @brief Authentication Module — Handles password hashing and verification.
 *
 * Uses PBKDF2-HMAC-SHA256 via OpenSSL for secure password hashing.
 * Each password is hashed with a unique random salt.
 */

namespace DoorAccess {
namespace Auth {

    /**
     * @brief Generate a cryptographically secure random salt.
     * @return Hex-encoded salt string.
     */
    std::string generateSalt();

    /**
     * @brief Hash a password using PBKDF2-HMAC-SHA256.
     * @param password The plaintext password.
     * @param salt The hex-encoded salt.
     * @return Hex-encoded hash string.
     */
    std::string hashPassword(const std::string& password, const std::string& salt);

    /**
     * @brief Verify a password against a stored hash.
     * @param password The plaintext password to verify.
     * @param storedHash The stored hex-encoded hash.
     * @param salt The hex-encoded salt used for the stored hash.
     * @return true if the password matches.
     */
    bool verifyPassword(const std::string& password,
                        const std::string& storedHash,
                        const std::string& salt);

    /**
     * @brief Mask a credential for safe logging.
     * @param credential The credential to mask (e.g., username or ID).
     * @return A masked version (e.g., "adm***" for "admin").
     */
    std::string maskCredential(const std::string& credential);

} // namespace Auth
} // namespace DoorAccess
