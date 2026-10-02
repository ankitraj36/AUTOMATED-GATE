#include "access_controller.hpp"
#include "auth_module.hpp"
#include "ui_utils.hpp"
#include <iostream>

namespace DoorAccess {

// ─── Constructor ──────────────────────────────────────────────────────────────

AccessController::AccessController(DatabaseManager& dbManager)
    : m_dbManager(dbManager) {}

// ─── Access by User ID ────────────────────────────────────────────────────────

AccessDecision AccessController::attemptAccessById(int userId,
                                                     const std::string& password) {
    if (!m_dbManager.isConnected()) {
        return {AccessResult::Denied,
                "System error: Database unavailable",
                "Database Connection Error",
                std::nullopt};
    }

    auto user = m_dbManager.findUserById(userId);
    std::string credential = "ID:" + std::to_string(userId);
    return evaluateAccess(user, password, credential);
}

// ─── Access by Username ───────────────────────────────────────────────────────

AccessDecision AccessController::attemptAccessByUsername(const std::string& username,
                                                          const std::string& password) {
    if (!m_dbManager.isConnected()) {
        return {AccessResult::Denied,
                "System error: Database unavailable",
                "Database Connection Error",
                std::nullopt};
    }

    auto user = m_dbManager.findUserByUsername(username);
    std::string credential = "user:" + username;
    return evaluateAccess(user, password, credential);
}

// ─── Access Evaluation ────────────────────────────────────────────────────────

AccessDecision AccessController::evaluateAccess(const std::optional<User>& user,
                                                  const std::string& password,
                                                  const std::string& credential) {
    // Check 1: User exists?
    if (!user.has_value()) {
        return {AccessResult::Denied,
                "Access denied. User not found.",
                "User Not Found",
                std::nullopt};
    }

    const User& u = user.value();

    // Check 2: Account active?
    if (u.accessStatus == AccessStatus::Inactive) {
        return {AccessResult::Denied,
                "Access denied. Account is inactive.",
                "Account Inactive",
                user};
    }

    // Check 3: Password correct?
    if (!Auth::verifyPassword(password, u.credentialHash, u.salt)) {
        return {AccessResult::Denied,
                "Access denied. Invalid credentials.",
                "Invalid Credentials",
                user};
    }

    // All checks passed — access granted
    return {AccessResult::Granted,
            "Welcome, " + u.fullName + "!",
            "",
            user};
}

} // namespace DoorAccess
