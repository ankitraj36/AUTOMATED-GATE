#include "admin_panel.hpp"
#include "auth_module.hpp"
#include "ui_utils.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <termios.h>
#include <unistd.h>

namespace DoorAccess {

// ─── Constructor ──────────────────────────────────────────────────────────────

AdminPanel::AdminPanel(DatabaseManager& dbManager, Logger& logger)
    : m_dbManager(dbManager), m_logger(logger) {}

// ─── Run Admin Panel ──────────────────────────────────────────────────────────

void AdminPanel::run() {
    UI::clearScreen();
    std::cout << "\n";
    std::cout << UI::BOLD << UI::MAGENTA;
    std::cout << "  ╔════════════════════════════════════════════════╗\n";
    std::cout << "  ║           🔐 ADMIN CONTROL PANEL 🔐           ║\n";
    std::cout << "  ╚════════════════════════════════════════════════╝\n";
    std::cout << UI::RESET << "\n";

    if (!authenticateAdmin()) {
        UI::printError("Admin authentication failed. Returning to main menu.");
        UI::pressEnterToContinue();
        return;
    }

    UI::printSuccess("Admin authentication successful!");
    showMenu();
}

// ─── Admin Authentication ─────────────────────────────────────────────────────

bool AdminPanel::authenticateAdmin() {
    std::cout << UI::YELLOW << "  Admin authentication required.\n" << UI::RESET;

    std::string username = readInput("  Admin Username: ");
    std::string password = readPassword("  Admin Password: ");

    auto user = m_dbManager.findUserByUsername(username);
    if (!user.has_value()) {
        return false;
    }

    if (user.value().role != Role::Admin) {
        UI::printError("User is not an administrator.");
        return false;
    }

    if (user.value().accessStatus == AccessStatus::Inactive) {
        UI::printError("Admin account is inactive.");
        return false;
    }

    return Auth::verifyPassword(password, user.value().credentialHash,
                                 user.value().salt);
}

// ─── Admin Menu ───────────────────────────────────────────────────────────────

void AdminPanel::showMenu() {
    bool running = true;

    while (running) {
        std::cout << "\n";
        std::cout << UI::BOLD << UI::CYAN;
        std::cout << "  ┌──────────────────────────────────────┐\n";
        std::cout << "  │         ADMIN MENU OPTIONS           │\n";
        std::cout << "  ├──────────────────────────────────────┤\n";
        std::cout << UI::RESET;
        std::cout << "  │  " << UI::BOLD << "1" << UI::RESET << ". List All Users                  │\n";
        std::cout << "  │  " << UI::BOLD << "2" << UI::RESET << ". Add New User                    │\n";
        std::cout << "  │  " << UI::BOLD << "3" << UI::RESET << ". Update User                     │\n";
        std::cout << "  │  " << UI::BOLD << "4" << UI::RESET << ". Deactivate User                 │\n";
        std::cout << "  │  " << UI::BOLD << "5" << UI::RESET << ". Activate User                   │\n";
        std::cout << "  │  " << UI::BOLD << "6" << UI::RESET << ". Remove User                     │\n";
        std::cout << "  │  " << UI::BOLD << "7" << UI::RESET << ". Reset User Password             │\n";
        std::cout << "  │  " << UI::BOLD << "8" << UI::RESET << ". View Recent Access Logs         │\n";
        std::cout << "  │  " << UI::BOLD << "9" << UI::RESET << ". Search Logs by User ID          │\n";
        std::cout << "  │ " << UI::BOLD << "10" << UI::RESET << ". Search Logs by Date Range       │\n";
        std::cout << "  │  " << UI::BOLD << "0" << UI::RESET << ". Return to Main Menu             │\n";
        std::cout << UI::CYAN;
        std::cout << "  └──────────────────────────────────────┘\n";
        std::cout << UI::RESET;

        int choice = readInt("  Select option: ");

        switch (choice) {
            case 1:  listUsers();         break;
            case 2:  addUser();           break;
            case 3:  updateUser();        break;
            case 4:  deactivateUser();    break;
            case 5:  activateUser();      break;
            case 6:  removeUser();        break;
            case 7:  resetUserPassword(); break;
            case 8:  viewRecentLogs();    break;
            case 9:  searchLogsByUser();  break;
            case 10: searchLogsByDate();  break;
            case 0:  running = false;     break;
            default:
                UI::printWarning("Invalid option. Please try again.");
                break;
        }
    }
}

// ─── User Management ─────────────────────────────────────────────────────────

void AdminPanel::listUsers() {
    auto users = m_dbManager.getAllUsers();
    printUserTable(users);
    UI::pressEnterToContinue();
}

void AdminPanel::addUser() {
    std::cout << "\n" << UI::BOLD << "  ── Add New User ──" << UI::RESET << "\n\n";

    std::string fullName = readInput("  Full Name: ");
    std::string username = readInput("  Username: ");

    // Check if username already exists
    if (m_dbManager.findUserByUsername(username).has_value()) {
        UI::printError("Username '" + username + "' already exists.");
        return;
    }

    std::string password = readPassword("  Password: ");
    if (password.size() < 6) {
        UI::printError("Password must be at least 6 characters.");
        return;
    }

    std::cout << "  Roles: Admin, Employee, Student, Visitor\n";
    std::string roleStr = readInput("  Role: ");
    auto role = stringToRole(roleStr);
    if (!role.has_value()) {
        UI::printError("Invalid role. Choose: Admin, Employee, Student, or Visitor.");
        return;
    }

    // Hash password
    std::string salt = Auth::generateSalt();
    std::string hash = Auth::hashPassword(password, salt);

    int userId = m_dbManager.addUser(fullName, username, hash, salt, role.value());
    if (userId > 0) {
        UI::printSuccess("User '" + fullName + "' added successfully with ID: " +
                          std::to_string(userId));
    } else {
        UI::printError("Failed to add user.");
    }
}

void AdminPanel::updateUser() {
    std::cout << "\n" << UI::BOLD << "  ── Update User ──" << UI::RESET << "\n\n";

    int userId = readInt("  User ID to update: ");
    auto user = m_dbManager.findUserById(userId);
    if (!user.has_value()) {
        UI::printError("User not found.");
        return;
    }

    std::cout << "  Current Name: " << user.value().fullName << "\n";
    std::cout << "  Current Role: " << roleToString(user.value().role) << "\n\n";

    std::string newName = readInput("  New Full Name (Enter to keep): ");
    if (newName.empty()) newName = user.value().fullName;

    std::string roleStr = readInput("  New Role (Enter to keep): ");
    Role newRole = user.value().role;
    if (!roleStr.empty()) {
        auto r = stringToRole(roleStr);
        if (!r.has_value()) {
            UI::printError("Invalid role.");
            return;
        }
        newRole = r.value();
    }

    if (m_dbManager.updateUser(userId, newName, newRole)) {
        UI::printSuccess("User updated successfully.");
    } else {
        UI::printError("Failed to update user.");
    }
}

void AdminPanel::deactivateUser() {
    int userId = readInt("\n  User ID to deactivate: ");
    if (m_dbManager.updateUserStatus(userId, AccessStatus::Inactive)) {
        UI::printSuccess("User " + std::to_string(userId) + " deactivated.");
    } else {
        UI::printError("Failed to deactivate user. Check if User ID exists.");
    }
}

void AdminPanel::activateUser() {
    int userId = readInt("\n  User ID to activate: ");
    if (m_dbManager.updateUserStatus(userId, AccessStatus::Active)) {
        UI::printSuccess("User " + std::to_string(userId) + " activated.");
    } else {
        UI::printError("Failed to activate user. Check if User ID exists.");
    }
}

void AdminPanel::removeUser() {
    int userId = readInt("\n  User ID to remove: ");

    auto user = m_dbManager.findUserById(userId);
    if (!user.has_value()) {
        UI::printError("User not found.");
        return;
    }

    std::cout << UI::YELLOW << "  ⚠ WARNING: This will permanently delete user '"
              << user.value().fullName << "' (ID: " << userId << ")\n" << UI::RESET;
    std::string confirm = readInput("  Type 'DELETE' to confirm: ");

    if (confirm != "DELETE") {
        UI::printInfo("Deletion cancelled.");
        return;
    }

    if (m_dbManager.removeUser(userId)) {
        UI::printSuccess("User removed successfully.");
    } else {
        UI::printError("Failed to remove user.");
    }
}

void AdminPanel::resetUserPassword() {
    std::cout << "\n" << UI::BOLD << "  ── Reset User Password ──" << UI::RESET << "\n\n";

    int userId = readInt("  User ID: ");
    auto user = m_dbManager.findUserById(userId);
    if (!user.has_value()) {
        UI::printError("User not found.");
        return;
    }

    std::cout << "  User: " << user.value().fullName << " (" << user.value().username << ")\n";

    std::string newPassword = readPassword("  New Password: ");
    if (newPassword.size() < 6) {
        UI::printError("Password must be at least 6 characters.");
        return;
    }

    std::string confirmPassword = readPassword("  Confirm Password: ");
    if (newPassword != confirmPassword) {
        UI::printError("Passwords do not match.");
        return;
    }

    std::string salt = Auth::generateSalt();
    std::string hash = Auth::hashPassword(newPassword, salt);

    if (m_dbManager.updateUserCredential(userId, hash, salt)) {
        UI::printSuccess("Password reset successfully for user " +
                          user.value().fullName + ".");
    } else {
        UI::printError("Failed to reset password.");
    }
}

// ─── Log Management ──────────────────────────────────────────────────────────

void AdminPanel::viewRecentLogs() {
    auto logs = m_logger.getRecentLogs(50);
    Logger::printLogTable(logs);
    UI::pressEnterToContinue();
}

void AdminPanel::searchLogsByUser() {
    int userId = readInt("\n  Enter User ID to search: ");
    auto logs = m_logger.getLogsByUser(userId);
    Logger::printLogTable(logs);
    UI::pressEnterToContinue();
}

void AdminPanel::searchLogsByDate() {
    std::cout << "\n" << UI::BOLD << "  ── Search Logs by Date Range ──"
              << UI::RESET << "\n\n";

    std::string startDate = readInput("  Start Date (YYYY-MM-DD): ");
    std::string endDate   = readInput("  End Date   (YYYY-MM-DD): ");

    auto logs = m_logger.searchLogs(startDate, endDate);
    Logger::printLogTable(logs);
    UI::pressEnterToContinue();
}

// ─── User Table Display ──────────────────────────────────────────────────────

void AdminPanel::printUserTable(const std::vector<User>& users) {
    if (users.empty()) {
        std::cout << UI::YELLOW << "\n  No users found.\n" << UI::RESET;
        return;
    }

    std::cout << "\n";
    std::cout << UI::BOLD << UI::CYAN;
    std::cout << "  ┌────────┬──────────────────┬────────────┬──────────┬──────────┬─────────────────────┐\n";
    std::cout << "  │ UserID │    Full Name     │  Username  │   Role   │  Status  │     Created At      │\n";
    std::cout << "  ├────────┼──────────────────┼────────────┼──────────┼──────────┼─────────────────────┤\n";
    std::cout << UI::RESET;

    for (const auto& user : users) {
        std::ostringstream oss;
        oss << "  │ " << std::setw(6) << user.userId << " │ ";

        // Truncate long names
        std::string name = user.fullName;
        if (name.size() > 16) name = name.substr(0, 13) + "...";
        oss << std::setw(16) << name << " │ ";

        std::string uname = user.username;
        if (uname.size() > 10) uname = uname.substr(0, 7) + "...";
        oss << std::setw(10) << uname << " │ ";

        oss << std::setw(8) << roleToString(user.role) << " │ ";

        if (user.accessStatus == AccessStatus::Active) {
            oss << UI::GREEN << std::setw(8) << "Active" << UI::RESET;
        } else {
            oss << UI::RED << std::setw(8) << "Inactive" << UI::RESET;
        }
        oss << " │ ";
        oss << std::setw(19) << user.createdAt.substr(0, 19) << " │";

        std::cout << oss.str() << "\n";
    }

    std::cout << UI::CYAN;
    std::cout << "  └────────┴──────────────────┴────────────┴──────────┴──────────┴─────────────────────┘\n";
    std::cout << UI::RESET;
    std::cout << UI::DIM << "  Total: " << users.size() << " user(s)\n" << UI::RESET;
}

// ─── Input Utilities ─────────────────────────────────────────────────────────

std::string AdminPanel::readInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int AdminPanel::readInt(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    try {
        return std::stoi(input);
    } catch (...) {
        return -1;
    }
}

std::string AdminPanel::readPassword(const std::string& prompt) {
    std::cout << prompt;

    // Disable terminal echo for password input
    struct termios oldAttr, newAttr;
    tcgetattr(STDIN_FILENO, &oldAttr);
    newAttr = oldAttr;
    newAttr.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newAttr);

    std::string password;
    std::getline(std::cin, password);

    // Restore terminal echo
    tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
    std::cout << "\n";

    return password;
}

} // namespace DoorAccess
