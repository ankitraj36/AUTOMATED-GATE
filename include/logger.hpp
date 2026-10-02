#pragma once

#include "types.hpp"
#include "database_manager.hpp"
#include <string>
#include <vector>

/**
 * @brief Logging Module — Records and retrieves access events.
 *
 * Provides structured logging of all access attempts with appropriate
 * credential masking. Supports log retrieval with filtering.
 */

namespace DoorAccess {

class Logger {
public:
    /**
     * @brief Construct a Logger.
     * @param dbManager Reference to the active DatabaseManager.
     */
    explicit Logger(DatabaseManager& dbManager);

    /**
     * @brief Log an access attempt.
     * @param decision The access decision to log.
     * @param credential The credential used (will be masked).
     * @return true if logged successfully.
     */
    bool logAccess(const AccessDecision& decision, const std::string& credential);

    /**
     * @brief Get recent access logs.
     * @param limit Number of logs to retrieve (0 = all).
     */
    std::vector<AccessLog> getRecentLogs(int limit = 50);

    /**
     * @brief Get access logs for a specific user.
     */
    std::vector<AccessLog> getLogsByUser(int userId);

    /**
     * @brief Search logs by date range.
     * @param startDate YYYY-MM-DD format.
     * @param endDate YYYY-MM-DD format.
     */
    std::vector<AccessLog> searchLogs(const std::string& startDate,
                                       const std::string& endDate);

    /**
     * @brief Print a formatted access log entry to the terminal.
     */
    static void printLogEntry(const AccessLog& log);

    /**
     * @brief Print a formatted table of access logs.
     */
    static void printLogTable(const std::vector<AccessLog>& logs);

private:
    DatabaseManager& m_dbManager;
};

} // namespace DoorAccess
