#pragma once

#include "database_manager.hpp"
#include "access_controller.hpp"
#include "door_controller.hpp"
#include "logger.hpp"
#include <string>

/**
 * @brief Admin Module — Administrative user and log management interface.
 *
 * Provides functionality for authorized administrators to:
 *   - Add, update, deactivate, and remove users
 *   - View and search access logs
 *   - Manage system settings
 */

namespace DoorAccess {

class AdminPanel {
public:
    /**
     * @brief Construct an AdminPanel.
     * @param dbManager Reference to the active DatabaseManager.
     * @param logger Reference to the Logger.
     */
    AdminPanel(DatabaseManager& dbManager, Logger& logger);

    /**
     * @brief Run the admin panel interactive menu.
     * Requires admin authentication before granting access.
     */
    void run();

private:
    DatabaseManager& m_dbManager;
    Logger&          m_logger;

    /**
     * @brief Authenticate an administrator.
     * @return true if admin authentication succeeded.
     */
    bool authenticateAdmin();

    /**
     * @brief Display the admin menu and handle selections.
     */
    void showMenu();

    // ─── User Management ─────────────────────────────────────────────────

    void listUsers();
    void addUser();
    void updateUser();
    void deactivateUser();
    void activateUser();
    void removeUser();
    void resetUserPassword();

    // ─── Log Management ──────────────────────────────────────────────────

    void viewRecentLogs();
    void searchLogsByUser();
    void searchLogsByDate();

    // ─── Utilities ───────────────────────────────────────────────────────

    /**
     * @brief Print a formatted table of users.
     */
    void printUserTable(const std::vector<User>& users);

    /**
     * @brief Read a line of input with a prompt.
     */
    std::string readInput(const std::string& prompt);

    /**
     * @brief Read an integer with a prompt.
     */
    int readInt(const std::string& prompt);

    /**
     * @brief Read a password (echoes asterisks or hides input).
     */
    std::string readPassword(const std::string& prompt);
};

} // namespace DoorAccess
