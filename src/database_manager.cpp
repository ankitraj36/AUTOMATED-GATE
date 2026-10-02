#include "database_manager.hpp"
#include "auth_module.hpp"
#include "ui_utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace DoorAccess {

// ─── Constructor / Destructor ─────────────────────────────────────────────────

DatabaseManager::DatabaseManager(const std::string& dbPath)
    : m_dbPath(dbPath), m_db(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close();
}

DatabaseManager::DatabaseManager(DatabaseManager&& other) noexcept
    : m_dbPath(std::move(other.m_dbPath)), m_db(other.m_db) {
    other.m_db = nullptr;
}

DatabaseManager& DatabaseManager::operator=(DatabaseManager&& other) noexcept {
    if (this != &other) {
        close();
        m_dbPath = std::move(other.m_dbPath);
        m_db = other.m_db;
        other.m_db = nullptr;
    }
    return *this;
}

// ─── Connection Management ────────────────────────────────────────────────────

bool DatabaseManager::initialize() {
    int rc = sqlite3_open(m_dbPath.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(m_db) << std::endl;
        m_db = nullptr;
        return false;
    }

    // Enable foreign keys
    char* errMsg = nullptr;
    sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg);
    if (errMsg) {
        sqlite3_free(errMsg);
    }

    // Enable WAL mode for better concurrent performance
    sqlite3_exec(m_db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, &errMsg);
    if (errMsg) {
        sqlite3_free(errMsg);
    }

    if (!createSchema()) {
        std::cerr << "Failed to create database schema." << std::endl;
        return false;
    }

    if (!hasUsers()) {
        if (!initializeSeedData()) {
            std::cerr << "Warning: Failed to initialize seed data." << std::endl;
        }
    }

    return true;
}

void DatabaseManager::close() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool DatabaseManager::isConnected() const {
    return m_db != nullptr;
}

// ─── Schema Creation ──────────────────────────────────────────────────────────

bool DatabaseManager::createSchema() {
    const char* schema = R"SQL(
        CREATE TABLE IF NOT EXISTS users (
            user_id         INTEGER PRIMARY KEY AUTOINCREMENT,
            full_name       TEXT    NOT NULL,
            username        TEXT    NOT NULL UNIQUE,
            credential_hash TEXT    NOT NULL,
            salt            TEXT    NOT NULL,
            role            TEXT    NOT NULL CHECK (role IN ('Admin', 'Employee', 'Student', 'Visitor')),
            access_status   TEXT    NOT NULL DEFAULT 'Active' CHECK (access_status IN ('Active', 'Inactive')),
            created_at      TEXT    NOT NULL DEFAULT (datetime('now', 'localtime'))
        );

        CREATE TABLE IF NOT EXISTS access_logs (
            log_id               INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id              INTEGER,
            attempted_credential TEXT    NOT NULL,
            access_time          TEXT    NOT NULL DEFAULT (datetime('now', 'localtime')),
            access_result        TEXT    NOT NULL CHECK (access_result IN ('Granted', 'Denied')),
            denial_reason        TEXT,
            FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL
        );

        CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
        CREATE INDEX IF NOT EXISTS idx_access_logs_user_id ON access_logs(user_id);
        CREATE INDEX IF NOT EXISTS idx_access_logs_time ON access_logs(access_time);
    )SQL";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, schema, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Schema creation error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// ─── Seed Data ────────────────────────────────────────────────────────────────

bool DatabaseManager::initializeSeedData() {
    struct SeedUser {
        std::string fullName;
        std::string username;
        std::string password;
        Role        role;
        AccessStatus status;
    };

    std::vector<SeedUser> seedUsers = {
        {"Ankit Raj",     "admin",  "admin123",    Role::Admin,    AccessStatus::Active},
        {"Ruman Kumar",   "ruman",  "password123", Role::Employee, AccessStatus::Active},
        {"Priya Sharma",  "priya",  "password123", Role::Employee, AccessStatus::Active},
        {"Rahul Verma",   "rahul",  "password123", Role::Student,  AccessStatus::Active},
        {"Sneha Gupta",   "sneha",  "password123", Role::Student,  AccessStatus::Inactive},
        {"John Doe",      "john",   "visitor123",  Role::Visitor,  AccessStatus::Active},
    };

    for (const auto& user : seedUsers) {
        std::string salt = Auth::generateSalt();
        std::string hash = Auth::hashPassword(user.password, salt);

        int result = addUser(user.fullName, user.username, hash, salt,
                             user.role, user.status);
        if (result < 0) {
            std::cerr << "Failed to add seed user: " << user.username << std::endl;
            return false;
        }
    }

    return true;
}

// ─── User Operations ──────────────────────────────────────────────────────────

