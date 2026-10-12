#ifndef SHADOWFORGE_HUD_H
#define SHADOWFORGE_HUD_H

#include <string>
#include <vector>
#include "GLCommon.h"
#include <glm/glm.hpp>
#include "Mesh.h"

/**
 * HUD Module:
 * Minimal futuristic geometric laboratory interface rendered entirely
 * in pure OpenGL 3.3 Core via procedural vector geometry (lines and quads).
 * 
 * Features:
 * - 0 external textures or font libraries required
 * - Crisp, resolution-independent vector stroke font
 * - Clean technical viewports (3D sculpture on Left, Target Silhouette on Right)
 * - Prominent MATCH percentage and precision threshold progress meter
 * - State overlays for MENU, SUCCESS, and COMPLETE
 */
class HUD {
private:
    std::vector<Vertex> m_lineVertices;
    std::vector<Vertex> m_triVertices;

    GLuint m_lineVao = 0;
    GLuint m_lineVbo = 0;
    GLuint m_triVao = 0;
    GLuint m_triVbo = 0;

    bool m_isInitialized = false;

public:
    HUD();
    ~HUD();

    HUD(const HUD&) = delete;
    HUD& operator=(const HUD&) = delete;

    void init();
    void destroy();

    // Clear buffered UI primitives before building frame
    void beginFrame();

    // -------------------------------------------------------------
    // Vector Primitives in NDC coordinates [-1, 1]
    // -------------------------------------------------------------
    void drawLine(float x0, float y0, float x1, float y1, const glm::vec3& color);
    void drawRect(float x0, float y0, float x1, float y1, const glm::vec3& color);
    void drawRectOutline(float x0, float y0, float x1, float y1, const glm::vec3& color);
    void drawCornerTicks(float x0, float y0, float x1, float y1, float tickSize, const glm::vec3& color);

    // -------------------------------------------------------------
    // Vector Stroke Typography
    // -------------------------------------------------------------
    void drawChar(char c, float x, float y, float sizeX, float sizeY, const glm::vec3& color);
    void drawText(const std::string& text, float x, float y, float sizeX, float sizeY, float spacingX, const glm::vec3& color);
    void drawTextCentered(const std::string& text, float centerX, float y, float sizeX, float sizeY, float spacingX, const glm::vec3& color);
    void drawTextRight(const std::string& text, float rightX, float y, float sizeX, float sizeY, float spacingX, const glm::vec3& color);
    float getTextWidth(const std::string& text, float sizeX, float spacingX) const;

    // -------------------------------------------------------------
    // Laboratory Component Layouts
    // -------------------------------------------------------------
    // Separator line and framing between Left (3D sculpture view) and Right (Projection / Target view)
    void renderLayoutFrames(bool showTarget, float curX = 0.0f, float curY = 0.0f, float curZ = 0.0f,
                            float tgtX = 0.0f, float tgtY = 0.0f, float tgtZ = 0.0f);


    // Subtle bottom controls
    void renderBottomControls(bool debugMode = false, bool manipulationMode = false, bool canSnap = false);

    // Manipulation Mode on-screen axis control strip and live angles
    void renderManipulationOverlay(float rotX, float rotY, float rotZ, bool isDragging = false);

    // Top HUD: Title, Level info, prominent Match % and Target threshold
    void renderTopHUD(int levelNumber, int totalLevels, const std::string& levelName,
                      const std::string& levelSubtitle, float matchPercentage, float threshold,
                      bool debugMode = false);

    // Dynamic Progress Bar underneath Match percentage
    void renderProgressBar(float x0, float y0, float x1, float y1,
                           float currentVal, float targetVal, bool isSuccess);

    // Animated full-screen boot loading screen overlay
    void renderLoadingScreen(float animTime, float loadProgress);

    // Menu state overlay
    void renderMenuOverlay(float animTime);

    // Success card celebration overlay
    void renderSuccessCard(int levelNumber, float matchScore, float animTime);

    // Complete state victory overlay
    void renderCompleteCard(float animTime);

    // Flush and render all buffered UI elements
    void render(GLint modelLoc, GLint viewLoc, GLint projLoc, GLint renderModeLoc, GLint alphaLoc = -1);
};

#endif // SHADOWFORGE_HUD_H
