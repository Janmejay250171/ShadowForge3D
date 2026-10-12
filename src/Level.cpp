#include "Level.h"
#include <algorithm>

LevelManager::LevelManager() {
    // Level 1: Introduction - Teaches the core Y-axis silhouette change
    m_levels.push_back({
        1,
        "INTRODUCTION",
        "The Monolith",
        0.0f, 90.0f, 0.0f,
        95.0f
    });

    // Level 2: Rotation - Teaches deliberate pitch (X-axis) silhouette change
    m_levels.push_back({
        2,
        "ROTATION",
        "The Cross-Blade",
        90.0f, 0.0f, 0.0f,
        95.0f
    });

    // Level 3: Complexity - Multi-axial rotation combining Pitch & Yaw (X & Y)
    m_levels.push_back({
        3,
        "COMPLEXITY",
        "The Totem",
        45.0f, 90.0f, 0.0f,
        95.0f
    });

    // Level 4: Precision - Precise combination of Yaw & Roll (Y & Z)
    m_levels.push_back({
        4,
        "PRECISION",
        "The Construct",
        0.0f, 90.0f, 45.0f,
        95.0f
    });

    // Level 5: Final Forge - Master 3-axis combination (X, Y, Z)
    m_levels.push_back({
        5,
        "FINAL FORGE",
        "The Shadow Forge",
        45.0f, 90.0f, 45.0f,
        95.0f
    });

    m_currentIndex = 0;
}

bool LevelManager::advanceLevel() {
    if (hasNextLevel()) {
        m_currentIndex++;
        return true;
    }
    return false;
}

void LevelManager::resetToFirstLevel() {
    m_currentIndex = 0;
}

void LevelManager::setLevelIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_levels.size())) {
        m_currentIndex = index;
    }
}
