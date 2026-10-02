/**
 * @file main.cpp
 * @brief Automated Door Access Control System — Main Application
 *
 * Entry point for the Embedded Linux-based door access control system.
 * Provides a terminal-based interface for:
 *   - User access via User ID or Username/Password
 *   - Admin management panel
 *   - Graceful shutdown and error handling
 *
 * @author Ankit Raj
 * @version 1.0.0
 */

#include "database_manager.hpp"
#include "access_controller.hpp"
#include "door_controller.hpp"
#include "logger.hpp"
#include "admin_panel.hpp"
#include "auth_module.hpp"
#include "ui_utils.hpp"

#include <iostream>
#include <string>
#include <csignal>
#include <limits>
#include <termios.h>
#include <unistd.h>

// ─── Global State for Signal Handling ─────────────────────────────────────────

static volatile sig_atomic_t g_running = 1;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_running = 0;
        std::cout << "\n\n";
        std::cout << DoorAccess::UI::YELLOW
                  << "  ⚠ Shutdown signal received. Closing safely..."
                  << DoorAccess::UI::RESET << "\n";
    }
}

// ─── Helper: Read Password with Hidden Input ──────────────────────────────────

static std::string readPassword(const std::string& prompt) {
    std::cout << prompt;

    struct termios oldAttr, newAttr;
    tcgetattr(STDIN_FILENO, &oldAttr);
    newAttr = oldAttr;
    newAttr.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newAttr);

    std::string password;
    std::getline(std::cin, password);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
    std::cout << "\n";

    return password;
}

// ─── Access by User ID ────────────────────────────────────────────────────────

static void accessById(DoorAccess::AccessController& controller,
                        DoorAccess::DoorController& door,
                        DoorAccess::Logger& logger) {
    std::cout << "  Enter User ID: ";
    std::string idStr;
    std::getline(std::cin, idStr);

    int userId;
    try {
        userId = std::stoi(idStr);
    } catch (...) {
        DoorAccess::UI::printError("Invalid User ID format.");
        return;
    }

    std::string password = readPassword("  Enter Password: ");

    std::cout << "\n";
    std::cout << DoorAccess::UI::DIM << "  Verifying credentials..."
              << DoorAccess::UI::RESET << "\n";
    std::cout << DoorAccess::UI::DIM << "  Checking SQL Database..."
              << DoorAccess::UI::RESET << "\n";

    auto decision = controller.attemptAccessById(userId, password);
    std::string accessTime = DoorAccess::UI::getCurrentTimeStr();

    if (decision.result == DoorAccess::AccessResult::Granted) {
        DoorAccess::UI::printAccessGranted(decision.user.value().fullName, accessTime);
        logger.logAccess(decision, "ID:" + idStr);
        door.unlock();
    } else {
        DoorAccess::UI::printAccessDenied(decision.denialReason);
        logger.logAccess(decision, "ID:" + idStr);
    }
}

// ─── Access by Username ───────────────────────────────────────────────────────

static void accessByUsername(DoorAccess::AccessController& controller,
                              DoorAccess::DoorController& door,
                              DoorAccess::Logger& logger) {
    std::cout << "  Enter Username: ";
    std::string username;
    std::getline(std::cin, username);

    if (username.empty()) {
        DoorAccess::UI::printError("Username cannot be empty.");
        return;
    }

    std::string password = readPassword("  Enter Password: ");

    std::cout << "\n";
    std::cout << DoorAccess::UI::DIM << "  Verifying credentials..."
              << DoorAccess::UI::RESET << "\n";
    std::cout << DoorAccess::UI::DIM << "  Checking SQL Database..."
              << DoorAccess::UI::RESET << "\n";

    auto decision = controller.attemptAccessByUsername(username, password);
    std::string accessTime = DoorAccess::UI::getCurrentTimeStr();

    if (decision.result == DoorAccess::AccessResult::Granted) {
        DoorAccess::UI::printAccessGranted(decision.user.value().fullName, accessTime);
        logger.logAccess(decision, "user:" + username);
        door.unlock();
    } else {
        DoorAccess::UI::printAccessDenied(decision.denialReason);
        logger.logAccess(decision, "user:" + username);
    }
}

// ─── Main Menu ────────────────────────────────────────────────────────────────

