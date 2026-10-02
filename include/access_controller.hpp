#pragma once

#include "types.hpp"
#include "database_manager.hpp"
#include <string>

/**
 * @brief Access Control Module — Makes access decisions based on credentials.
 *
 * Validates user credentials against the database and determines whether
 * to grant or deny access, providing clear reasons for denial.
 */

namespace DoorAccess {

class AccessController {
public:
    /**
     * @brief Construct an AccessController.
     * @param dbManager Reference to the active DatabaseManager.
     */
    explicit AccessController(DatabaseManager& dbManager);

    /**
     * @brief Attempt access using a user ID and password.
     * @param userId The user's ID number.
     * @param password The plaintext password.
     * @return An AccessDecision with the result and details.
     */
    AccessDecision attemptAccessById(int userId, const std::string& password);

    /**
     * @brief Attempt access using a username and password.
     * @param username The user's username.
     * @param password The plaintext password.
     * @return An AccessDecision with the result and details.
     */
    AccessDecision attemptAccessByUsername(const std::string& username,
                                           const std::string& password);

private:
    DatabaseManager& m_dbManager;

    /**
     * @brief Evaluate a user's authorization.
     */
    AccessDecision evaluateAccess(const std::optional<User>& user,
                                   const std::string& password,
                                   const std::string& credential);
};

} // namespace DoorAccess
