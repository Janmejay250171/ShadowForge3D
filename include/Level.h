#ifndef SHADOWFORGE_LEVEL_H
#define SHADOWFORGE_LEVEL_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Matrix4.h"

/**
 * Data specification for a ShadowForge puzzle level.
 */
struct LevelInfo {
    int number;
    std::string name;
    std::string subtitle;
    float targetRotX;
    float targetRotY;
    float targetRotZ;
    float threshold = 95.0f;

    // Constructs the exact target transformation matrix using non-locking YZX Euler composition
    Matrix4 getTargetMatrix() const {
        Matrix4 rx = Matrix4::rotationX(glm::radians(targetRotX));
        Matrix4 ry = Matrix4::rotationY(glm::radians(targetRotY));
        Matrix4 rz = Matrix4::rotationZ(glm::radians(targetRotZ));
        return ry * rz * rx;
    }
};

/**
 * LevelManager:
 * Manages the 5-level game progression, target orientations, and level switching.
 */
class LevelManager {
private:
    std::vector<LevelInfo> m_levels;
    int m_currentIndex = 0; // 0-based index [0..4]

public:
    LevelManager();

    int getCurrentLevelNumber() const { return m_currentIndex + 1; }
    int getTotalLevels() const { return static_cast<int>(m_levels.size()); }
    const LevelInfo& getCurrentLevel() const { return m_levels[m_currentIndex]; }

    bool hasNextLevel() const { return (m_currentIndex + 1) < static_cast<int>(m_levels.size()); }
    bool advanceLevel();

    void resetToFirstLevel();
    void setLevelIndex(int index);

    const std::vector<LevelInfo>& getAllLevels() const { return m_levels; }
};

#endif // SHADOWFORGE_LEVEL_H
