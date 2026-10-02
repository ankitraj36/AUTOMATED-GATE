#include "logger.hpp"
#include "auth_module.hpp"
#include "ui_utils.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace DoorAccess {

// ─── Constructor ──────────────────────────────────────────────────────────────

Logger::Logger(DatabaseManager& dbManager)
    : m_dbManager(dbManager) {}

// ─── Log Access ───────────────────────────────────────────────────────────────

bool Logger::logAccess(const AccessDecision& decision, const std::string& credential) {
    int userId = 0;
    if (decision.user.has_value()) {
        userId = decision.user.value().userId;
    }

    std::string maskedCredential = Auth::maskCredential(credential);

    bool success = m_dbManager.logAccessAttempt(
        userId,
        maskedCredential,
        decision.result,
        decision.denialReason
    );

    if (success) {
        if (decision.result == AccessResult::Granted) {
            std::cout << UI::DIM << "  ✓ Access log saved successfully."
                      << UI::RESET << "\n";
        } else {
            std::cout << UI::DIM << "  ✓ Access attempt logged."
                      << UI::RESET << "\n";
        }
    } else {
        std::cout << UI::RED << "  ✗ Warning: Failed to save access log."
                  << UI::RESET << "\n";
    }

    return success;
}

// ─── Retrieve Logs ────────────────────────────────────────────────────────────

std::vector<AccessLog> Logger::getRecentLogs(int limit) {
    return m_dbManager.getAccessLogs(limit);
}

std::vector<AccessLog> Logger::getLogsByUser(int userId) {
    return m_dbManager.getAccessLogsByUserId(userId);
}

std::vector<AccessLog> Logger::searchLogs(const std::string& startDate,
                                            const std::string& endDate) {
    return m_dbManager.searchAccessLogs(startDate, endDate);
}

// ─── Log Display ──────────────────────────────────────────────────────────────

void Logger::printLogEntry(const AccessLog& log) {
    std::cout << UI::DIM << "  ├─ ";
    std::cout << UI::RESET << "Log #" << log.logId;
    std::cout << " | User ID: " << (log.userId > 0 ? std::to_string(log.userId) : "N/A");
    std::cout << " | Credential: " << log.attemptedCredential;
    std::cout << " | Time: " << log.accessTime;

    if (log.accessResult == AccessResult::Granted) {
        std::cout << " | " << UI::GREEN << "GRANTED" << UI::RESET;
    } else {
        std::cout << " | " << UI::RED << "DENIED" << UI::RESET;
        if (!log.denialReason.empty()) {
            std::cout << " (" << log.denialReason << ")";
        }
    }
    std::cout << "\n";
}

void Logger::printLogTable(const std::vector<AccessLog>& logs) {
    if (logs.empty()) {
        std::cout << UI::YELLOW << "\n  No access logs found.\n" << UI::RESET;
        return;
    }

    // Table header
    std::cout << "\n";
    std::cout << UI::BOLD << UI::CYAN;
    std::cout << "  ┌────────┬─────────┬────────────────┬─────────────────────┬──────────┬──────────────────────┐\n";
    std::cout << "  │ Log ID │ User ID │   Credential   │     Access Time     │  Result  │    Denial Reason     │\n";
    std::cout << "  ├────────┼─────────┼────────────────┼─────────────────────┼──────────┼──────────────────────┤\n";
    std::cout << UI::RESET;

    for (const auto& log : logs) {
        std::ostringstream oss;
        oss << "  │ " << std::setw(6) << log.logId << " │ ";
        oss << std::setw(7) << (log.userId > 0 ? std::to_string(log.userId) : "N/A") << " │ ";
        oss << std::setw(14) << log.attemptedCredential.substr(0, 14) << " │ ";
        oss << std::setw(19) << log.accessTime.substr(0, 19) << " │ ";

        if (log.accessResult == AccessResult::Granted) {
            oss << UI::GREEN << std::setw(8) << "Granted" << UI::RESET << " │ ";
        } else {
            oss << UI::RED << std::setw(8) << "Denied" << UI::RESET << " │ ";
        }

        std::string reason = log.denialReason.empty() ? "—" : log.denialReason;
        if (reason.size() > 20) reason = reason.substr(0, 17) + "...";
        oss << std::setw(20) << reason << " │";

        std::cout << oss.str() << "\n";
    }

    std::cout << UI::CYAN;
    std::cout << "  └────────┴─────────┴────────────────┴─────────────────────┴──────────┴──────────────────────┘\n";
    std::cout << UI::RESET;
    std::cout << UI::DIM << "  Total: " << logs.size() << " log(s)\n" << UI::RESET;
}

} // namespace DoorAccess
