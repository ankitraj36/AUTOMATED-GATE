#pragma once

#include <string>

/**
 * @brief UI Utilities — Terminal display helpers for the Door Access System.
 *
 * Provides consistent, attractive terminal output formatting
 * including banners, status displays, and color codes.
 */

namespace DoorAccess {
namespace UI {

    // ─── ANSI Color Codes ─────────────────────────────────────────────────

    constexpr const char* RESET      = "\033[0m";
    constexpr const char* BOLD       = "\033[1m";
    constexpr const char* DIM        = "\033[2m";
    constexpr const char* UNDERLINE  = "\033[4m";

    constexpr const char* RED        = "\033[31m";
    constexpr const char* GREEN      = "\033[32m";
    constexpr const char* YELLOW     = "\033[33m";
    constexpr const char* BLUE       = "\033[34m";
    constexpr const char* MAGENTA    = "\033[35m";
    constexpr const char* CYAN       = "\033[36m";
    constexpr const char* WHITE      = "\033[37m";

    constexpr const char* BG_RED     = "\033[41m";
    constexpr const char* BG_GREEN   = "\033[42m";
    constexpr const char* BG_YELLOW  = "\033[43m";
    constexpr const char* BG_BLUE    = "\033[44m";

    // ─── Display Functions ────────────────────────────────────────────────

    /**
     * @brief Print the main application banner.
     */
    void printBanner();

    /**
     * @brief Print a separator line.
     */
    void printSeparator(char ch = '=', int width = 50);

    /**
     * @brief Print an "Access Granted" message.
     * @param userName Name of the authorized user.
     * @param accessTime Formatted access time string.
     */
    void printAccessGranted(const std::string& userName, const std::string& accessTime);

    /**
     * @brief Print an "Access Denied" message.
     * @param reason Reason for denial.
     */
    void printAccessDenied(const std::string& reason);

    /**
     * @brief Print door status.
     * @param isUnlocked Current door state.
     */
    void printDoorStatus(bool isUnlocked);

    /**
     * @brief Print an error message.
     */
    void printError(const std::string& message);

    /**
     * @brief Print a success message.
     */
    void printSuccess(const std::string& message);

    /**
     * @brief Print a warning message.
     */
    void printWarning(const std::string& message);

    /**
     * @brief Print an info message.
     */
    void printInfo(const std::string& message);

    /**
     * @brief Get the current time as a formatted string.
     * @return Time string in "HH:MM:SS" format.
     */
    std::string getCurrentTimeStr();

    /**
     * @brief Get the current date-time as a formatted string.
     * @return DateTime string in "YYYY-MM-DD HH:MM:SS" format.
     */
    std::string getCurrentDateTimeStr();

    /**
     * @brief Clear the terminal screen.
     */
    void clearScreen();

    /**
     * @brief Pause and wait for the user to press Enter.
     */
    void pressEnterToContinue();

} // namespace UI
} // namespace DoorAccess
