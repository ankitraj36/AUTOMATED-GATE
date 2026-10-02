-- Seed Data for Door Access Control System
-- Passwords are pre-hashed (SHA-256 with salt) for demonstration.
-- Default password for all seed users: "password123"
--
-- NOTE: The application will hash passwords on user creation.
--       These seed records are inserted by the application's
--       DatabaseManager::initializeSeedData() method, NOT by running
--       this SQL file directly, because the hashes depend on the
--       runtime salt generation.
--
-- This file serves as documentation of the intended seed data.

-- Admin User
--   user_id: auto
--   full_name: Ankit Raj
--   username: admin
--   password: admin123
--   role: Admin
--   access_status: Active

-- Employee Users
--   full_name: Ruman Kumar
--   username: ruman
--   password: password123
--   role: Employee
--   access_status: Active

--   full_name: Priya Sharma
--   username: priya
--   password: password123
--   role: Employee
--   access_status: Active

-- Student Users
--   full_name: Rahul Verma
--   username: rahul
--   password: password123
--   role: Student
--   access_status: Active

--   full_name: Sneha Gupta
--   username: sneha
--   password: password123
--   role: Student
--   access_status: Inactive (for testing denied access)

-- Visitor
--   full_name: John Doe
--   username: john
--   password: visitor123
--   role: Visitor
--   access_status: Active