static void showMainMenu() {
    std::cout << DoorAccess::UI::BOLD << DoorAccess::UI::CYAN;
    std::cout << "  ┌──────────────────────────────────────┐\n";
    std::cout << "  │           MAIN MENU                  │\n";
    std::cout << "  ├──────────────────────────────────────┤\n";
    std::cout << DoorAccess::UI::RESET;
    std::cout << "  │  " << DoorAccess::UI::BOLD << "1"
              << DoorAccess::UI::RESET << ". Access by User ID              │\n";
    std::cout << "  │  " << DoorAccess::UI::BOLD << "2"
              << DoorAccess::UI::RESET << ". Access by Username/Password    │\n";
    std::cout << "  │  " << DoorAccess::UI::BOLD << "3"
              << DoorAccess::UI::RESET << ". Admin Panel                    │\n";
    std::cout << "  │  " << DoorAccess::UI::BOLD << "0"
              << DoorAccess::UI::RESET << ". Exit System                    │\n";
    std::cout << DoorAccess::UI::CYAN;
    std::cout << "  └──────────────────────────────────────┘\n";
    std::cout << DoorAccess::UI::RESET;
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    // Set up signal handlers for graceful shutdown
    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Database path (configurable via command-line argument)
    std::string dbPath = "door_access.db";
    if (argc > 1) {
        dbPath = argv[1];
    }

    // ─── Initialize System Components ─────────────────────────────────────

    DoorAccess::UI::clearScreen();
    DoorAccess::UI::printBanner();

    std::cout << DoorAccess::UI::DIM
              << "  Initializing system...\n" << DoorAccess::UI::RESET;

    // Initialize Database
    DoorAccess::DatabaseManager dbManager(dbPath);
    if (!dbManager.initialize()) {
        DoorAccess::UI::printError("Failed to initialize database at: " + dbPath);
        DoorAccess::UI::printError("Please check file permissions and SQLite installation.");
        return 1;
    }
    DoorAccess::UI::printSuccess("Database initialized: " + dbPath);

    // Initialize Modules
    DoorAccess::AccessController controller(dbManager);
    DoorAccess::DoorController   door(3);  // 3-second unlock duration
    DoorAccess::Logger           logger(dbManager);
    DoorAccess::AdminPanel       admin(dbManager, logger);

    DoorAccess::UI::printSuccess("All modules initialized successfully.");
    std::cout << DoorAccess::UI::DIM
              << "  System ready. Current time: "
              << DoorAccess::UI::getCurrentDateTimeStr()
              << DoorAccess::UI::RESET << "\n";

    // ─── Main Application Loop ────────────────────────────────────────────

    while (g_running) {
        std::cout << "\n";
        DoorAccess::UI::printSeparator('=', 48);
        showMainMenu();

        std::cout << "  Select option: ";
        std::string choiceStr;
        std::getline(std::cin, choiceStr);

        if (std::cin.eof() || !g_running) {
            break;
        }

        int choice;
        try {
            choice = std::stoi(choiceStr);
        } catch (...) {
            DoorAccess::UI::printWarning("Invalid input. Please enter a number.");
            continue;
        }

        switch (choice) {
            case 1:
                DoorAccess::UI::printSeparator('-', 48);
                accessById(controller, door, logger);
                DoorAccess::UI::printSeparator('-', 48);
                break;

            case 2:
                DoorAccess::UI::printSeparator('-', 48);
                accessByUsername(controller, door, logger);
                DoorAccess::UI::printSeparator('-', 48);
                break;

            case 3:
                admin.run();
                DoorAccess::UI::clearScreen();
                DoorAccess::UI::printBanner();
                break;

            case 0:
                g_running = 0;
                break;

            default:
                DoorAccess::UI::printWarning("Invalid option. Please try again.");
                break;
        }
    }

    // ─── Graceful Shutdown ────────────────────────────────────────────────

    std::cout << "\n";
    DoorAccess::UI::printSeparator('=', 48);
    std::cout << DoorAccess::UI::DIM
              << "  Closing database connection...\n" << DoorAccess::UI::RESET;
    dbManager.close();

    std::cout << DoorAccess::UI::GREEN << DoorAccess::UI::BOLD;
    std::cout << "\n  🔒 Door Access Control System shut down safely.\n";
    std::cout << DoorAccess::UI::RESET;
    std::cout << DoorAccess::UI::DIM
              << "  Goodbye!\n\n" << DoorAccess::UI::RESET;

    return 0;
}
