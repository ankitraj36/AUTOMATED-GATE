#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <sqlite3.h>
#include "types.hpp"

/**
 * @brief Database Module — Manages SQLite connection and all database operations.
 *
 * Uses prepared statements exclusively to prevent SQL injection.
 * Handles connection lifecycle, schema initialization, and CRUD operations.
 */

namespace DoorAccess {

class DatabaseManager {
public:
    /**
     * @brief Construct a new DatabaseManager.
     * @param dbPath Path to the SQLite database file.
     */
    explicit DatabaseManager(const std::string& dbPath);

    /**
     * @brief Destructor — closes the database connection.
     */
    ~DatabaseManager();

    // Non-copyable, movable
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    DatabaseManager(DatabaseManager&& other) noexcept;
    DatabaseManager& operator=(DatabaseManager&& other) noexcept;

    /**
     * @brief Open the database connection and initialize schema.
     * @return true if successful.
     */
    bool initialize();

    /**
     * @brief Close the database connection.
     */
    void close();

    /**
     * @brief Check if the database connection is open.
     */
    bool isConnected() const;

    // ─── User Operations ──────────────────────────────────────────────────

    /**
     * @brief Find a user by their user ID.
     */
    std::optional<User> findUserById(int userId);

    /**
     * @brief Find a user by their username.
     */
    std::optional<User> findUserByUsername(const std::string& username);

    /**
     * @brief Add a new user to the database.
     * @return The new user's ID, or -1 on failure.
     */
    int addUser(const std::string& fullName,
                const std::string& username,
                const std::string& credentialHash,
                const std::string& salt,
                Role role,
                AccessStatus status = AccessStatus::Active);

    /**
     * @brief Update a user's full name and role.
     */
    bool updateUser(int userId, const std::string& fullName, Role role);

    /**
     * @brief Update a user's access status (Active/Inactive).
     */
    bool updateUserStatus(int userId, AccessStatus status);

    /**
     * @brief Update a user's password hash and salt.
     */
    bool updateUserCredential(int userId, const std::string& hash, const std::string& salt);

    /**
     * @brief Remove a user from the database.
     */
    bool removeUser(int userId);

    /**
     * @brief Get all users.
     */
    std::vector<User> getAllUsers();

    /**
     * @brief Check if any users exist in the database.
     */
    bool hasUsers();

    // ─── Access Log Operations ────────────────────────────────────────────

    /**
     * @brief Log an access attempt.
     * @param userId User ID (0 for unknown users).
     * @param attemptedCredential Masked credential reference.
     * @param result Access result (Granted/Denied).
     * @param denialReason Reason for denial (empty if granted).
     * @return true if logged successfully.
     */
    bool logAccessAttempt(int userId,
                          const std::string& attemptedCredential,
                          AccessResult result,
                          const std::string& denialReason = "");

    /**
     * @brief Get all access logs, ordered by most recent first.
     * @param limit Maximum number of logs to return (0 = all).
     */
    std::vector<AccessLog> getAccessLogs(int limit = 0);

    /**
     * @brief Get access logs for a specific user.
     */
    std::vector<AccessLog> getAccessLogsByUserId(int userId);

    /**
     * @brief Search access logs by date range.
     * @param startDate Start date (YYYY-MM-DD format).
     * @param endDate End date (YYYY-MM-DD format).
     */
    std::vector<AccessLog> searchAccessLogs(const std::string& startDate,
                                            const std::string& endDate);

private:
    std::string m_dbPath;
    sqlite3*    m_db = nullptr;

    /**
     * @brief Create database tables if they don't exist.
     */
    bool createSchema();

    /**
     * @brief Insert seed data if the database is empty.
     */
    bool initializeSeedData();

    /**
     * @brief Parse a User from a prepared statement's current row.
     */
    User parseUserRow(sqlite3_stmt* stmt);

    /**
     * @brief Parse an AccessLog from a prepared statement's current row.
     */
    AccessLog parseAccessLogRow(sqlite3_stmt* stmt);
};

} // namespace DoorAccess