std::optional<User> DatabaseManager::findUserById(int userId) {
    const char* sql = "SELECT user_id, full_name, username, credential_hash, "
                      "salt, role, access_status, created_at "
                      "FROM users WHERE user_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "DB Error: " << sqlite3_errmsg(m_db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, userId);

    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = parseUserRow(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> DatabaseManager::findUserByUsername(const std::string& username) {
    const char* sql = "SELECT user_id, full_name, username, credential_hash, "
                      "salt, role, access_status, created_at "
                      "FROM users WHERE username = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "DB Error: " << sqlite3_errmsg(m_db) << std::endl;
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = parseUserRow(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

int DatabaseManager::addUser(const std::string& fullName,
                              const std::string& username,
                              const std::string& credentialHash,
                              const std::string& salt,
                              Role role,
                              AccessStatus status) {
    const char* sql = "INSERT INTO users (full_name, username, credential_hash, "
                      "salt, role, access_status) VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "DB Error: " << sqlite3_errmsg(m_db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, fullName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, credentialHash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, salt.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, roleToString(role).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, accessStatusToString(status).c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "DB Error inserting user: " << sqlite3_errmsg(m_db) << std::endl;
        return -1;
    }

    return static_cast<int>(sqlite3_last_insert_rowid(m_db));
}

bool DatabaseManager::updateUser(int userId, const std::string& fullName, Role role) {
    const char* sql = "UPDATE users SET full_name = ?, role = ? WHERE user_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, fullName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, roleToString(role).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, userId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && sqlite3_changes(m_db) > 0;
}

bool DatabaseManager::updateUserStatus(int userId, AccessStatus status) {
    const char* sql = "UPDATE users SET access_status = ? WHERE user_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, accessStatusToString(status).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, userId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && sqlite3_changes(m_db) > 0;
}

bool DatabaseManager::updateUserCredential(int userId, const std::string& hash,
                                            const std::string& salt) {
    const char* sql = "UPDATE users SET credential_hash = ?, salt = ? WHERE user_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, salt.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, userId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && sqlite3_changes(m_db) > 0;
}

bool DatabaseManager::removeUser(int userId) {
    const char* sql = "DELETE FROM users WHERE user_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, userId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && sqlite3_changes(m_db) > 0;
}

std::vector<User> DatabaseManager::getAllUsers() {
    const char* sql = "SELECT user_id, full_name, username, credential_hash, "
                      "salt, role, access_status, created_at "
                      "FROM users ORDER BY user_id;";

    std::vector<User> users;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return users;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        users.push_back(parseUserRow(stmt));
    }

    sqlite3_finalize(stmt);
    return users;
}

bool DatabaseManager::hasUsers() {
    const char* sql = "SELECT COUNT(*) FROM users;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    bool result = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return result;
}

// ─── Access Log Operations ────────────────────────────────────────────────────

bool DatabaseManager::logAccessAttempt(int userId,
                                        const std::string& attemptedCredential,
                                        AccessResult result,
                                        const std::string& denialReason) {
    const char* sql = "INSERT INTO access_logs (user_id, attempted_credential, "
                      "access_result, denial_reason) VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "DB Error: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    if (userId > 0) {
        sqlite3_bind_int(stmt, 1, userId);
    } else {
        sqlite3_bind_null(stmt, 1);
    }

    sqlite3_bind_text(stmt, 2, attemptedCredential.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, accessResultToString(result).c_str(), -1, SQLITE_TRANSIENT);

    if (denialReason.empty()) {
        sqlite3_bind_null(stmt, 4);
    } else {
        sqlite3_bind_text(stmt, 4, denialReason.c_str(), -1, SQLITE_STATIC);
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "DB Error logging access: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    return true;
}

std::vector<AccessLog> DatabaseManager::getAccessLogs(int limit) {
    std::string sql = "SELECT log_id, user_id, attempted_credential, "
                      "access_time, access_result, denial_reason "
                      "FROM access_logs ORDER BY access_time DESC";
    if (limit > 0) {
        sql += " LIMIT " + std::to_string(limit);
    }
    sql += ";";

    std::vector<AccessLog> logs;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return logs;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        logs.push_back(parseAccessLogRow(stmt));
    }

    sqlite3_finalize(stmt);
    return logs;
}

std::vector<AccessLog> DatabaseManager::getAccessLogsByUserId(int userId) {
    const char* sql = "SELECT log_id, user_id, attempted_credential, "
                      "access_time, access_result, denial_reason "
                      "FROM access_logs WHERE user_id = ? "
                      "ORDER BY access_time DESC;";

    std::vector<AccessLog> logs;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return logs;
    }

    sqlite3_bind_int(stmt, 1, userId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        logs.push_back(parseAccessLogRow(stmt));
    }

    sqlite3_finalize(stmt);
    return logs;
}

std::vector<AccessLog> DatabaseManager::searchAccessLogs(const std::string& startDate,
                                                          const std::string& endDate) {
    const char* sql = "SELECT log_id, user_id, attempted_credential, "
                      "access_time, access_result, denial_reason "
                      "FROM access_logs "
                      "WHERE date(access_time) BETWEEN ? AND ? "
                      "ORDER BY access_time DESC;";

    std::vector<AccessLog> logs;
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return logs;
    }

    sqlite3_bind_text(stmt, 1, startDate.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, endDate.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        logs.push_back(parseAccessLogRow(stmt));
    }

    sqlite3_finalize(stmt);
    return logs;
}

// ─── Row Parsers ──────────────────────────────────────────────────────────────

User DatabaseManager::parseUserRow(sqlite3_stmt* stmt) {
    User user;
    user.userId         = sqlite3_column_int(stmt, 0);
    user.fullName       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    user.username       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    user.credentialHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    user.salt           = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

    std::string roleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    auto role = stringToRole(roleStr);
    user.role = role.value_or(Role::Visitor);

    std::string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    auto status = stringToAccessStatus(statusStr);
    user.accessStatus = status.value_or(AccessStatus::Inactive);

    user.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

    return user;
}

AccessLog DatabaseManager::parseAccessLogRow(sqlite3_stmt* stmt) {
    AccessLog log;
    log.logId = sqlite3_column_int(stmt, 0);

    if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
        log.userId = sqlite3_column_int(stmt, 1);
    } else {
        log.userId = 0;
    }

    log.attemptedCredential = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    log.accessTime          = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

    std::string resultStr   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    log.accessResult = (resultStr == "Granted") ? AccessResult::Granted : AccessResult::Denied;

    if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
        log.denialReason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }

    return log;
}

} // namespace DoorAccess
