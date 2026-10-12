#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <cassert>

#include "GLCommon.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Matrix4.h"
#include "Sculpture.h"
#include "ShadowProjection.h"
#include "ShadowMatcher.h"
#include "Level.h"

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Audit", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create headless window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to load GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glm::vec3 cameraPos(0.0f, 1.2f, 3.2f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    float aspect = 640.0f / 720.0f;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    LevelManager levelManager;
    Sculpture sculpture;
    ShadowProjection shadowProj;
    ShadowMatcher shadowMatcher;

    std::cout << "========================================" << std::endl;
    std::cout << "SHADOWFORGE LEVEL REACHABILITY AUDIT" << std::endl;
    std::cout << "========================================" << std::endl;

    bool allPassed = true;

    for (int i = 0; i < levelManager.getTotalLevels(); ++i) {
        levelManager.setLevelIndex(i);
        const LevelInfo& lvl = levelManager.getCurrentLevel();

        sculpture.buildLevel(lvl.number);
        Matrix4 targetMatrix = lvl.getTargetMatrix();
        shadowProj.init(sculpture.getVertices(), targetMatrix, view, projection);
        shadowMatcher.init(shadowProj.getTargetVertices());

        // Simulated player control starting from 0, 0, 0
        float curX = 0.0f;
        float curY = 0.0f;
        float curZ = 0.0f;

        // Player applies discrete rotations (simulating key presses: A/D for X, W/S for Y, Q/E for Z)
        // Step size: 1.0 degree increments
        const float STEP = 1.0f;

        // Rotate X toward target
        while (std::abs(curX - lvl.targetRotX) >= 0.5f * STEP) {
            if (curX < lvl.targetRotX) curX += STEP;
            else curX -= STEP;
        }
        // Rotate Y toward target
        while (std::abs(curY - lvl.targetRotY) >= 0.5f * STEP) {
            if (curY < lvl.targetRotY) curY += STEP;
            else curY -= STEP;
        }
        // Rotate Z toward target
        while (std::abs(curZ - lvl.targetRotZ) >= 0.5f * STEP) {
            if (curZ < lvl.targetRotZ) curZ += STEP;
            else curZ -= STEP;
        }

        // Construct current model matrix exactly as in game
        Matrix4 rotX = Matrix4::rotationX(glm::radians(curX));
        Matrix4 rotY = Matrix4::rotationY(glm::radians(curY));
        Matrix4 rotZ = Matrix4::rotationZ(glm::radians(curZ));
        Matrix4 customModel = rotY * rotZ * rotX;

        // Project geometry through the real projection pipeline
        shadowProj.updateLiveProjection(sculpture.getVertices(), customModel, view, projection, false);

        // Evaluate similarity using real ShadowMatcher
        float match = shadowMatcher.evaluateSimilarity(shadowProj.getLiveVertices());
        bool success = (match >= lvl.threshold && shadowMatcher.isSuccess());

        std::cout << "\n--------------------------------" << std::endl;
        std::cout << "LEVEL " << lvl.number << " (" << lvl.name << ": " << lvl.subtitle << ")" << std::endl;
        std::cout << "Target:       " << static_cast<int>(lvl.targetRotX) << ", "
                  << static_cast<int>(lvl.targetRotY) << ", "
                  << static_cast<int>(lvl.targetRotZ) << std::endl;
        std::cout << "Current:      " << static_cast<int>(curX) << ", "
                  << static_cast<int>(curY) << ", "
                  << static_cast<int>(curZ) << std::endl;
        std::cout << "Current occ:  " << shadowMatcher.getLastLiveOccupancyCount() << std::endl;
        std::cout << "Target occ:   " << shadowMatcher.getTargetOccupancyCount() << std::endl;
        std::cout << "Intersection: " << shadowMatcher.getLastIntersectionCount() << std::endl;
        std::cout << "Union:        " << shadowMatcher.getLastUnionCount() << std::endl;
        std::cout << "Match:        " << std::fixed << std::setprecision(1) << match << "%" << std::endl;
        std::cout << "PASS/FAIL:    " << (success ? "PASS" : "FAIL") << std::endl;
        std::cout << "--------------------------------" << std::endl;

        // Developer Key 'F' Snap verification:
        // Snap from arbitrary orientation directly to target and test score
        float testX = lvl.targetRotX;
        float testY = lvl.targetRotY;
        float testZ = lvl.targetRotZ;
        Matrix4 mF = Matrix4::rotationY(glm::radians(testY)) *
                     Matrix4::rotationZ(glm::radians(testZ)) *
                     Matrix4::rotationX(glm::radians(testX));
        shadowProj.updateLiveProjection(sculpture.getVertices(), mF, view, projection, false);
        float matchF = shadowMatcher.evaluateSimilarity(shadowProj.getLiveVertices());
        std::cout << "   [Key 'F' Snap Test]: MATCH = "
                  << std::fixed << std::setprecision(1) << matchF << "% ["
                  << (matchF >= lvl.threshold ? "PASS" : "FAIL") << "]" << std::endl;

        if (!success || match < lvl.threshold || matchF < lvl.threshold) {
            allPassed = false;
        }

        shadowMatcher.destroy();
        shadowProj.destroy();
        sculpture.destroy();
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "SHADOWFORGE FINAL LEVEL VALIDATION" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "ALL LEVELS SOLVABLE & VALIDATED: " << (allPassed ? "YES" : "NO") << std::endl;
    std::cout << "========================================" << std::endl;

    glfwDestroyWindow(window);
    glfwTerminate();

    return allPassed ? 0 : 1;
}

