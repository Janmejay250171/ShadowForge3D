#include "Sculpture.h"
#include <iostream>

void Sculpture::buildLevel(int levelNumber) {
    m_mesh.destroy();
    m_currentLevel = levelNumber;

    switch (levelNumber) {
        case 1: {
            // =================================================================
            // Level 1: "The Monolith" (Introduction)
            // =================================================================
            // Central trunk
            m_mesh.addBox(glm::vec3(0.0f, -0.05f, 0.0f), glm::vec3(0.36f, 1.10f, 0.36f), glm::vec3(0.24f, 0.58f, 0.68f));
            // Prominent horizontal arm (+X)
            m_mesh.addBox(glm::vec3(0.42f, 0.15f, 0.0f), glm::vec3(0.56f, 0.28f, 0.30f), glm::vec3(0.92f, 0.68f, 0.20f));
            // Top crown pyramid (+Y)
            m_mesh.addPyramid(glm::vec3(0.0f, 0.50f, 0.0f), glm::vec2(0.36f, 0.36f), 0.44f, glm::vec3(0.68f, 0.32f, 0.82f));
            // Offset block (-X, +Z)
            m_mesh.addBox(glm::vec3(-0.24f, -0.32f, 0.22f), glm::vec3(0.26f, 0.28f, 0.26f), glm::vec3(0.90f, 0.32f, 0.32f));
            // Upper rear accent (-X, -Z)
            m_mesh.addBox(glm::vec3(-0.22f, 0.30f, -0.12f), glm::vec3(0.20f, 0.22f, 0.22f), glm::vec3(0.22f, 0.76f, 0.54f));
            break;
        }

        case 2: {
            // =================================================================
            // Level 2: "The Cross-Blade" (Pitch Rotation)
            // =================================================================
            // Main vertical blade
            m_mesh.addBox(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.32f, 1.25f, 0.32f), glm::vec3(0.25f, 0.45f, 0.85f));
            // High arm extending right (+X, +Y)
            m_mesh.addBox(glm::vec3(0.40f, 0.32f, 0.0f), glm::vec3(0.52f, 0.24f, 0.26f), glm::vec3(0.95f, 0.55f, 0.25f));
            // Low arm extending left (-X, -Y)
            m_mesh.addBox(glm::vec3(-0.35f, -0.25f, 0.0f), glm::vec3(0.42f, 0.24f, 0.26f), glm::vec3(0.20f, 0.75f, 0.50f));
            // Forward base anchor (+Z, -Y)
            m_mesh.addBox(glm::vec3(0.05f, -0.48f, 0.25f), glm::vec3(0.28f, 0.26f, 0.30f), glm::vec3(0.85f, 0.25f, 0.30f));
            // Top pointed blade pyramid (+Y)
            m_mesh.addPyramid(glm::vec3(0.0f, 0.625f, 0.0f), glm::vec2(0.32f, 0.32f), 0.40f, glm::vec3(0.85f, 0.75f, 0.20f));
            break;
        }

        case 3: {
            // =================================================================
            // Level 3: "The Totem" (Complexity: Pitch & Yaw)
            // =================================================================
            // Central staggered totem pillar
            m_mesh.addBox(glm::vec3(0.0f, -0.05f, 0.0f), glm::vec3(0.36f, 1.15f, 0.36f), glm::vec3(0.20f, 0.60f, 0.65f));
            // Upper broad wing (+X)
            m_mesh.addBox(glm::vec3(0.42f, 0.25f, 0.0f), glm::vec3(0.55f, 0.30f, 0.28f), glm::vec3(0.92f, 0.65f, 0.20f));
            // Forward stepped block (-X, +Z)
            m_mesh.addBox(glm::vec3(-0.25f, -0.15f, 0.25f), glm::vec3(0.30f, 0.30f, 0.30f), glm::vec3(0.88f, 0.30f, 0.35f));
            // Lower rear stabilizing fin (+X, -Z)
            m_mesh.addBox(glm::vec3(0.18f, -0.38f, -0.24f), glm::vec3(0.25f, 0.35f, 0.28f), glm::vec3(0.35f, 0.30f, 0.75f));
            // Crown apex pyramid
            m_mesh.addPyramid(glm::vec3(0.0f, 0.525f, 0.0f), glm::vec2(0.36f, 0.36f), 0.42f, glm::vec3(0.70f, 0.35f, 0.85f));
            break;
        }

        case 4: {
            // =================================================================
            // Level 4: "The Construct" (Precision: Yaw & Roll)
            // =================================================================
            // Main vertical structure
            m_mesh.addBox(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.34f, 1.15f, 0.34f), glm::vec3(0.28f, 0.40f, 0.70f));
            // High cantilever beam (+X, +Y, +Z)
            m_mesh.addBox(glm::vec3(0.42f, 0.32f, 0.12f), glm::vec3(0.58f, 0.26f, 0.28f), glm::vec3(0.95f, 0.72f, 0.18f));
            // Low offset stepped foundation (-X, -Y, -Z)
            m_mesh.addBox(glm::vec3(-0.32f, -0.35f, -0.12f), glm::vec3(0.42f, 0.32f, 0.30f), glm::vec3(0.88f, 0.42f, 0.20f));
            // Mid forward wedge (+X, -Y, +Z)
            m_mesh.addBox(glm::vec3(0.12f, -0.10f, 0.26f), glm::vec3(0.28f, 0.28f, 0.30f), glm::vec3(0.22f, 0.80f, 0.55f));
            // Asymmetric apex pyramid tilted offset
            m_mesh.addPyramid(glm::vec3(-0.06f, 0.575f, 0.0f), glm::vec2(0.34f, 0.34f), 0.40f, glm::vec3(0.65f, 0.30f, 0.80f));
            break;
        }

        case 5:
        default: {
            // =================================================================
            // Level 5: "The Shadow Forge" (Final Forge: 3-Axis Master)
            // =================================================================
            // Central monument trunk
            m_mesh.addBox(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.38f, 1.18f, 0.38f), glm::vec3(0.22f, 0.35f, 0.60f));
            // Primary high arm (+X, +Y, -Z)
            m_mesh.addBox(glm::vec3(0.44f, 0.22f, -0.12f), glm::vec3(0.56f, 0.28f, 0.30f), glm::vec3(0.95f, 0.65f, 0.15f));
            // Secondary low counter-arm (-X, -Y, +Z)
            m_mesh.addBox(glm::vec3(-0.40f, -0.22f, 0.14f), glm::vec3(0.48f, 0.26f, 0.28f), glm::vec3(0.90f, 0.28f, 0.32f));
            // Forward vertical spine (+X, -Y, +Z)
            m_mesh.addBox(glm::vec3(0.16f, -0.16f, 0.30f), glm::vec3(0.26f, 0.44f, 0.28f), glm::vec3(0.18f, 0.78f, 0.52f));
            // Rear high shoulder (-X, +Y, -Z)
            m_mesh.addBox(glm::vec3(-0.20f, 0.26f, -0.26f), glm::vec3(0.25f, 0.30f, 0.26f), glm::vec3(0.20f, 0.70f, 0.85f));
            // Regal crown apex pyramid
            m_mesh.addPyramid(glm::vec3(0.0f, 0.59f, 0.0f), glm::vec2(0.38f, 0.38f), 0.46f, glm::vec3(0.75f, 0.28f, 0.88f));
            break;
        }
    }

    // Upload geometry to GPU memory
    m_mesh.upload();

    std::cout << "[Sculpture] Level " << levelNumber << " generated ("
              << m_mesh.getVertexCount() << " vertices, "
              << m_mesh.getTriangleCount() << " triangles)." << std::endl;
}

void Sculpture::draw() const {
    m_mesh.draw();
}

void Sculpture::destroy() {
    m_mesh.destroy();
}
