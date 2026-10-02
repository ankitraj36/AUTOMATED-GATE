-- Door Access Control System Database Schema
-- SQLite3 Database: door_access.db

-- Enable foreign keys
PRAGMA foreign_keys = ON;

-- Users Table
CREATE TABLE IF NOT EXISTS users (
    user_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    full_name     TEXT    NOT NULL,
    username      TEXT    NOT NULL UNIQUE,
    credential_hash TEXT  NOT NULL,
    salt          TEXT    NOT NULL,
    role          TEXT    NOT NULL CHECK (role IN ('Admin', 'Employee', 'Student', 'Visitor')),
    access_status TEXT    NOT NULL DEFAULT 'Active' CHECK (access_status IN ('Active', 'Inactive')),
    created_at    TEXT    NOT NULL DEFAULT (datetime('now', 'localtime'))
);

-- Access Logs Table
CREATE TABLE IF NOT EXISTS access_logs (
    log_id             INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id            INTEGER,
    attempted_credential TEXT NOT NULL,
    access_time        TEXT    NOT NULL DEFAULT (datetime('now', 'localtime')),
    access_result      TEXT    NOT NULL CHECK (access_result IN ('Granted', 'Denied')),
    denial_reason      TEXT,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL
);

-- Index for faster lookups
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_access_logs_user_id ON access_logs(user_id);
CREATE INDEX IF NOT EXISTS idx_access_logs_time ON access_logs(access_time);
