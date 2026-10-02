#pragma once

#include <string>
#include <vector>
#include <optional>
#include <ctime>

/**
 * @brief Common data structures used throughout the Door Access Control System.
 */

namespace DoorAccess {

// ─── User Roles ───────────────────────────────────────────────────────────────

enum class Role {
    Admin,
    Employee,
    Student,
    Visitor
};

inline std::string roleToString(Role role) {
    switch (role) {
        case Role::Admin:    return "Admin";
        case Role::Employee: return "Employee";
        case Role::Student:  return "Student";
        case Role::Visitor:  return "Visitor";
    }
    return "Unknown";
}

inline std::optional<Role> stringToRole(const std::string& str) {
    if (str == "Admin")    return Role::Admin;
    if (str == "Employee") return Role::Employee;
    if (str == "Student")  return Role::Student;
    if (str == "Visitor")  return Role::Visitor;
    return std::nullopt;
}

// ─── Access Status ────────────────────────────────────────────────────────────

enum class AccessStatus {
    Active,
    Inactive
};

inline std::string accessStatusToString(AccessStatus status) {
    switch (status) {
        case AccessStatus::Active:   return "Active";
        case AccessStatus::Inactive: return "Inactive";
    }
    return "Unknown";
}

inline std::optional<AccessStatus> stringToAccessStatus(const std::string& str) {
    if (str == "Active")   return AccessStatus::Active;
    if (str == "Inactive") return AccessStatus::Inactive;
    return std::nullopt;
}

// ─── Access Result ────────────────────────────────────────────────────────────

enum class AccessResult {
    Granted,
    Denied
};

inline std::string accessResultToString(AccessResult result) {
    switch (result) {
        case AccessResult::Granted: return "Granted";
        case AccessResult::Denied:  return "Denied";
    }
    return "Unknown";
}

// ─── User Record ──────────────────────────────────────────────────────────────

struct User {
    int         userId;
    std::string fullName;
    std::string username;
    std::string credentialHash;
    std::string salt;
    Role        role;
    AccessStatus accessStatus;
    std::string  createdAt;
};

// ─── Access Log Record ────────────────────────────────────────────────────────

struct AccessLog {
    int         logId;
    int         userId;       // 0 if unknown user
    std::string attemptedCredential;
    std::string accessTime;
    AccessResult accessResult;
    std::string  denialReason;
};

// ─── Access Decision ──────────────────────────────────────────────────────────

struct AccessDecision {
    AccessResult result;
    std::string  message;
    std::string  denialReason;
    std::optional<User> user;
};

} // namespace DoorAccess
