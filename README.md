# 🏢 Automated Door Access Control System

> Embedded Linux-Based Secure Door Access Control using C++17, SQLite, and modular architecture.

## Overview

A software-based Automated Door Access Control System that verifies a person's authorization against an SQLite database before granting or denying entry. Designed for offices, colleges, laboratories, data centers, and other restricted areas.

The system simulates a real-world electronic door lock and is architected to support future Raspberry Pi GPIO integration without requiring a software redesign.

## Features

- **User Authentication** — Login via User ID or Username/Password
- **Secure Credentials** — PBKDF2-HMAC-SHA256 password hashing with random salts (100,000 iterations)
- **Role-Based Access** — Admin, Employee, Student, and Visitor roles
- **Access Logging** — Every access attempt recorded with timestamp and result
- **Admin Panel** — Full user CRUD, password reset, log search, account activation/deactivation
- **SQL Injection Prevention** — All queries use prepared statements exclusively
- **Door Simulation** — Timed unlock with auto-lock, GPIO-ready architecture
- **Graceful Shutdown** — Signal handling (SIGINT/SIGTERM) for safe database closure
- **Hidden Password Input** — Terminal echo disabled during password entry

## Technology Stack

| Component         | Technology             |
|-------------------|------------------------|
| Language          | C++17                  |
| Database          | SQLite3                |
| Password Hashing  | OpenSSL (PBKDF2-SHA256)|
| Build System      | CMake 3.16+            |
| Target OS         | Linux / macOS          |

## Project Structure

```
Ruman/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── .gitignore                  # Git ignore rules
├── include/                    # Header files
│   ├── types.hpp               # Core data structures & enums
│   ├── database_manager.hpp    # Database module interface
│   ├── auth_module.hpp         # Authentication module
│   ├── access_controller.hpp   # Access control logic
│   ├── door_controller.hpp     # Door lock simulation
│   ├── logger.hpp              # Access logging module
│   ├── admin_panel.hpp         # Admin management interface
│   └── ui_utils.hpp            # Terminal display utilities
├── src/                        # Source files
│   ├── main.cpp                # Application entry point
│   ├── database_manager.cpp    # Database operations
│   ├── auth_module.cpp         # Password hashing & verification
│   ├── access_controller.cpp   # Access decision logic
│   ├── door_controller.cpp     # Door lock simulation
│   ├── logger.cpp              # Log recording & display
│   ├── admin_panel.cpp         # Admin panel implementation
│   └── ui_utils.cpp            # UI formatting & colors
└── sql/                        # SQL files
    ├── schema.sql              # Database schema
    └── seed_data.sql           # Sample data documentation
```

## System Architecture

```
┌─────────────────────────────────────────────────────┐
│                    Main Application                  │
│                     (main.cpp)                       │
├──────────┬──────────┬───────────┬───────────────────┤
│  Access  │   Door   │  Logger   │   Admin Panel     │
│Controller│Controller│           │                   │
├──────────┴──────────┴───────────┴───────────────────┤
│              Authentication Module                   │
│           (PBKDF2-HMAC-SHA256 + Salt)                │
├─────────────────────────────────────────────────────┤
│              Database Manager (SQLite3)               │
│           (Prepared Statements Only)                  │
├─────────────────────────────────────────────────────┤
│              SQLite Database (door_access.db)         │
│         ┌──────────┐    ┌─────────────┐             │
│         │  users   │    │ access_logs │             │
│         └──────────┘    └─────────────┘             │
└─────────────────────────────────────────────────────┘
```

## Prerequisites

- **C++17 compiler** (GCC 7+, Clang 5+, or later)
- **CMake** 3.16 or later
- **SQLite3** development libraries
- **OpenSSL** development libraries

### Install Dependencies

**Ubuntu / Debian / Raspberry Pi OS:**
```bash
sudo apt update
sudo apt install build-essential cmake libsqlite3-dev libssl-dev
```

