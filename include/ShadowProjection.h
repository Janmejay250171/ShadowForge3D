#ifndef SHADOWFORGE_SHADOW_PROJECTION_H
#define SHADOWFORGE_SHADOW_PROJECTION_H

#include <vector>
#include "GLCommon.h"
#include <glm/glm.hpp>
#include "Matrix4.h"
#include "Mesh.h"

/**
 * ShadowProjection Module:
 * Projects 3D sculpture geometry onto a 2D viewing plane in Normalized Device Coordinates (NDC).
 * 
 * Mathematical Transformation Pipeline:
 *   local vertex (x, y, z, 1.0)
 *       ↓
 *   Model matrix (Custom Matrix4)
 *       ↓
 *   View matrix (glm::lookAt)
 *       ↓
 *   Projection matrix (glm::perspective)
 *       ↓
 *   Clip coordinates (x_clip, y_clip, z_clip, w_clip)
 *       ↓
 *   Perspective divide by w (x_clip/w, y_clip/w, z_clip/w)
 *       ↓
 *   Normalized Device Coordinates (NDC) in [-1, 1]
 */
class ShadowProjection {
private:
    // Dynamic live silhouette GPU buffers & vertex cache
    GLuint m_liveFillVao = 0;
    GLuint m_liveFillVbo = 0;
    GLuint m_liveLineVao = 0;
    GLuint m_liveLineVbo = 0;
    GLuint m_liveDebugLineVao = 0;
    GLuint m_liveDebugLineVbo = 0;
    size_t m_liveFillVertexCount = 0;
    size_t m_liveLineVertexCount = 0;
    size_t m_liveDebugLineVertexCount = 0;
    std::vector<Vertex> m_liveFillVertices;

    // Fixed target silhouette GPU buffers & vertex cache
    GLuint m_targetFillVao = 0;
    GLuint m_targetFillVbo = 0;
    GLuint m_targetLineVao = 0;
    GLuint m_targetLineVbo = 0;
    GLuint m_targetDebugLineVao = 0;
    GLuint m_targetDebugLineVbo = 0;
    size_t m_targetFillVertexCount = 0;
    size_t m_targetLineVertexCount = 0;
    size_t m_targetDebugLineVertexCount = 0;
    std::vector<Vertex> m_targetFillVertices;

    // 2D Projection canvas backdrop (frame border / reference marks)
    GLuint m_canvasVao = 0;
    GLuint m_canvasVbo = 0;
    size_t m_canvasVertexCount = 0;

    bool m_isInitialized = false;

public:
    ShadowProjection() = default;
    ~ShadowProjection();

    // Prevent copying of OpenGL handles
    ShadowProjection(const ShadowProjection&) = delete;
    ShadowProjection& operator=(const ShadowProjection&) = delete;

    // Initialize canvas backdrop and precompute the fixed target silhouette
    void init(const std::vector<Vertex>& modelVertices,
              const Matrix4& targetModelMatrix,
              const glm::mat4& viewMatrix,
              const glm::mat4& projMatrix);

    // Compute live 2D projected geometry from the current 3D orientation
    void updateLiveProjection(const std::vector<Vertex>& modelVertices,
                             const Matrix4& currentModelMatrix,
                             const glm::mat4& viewMatrix,
                             const glm::mat4& projMatrix,
                             bool isMatched = false);

    // Render the 2D projection screen: backdrop, target silhouette, and live silhouette
    void render(GLint modelLoc, GLint viewLoc, GLint projLoc, GLint alphaLoc = -1,
                bool showTarget = false, bool debugMode = false, bool isVictory = false);
    void render(GLint modelLoc, GLint viewLoc, GLint projLoc, bool debugMode) {
        render(modelLoc, viewLoc, projLoc, -1, false, debugMode, false);
    }

    // Accessors for 2D projected geometry to feed into ShadowMatcher
    const std::vector<Vertex>& getTargetVertices() const { return m_targetFillVertices; }
    const std::vector<Vertex>& getLiveVertices() const { return m_liveFillVertices; }

    // Release all allocated OpenGL buffer objects
    void destroy();

    // Helper: Project 3D geometry into 2D NDC triangles, clean boundary lines, and debug lines
    static void projectGeometry(const std::vector<Vertex>& inVertices,
                               const Matrix4& modelMatrix,
                               const glm::mat4& viewMatrix,
                               const glm::mat4& projMatrix,
                               const glm::vec3& fillColor,
                               const glm::vec3& lineColor,
                               const glm::vec3& debugLineColor,
                               std::vector<Vertex>& outFillVertices,
                               std::vector<Vertex>& outCleanLineVertices,
                               std::vector<Vertex>& outDebugLineVertices);

    // Overload for backward-compatibility with tests
    static void projectGeometry(const std::vector<Vertex>& inVertices,
                               const Matrix4& modelMatrix,
                               const glm::mat4& viewMatrix,
                               const glm::mat4& projMatrix,
                               const glm::vec3& fillColor,
                               const glm::vec3& lineColor,
                               std::vector<Vertex>& outFillVertices,
                               std::vector<Vertex>& outLineVertices) {
        std::vector<Vertex> dummyDebug;
        projectGeometry(inVertices, modelMatrix, viewMatrix, projMatrix,
                        fillColor, lineColor, lineColor,
                        outFillVertices, outLineVertices, dummyDebug);
    }
};

#endif // SHADOWFORGE_SHADOW_PROJECTION_H
