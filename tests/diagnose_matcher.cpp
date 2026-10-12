#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "GLCommon.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Matrix4.h"
#include "Sculpture.h"
#include "ShadowProjection.h"
#include "ShadowMatcher.h"
#include "Level.h"

int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Diag", nullptr, nullptr);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) return 1;

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

    auto testAngle = [&](int lvlNum, float rx, float ry, float rz) {
        levelManager.setLevelIndex(lvlNum - 1);
        const LevelInfo& lvl = levelManager.getCurrentLevel();
        sculpture.buildLevel(lvl.number);
        Matrix4 targetMatrix = lvl.getTargetMatrix();
        shadowProj.init(sculpture.getVertices(), targetMatrix, view, projection);
        shadowMatcher.init(shadowProj.getTargetVertices());
        std::cout << "DEBUG: Level " << lvlNum << " target Fill Vertices: " << shadowProj.getTargetVertices().size() << std::endl;


        Matrix4 m = Matrix4::rotationY(glm::radians(ry)) *
                    Matrix4::rotationZ(glm::radians(rz)) *
                    Matrix4::rotationX(glm::radians(rx));
        shadowProj.updateLiveProjection(sculpture.getVertices(), m, view, projection, false);
        float score = shadowMatcher.evaluateSimilarity(shadowProj.getLiveVertices());
        std::cout << "L" << lvlNum << " (" << rx << ", " << ry << ", " << rz << "): "
                  << "Match=" << score << "% (Live=" << shadowMatcher.getLastLiveOccupancyCount()
                  << ", Tgt=" << shadowMatcher.getTargetOccupancyCount()
                  << ", Inter=" << shadowMatcher.getLastIntersectionCount()
                  << ", Union=" << shadowMatcher.getLastUnionCount() << ")" << std::endl;
        shadowMatcher.destroy();
        shadowProj.destroy();
        sculpture.destroy();
        return score;
    };

    std::cout << "\n============================================================" << std::endl;
    std::cout << "TESTING ALL 5 LEVELS AT EXACT TARGET & NEIGHBORING ANGLES" << std::endl;
    std::cout << "============================================================" << std::endl;

    for (int lvl = 1; lvl <= 5; ++lvl) {
        levelManager.setLevelIndex(lvl - 1);
        const LevelInfo& l = levelManager.getCurrentLevel();
        std::cout << "\n>>> LEVEL " << lvl << ": " << l.name << " (" << l.subtitle << ") <<<" << std::endl;
        std::cout << "Target: (" << l.targetRotX << ", " << l.targetRotY << ", " << l.targetRotZ << ")" << std::endl;

        // Exact target
        testAngle(lvl, l.targetRotX, l.targetRotY, l.targetRotZ);

        // +/- 1 degree on each active axis
        testAngle(lvl, l.targetRotX + 1, l.targetRotY, l.targetRotZ);
        testAngle(lvl, l.targetRotX - 1, l.targetRotY, l.targetRotZ);
        testAngle(lvl, l.targetRotX, l.targetRotY + 1, l.targetRotZ);
        testAngle(lvl, l.targetRotX, l.targetRotY - 1, l.targetRotZ);
        testAngle(lvl, l.targetRotX, l.targetRotY, l.targetRotZ + 1);
        testAngle(lvl, l.targetRotX, l.targetRotY, l.targetRotZ - 1);
    }



    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