**macOS (Homebrew):**
```bash
brew install cmake sqlite3 openssl
```

## Build & Run

```bash
# Clone the repository
git clone <repository-url>
cd Ruman

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run
./door_access
```

You can also specify a custom database path:
```bash
./door_access /path/to/custom_database.db
```

## Default Seed Users

| Username | Password      | Role     | Status   |
|----------|---------------|----------|----------|
| admin    | admin123      | Admin    | Active   |
| ruman    | password123   | Employee | Active   |
| priya    | password123   | Employee | Active   |
| rahul    | password123   | Student  | Active   |
| sneha    | password123   | Student  | Inactive |
| john     | visitor123    | Visitor  | Active   |

## Usage Examples

### Access by User ID
```
==================================================
  MAIN MENU
==================================================
  Select option: 1
  Enter User ID: 1
  Enter Password: ********

  Verifying credentials...
  Checking SQL Database...

  ╔══════════════════════════════════════╗
  ║         ✅ ACCESS GRANTED ✅         ║
  ╚══════════════════════════════════════╝
  Welcome, Ankit Raj!

  Door Status: UNLOCKED
  Access Time: 22:01:59
```

### Access Denied (Inactive Account)
```
  Enter Username: sneha
  Enter Password: ********

  ╔══════════════════════════════════════╗
  ║         ❌ ACCESS DENIED ❌          ║
  ╚══════════════════════════════════════╝
  Reason: Account Inactive

  Door Status: LOCKED
```

### Admin Panel
Access the admin panel (option 3) to:
- List, add, update, and remove users
- Activate/deactivate accounts
- Reset user passwords
- View and search access logs by User ID or date range

## Security

- **Password Hashing**: PBKDF2-HMAC-SHA256 with 100,000 iterations and per-user random salt
- **SQL Injection Prevention**: All database queries use prepared statements (never string concatenation)
- **Credential Masking**: Passwords are never stored in plaintext; log entries mask credential references
- **Hidden Input**: Password input disables terminal echo
- **Constant-Time Comparison**: Hash verification uses XOR comparison to prevent timing attacks
- **Fail-Secure**: Access is denied if the database connection fails

## Database Schema

### Users Table
| Column          | Type    | Constraints                              |
|-----------------|---------|------------------------------------------|
| user_id         | INTEGER | PRIMARY KEY AUTOINCREMENT                |
| full_name       | TEXT    | NOT NULL                                 |
| username        | TEXT    | NOT NULL UNIQUE                          |
| credential_hash | TEXT    | NOT NULL                                 |
| salt            | TEXT    | NOT NULL                                 |
| role            | TEXT    | CHECK IN (Admin, Employee, Student, Visitor) |
| access_status   | TEXT    | CHECK IN (Active, Inactive)              |
| created_at      | TEXT    | DEFAULT datetime('now', 'localtime')     |

### Access Logs Table
| Column               | Type    | Constraints                       |
|----------------------|---------|-----------------------------------|
| log_id               | INTEGER | PRIMARY KEY AUTOINCREMENT         |
| user_id              | INTEGER | FK → users(user_id) ON DELETE SET NULL |
| attempted_credential | TEXT    | NOT NULL                          |
| access_time          | TEXT    | DEFAULT datetime('now', 'localtime') |
| access_result        | TEXT    | CHECK IN (Granted, Denied)        |
| denial_reason        | TEXT    | Nullable                          |

## Future Enhancements

- [ ] RFID-based access control (MFRC522 reader)
- [ ] Facial recognition integration (OpenCV)
- [ ] Fingerprint authentication
- [ ] Raspberry Pi GPIO integration with relay-controlled electronic lock
- [ ] Web-based administrator dashboard
- [ ] Email/SMS alerts for suspicious access attempts
- [ ] Multiple-door management
- [ ] Remote monitoring and reporting

## License

This project is developed for educational and demonstration purposes.

## Author

**Ankit Raj** — Embedded Linux & C++ Developer
