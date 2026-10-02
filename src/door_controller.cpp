#include "door_controller.hpp"
#include "ui_utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace DoorAccess {

// ─── Constructor ──────────────────────────────────────────────────────────────

DoorController::DoorController(int unlockDurationSec)
    : m_isUnlocked(false), m_unlockDurationSec(unlockDurationSec) {}

// ─── Unlock ───────────────────────────────────────────────────────────────────

void DoorController::unlock() {
    m_isUnlocked = true;
    gpioWrite(17, true);  // GPIO pin 17 — relay ON (simulation)

    std::cout << "\n";
    std::cout << UI::BOLD << UI::GREEN;
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║        🔓 DOOR UNLOCKED 🔓          ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n";
    std::cout << UI::RESET;

    std::cout << UI::DIM << "  Door will auto-lock in "
              << m_unlockDurationSec << " seconds..." << UI::RESET << "\n";

    // Simulate door open duration
    std::this_thread::sleep_for(std::chrono::seconds(m_unlockDurationSec));

    lock();
}

// ─── Lock ─────────────────────────────────────────────────────────────────────

void DoorController::lock() {
    m_isUnlocked = false;
    gpioWrite(17, false);  // GPIO pin 17 — relay OFF (simulation)

    std::cout << UI::DIM << "  🔒 Door auto-locked." << UI::RESET << "\n";
}

// ─── Status ───────────────────────────────────────────────────────────────────

std::string DoorController::getStatus() const {
    return m_isUnlocked ? "UNLOCKED" : "LOCKED";
}

bool DoorController::isUnlocked() const {
    return m_isUnlocked;
}

// ─── Configuration ────────────────────────────────────────────────────────────

void DoorController::setUnlockDuration(int seconds) {
    if (seconds > 0) {
        m_unlockDurationSec = seconds;
    }
}

int DoorController::getUnlockDuration() const {
    return m_unlockDurationSec;
}

// ─── GPIO Simulation ─────────────────────────────────────────────────────────

void DoorController::gpioWrite([[maybe_unused]] int pin,
                                [[maybe_unused]] bool state) {
    // SIMULATION MODE:
    // In a real Raspberry Pi deployment, this function would:
    //
    //   #include <wiringPi.h>
    //   wiringPiSetupGpio();
    //   pinMode(pin, OUTPUT);
    //   digitalWrite(pin, state ? HIGH : LOW);
    //
    // For now, this is a no-op placeholder that documents the
    // intended hardware interface for future GPIO integration.
}

} // namespace DoorAccess
