#include "ui_utils.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace DoorAccess {
namespace UI {

// ─── Banner ───────────────────────────────────────────────────────────────────

void printBanner() {
    std::cout << "\n";
    std::cout << BOLD << CYAN;
    std::cout << "  ╔════════════════════════════════════════════════╗\n";
    std::cout << "  ║                                                ║\n";
    std::cout << "  ║     🏢 AUTOMATED DOOR ACCESS SYSTEM 🏢       ║\n";
    std::cout << "  ║        Embedded Linux · C++ · SQLite           ║\n";
    std::cout << "  ║                                                ║\n";
    std::cout << "  ╚════════════════════════════════════════════════╝\n";
    std::cout << RESET;
    std::cout << "\n";
}

// ─── Separator ────────────────────────────────────────────────────────────────

void printSeparator(char ch, int width) {
    std::cout << "  ";
    for (int i = 0; i < width; ++i) {
        std::cout << ch;
    }
    std::cout << "\n";
}

// ─── Access Granted ───────────────────────────────────────────────────────────

void printAccessGranted(const std::string& userName, const std::string& accessTime) {
    std::cout << "\n";
    std::cout << BOLD << GREEN;
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║         ✅ ACCESS GRANTED ✅         ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n";
    std::cout << RESET;
    std::cout << GREEN << "  Welcome, " << BOLD << userName << RESET
              << GREEN << "!" << RESET << "\n";
    std::cout << "\n";
    std::cout << "  Door Status: " << GREEN << BOLD << "UNLOCKED" << RESET << "\n";
    std::cout << "  Access Time: " << accessTime << "\n";
}

// ─── Access Denied ────────────────────────────────────────────────────────────

void printAccessDenied(const std::string& reason) {
    std::cout << "\n";
    std::cout << BOLD << RED;
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║         ❌ ACCESS DENIED ❌          ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n";
    std::cout << RESET;
    std::cout << RED << "  Reason: " << reason << RESET << "\n";
    std::cout << "\n";
    std::cout << "  Door Status: " << RED << BOLD << "LOCKED" << RESET << "\n";
}

// ─── Door Status ──────────────────────────────────────────────────────────────

void printDoorStatus(bool isUnlocked) {
    if (isUnlocked) {
        std::cout << "  Door Status: " << GREEN << BOLD << "UNLOCKED" << RESET << "\n";
    } else {
        std::cout << "  Door Status: " << RED << BOLD << "LOCKED" << RESET << "\n";
    }
}

// ─── Messages ─────────────────────────────────────────────────────────────────

void printError(const std::string& message) {
    std::cout << RED << BOLD << "  ✗ Error: " << RESET << RED << message
              << RESET << "\n";
}

void printSuccess(const std::string& message) {
    std::cout << GREEN << "  ✓ " << message << RESET << "\n";
}

void printWarning(const std::string& message) {
    std::cout << YELLOW << "  ⚠ " << message << RESET << "\n";
}

void printInfo(const std::string& message) {
    std::cout << CYAN << "  ℹ " << message << RESET << "\n";
}

// ─── Time Utilities ───────────────────────────────────────────────────────────

std::string getCurrentTimeStr() {
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm, "%H:%M:%S");
    return oss.str();
}

std::string getCurrentDateTimeStr() {
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// ─── Screen Utilities ─────────────────────────────────────────────────────────

void clearScreen() {
    // ANSI escape code to clear screen and move cursor to top-left
    std::cout << "\033[2J\033[H";
}

void pressEnterToContinue() {
    std::cout << "\n" << DIM << "  Press Enter to continue..." << RESET;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace UI
} // namespace DoorAccess
