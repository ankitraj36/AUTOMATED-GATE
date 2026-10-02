#pragma once

#include <string>

/**
 * @brief Door Control Module — Simulates door lock/unlock mechanism.
 *
 * Provides a software-based simulation of an electronic door lock.
 * Designed to be extended with GPIO-based hardware control in the future
 * (e.g., Raspberry Pi GPIO pins controlling a relay/solenoid lock).
 */

namespace DoorAccess {

class DoorController {
public:
    /**
     * @brief Construct a DoorController.
     * @param unlockDurationSec How long the door remains unlocked (seconds).
     */
    explicit DoorController(int unlockDurationSec = 5);

    /**
     * @brief Unlock the door (simulation).
     * Displays door status and simulates the unlock duration.
     */
    void unlock();

    /**
     * @brief Lock the door (simulation).
     */
    void lock();

    /**
     * @brief Get the current door status.
     * @return "LOCKED" or "UNLOCKED".
     */
    std::string getStatus() const;

    /**
     * @brief Check if the door is currently unlocked.
     */
    bool isUnlocked() const;

    /**
     * @brief Set the unlock duration.
     * @param seconds Duration in seconds.
     */
    void setUnlockDuration(int seconds);

    /**
     * @brief Get the configured unlock duration.
     */
    int getUnlockDuration() const;

private:
    bool m_isUnlocked = false;
    int  m_unlockDurationSec;

    /**
     * @brief GPIO pin control placeholder for future hardware integration.
     * @param pin GPIO pin number.
     * @param state HIGH or LOW.
     */
    void gpioWrite(int pin, bool state);
};

} // namespace DoorAccess
