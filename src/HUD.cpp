#include "HUD.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

HUD::HUD() {
    init();
}

HUD::~HUD() {
    destroy();
}

void HUD::init() {
    if (m_isInitialized) return;

    // Line VAO/VBO
    glGenVertexArrays(1, &m_lineVao);
    glGenBuffers(1, &m_lineVbo);

    glBindVertexArray(m_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Triangle VAO/VBO
    glGenVertexArrays(1, &m_triVao);
    glGenBuffers(1, &m_triVbo);

    glBindVertexArray(m_triVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_triVbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_isInitialized = true;
}

void HUD::destroy() {
    if (m_lineVbo != 0) { glDeleteBuffers(1, &m_lineVbo); m_lineVbo = 0; }
    if (m_lineVao != 0) { glDeleteVertexArrays(1, &m_lineVao); m_lineVao = 0; }
    if (m_triVbo  != 0) { glDeleteBuffers(1, &m_triVbo);  m_triVbo = 0; }
    if (m_triVao  != 0) { glDeleteVertexArrays(1, &m_triVao);  m_triVao = 0; }

    m_lineVertices.clear();
    m_triVertices.clear();
    m_isInitialized = false;
}

void HUD::beginFrame() {
    m_lineVertices.clear();
    m_triVertices.clear();
}

void HUD::drawLine(float x0, float y0, float x1, float y1, const glm::vec3& color) {
    m_lineVertices.push_back(Vertex(glm::vec3(x0, y0, 0.0f), color));
    m_lineVertices.push_back(Vertex(glm::vec3(x1, y1, 0.0f), color));
}

void HUD::drawRect(float x0, float y0, float x1, float y1, const glm::vec3& color) {
    m_triVertices.push_back(Vertex(glm::vec3(x0, y0, 0.0f), color));
    m_triVertices.push_back(Vertex(glm::vec3(x1, y0, 0.0f), color));
    m_triVertices.push_back(Vertex(glm::vec3(x1, y1, 0.0f), color));

    m_triVertices.push_back(Vertex(glm::vec3(x0, y0, 0.0f), color));
    m_triVertices.push_back(Vertex(glm::vec3(x1, y1, 0.0f), color));
    m_triVertices.push_back(Vertex(glm::vec3(x0, y1, 0.0f), color));
}

void HUD::drawRectOutline(float x0, float y0, float x1, float y1, const glm::vec3& color) {
    drawLine(x0, y0, x1, y0, color);
    drawLine(x1, y0, x1, y1, color);
    drawLine(x1, y1, x0, y1, color);
    drawLine(x0, y1, x0, y0, color);
}

void HUD::drawCornerTicks(float x0, float y0, float x1, float y1, float tickSize, const glm::vec3& color) {
    // Bottom-Left
    drawLine(x0, y0, x0 + tickSize, y0, color);
    drawLine(x0, y0, x0, y0 + tickSize, color);

    // Bottom-Right
    drawLine(x1, y0, x1 - tickSize, y0, color);
    drawLine(x1, y0, x1, y0 + tickSize, color);

    // Top-Right
    drawLine(x1, y1, x1 - tickSize, y1, color);
    drawLine(x1, y1, x1, y1 - tickSize, color);

    // Top-Left
    drawLine(x0, y1, x0 + tickSize, y1, color);
    drawLine(x0, y1, x0, y1 - tickSize, color);
}

// -----------------------------------------------------------------------------
// Procedural Vector Typography
// -----------------------------------------------------------------------------
struct StrokeSegment {
    float x0, y0, x1, y1;
};

static const std::vector<StrokeSegment>& getGlyphStrokes(char rawChar) {
    static const std::vector<StrokeSegment> emptyStrokes;
    char c = static_cast<char>(std::toupper(static_cast<unsigned char>(rawChar)));

    // Cache stroke definitions
    static std::vector<StrokeSegment> glyphs[256];
    static bool tableInitialized = false;

    if (!tableInitialized) {
        // Letters
        glyphs['A'] = { {0.0f,0.0f, 0.0f,0.6f}, {0.0f,0.6f, 0.5f,1.0f}, {0.5f,1.0f, 1.0f,0.6f}, {1.0f,0.6f, 1.0f,0.0f}, {0.0f,0.4f, 1.0f,0.4f} };
        glyphs['B'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.7f,1.0f}, {0.7f,1.0f, 0.9f,0.75f}, {0.9f,0.75f, 0.7f,0.5f}, {0.7f,0.5f, 0.0f,0.5f}, {0.7f,0.5f, 0.9f,0.25f}, {0.9f,0.25f, 0.7f,0.0f}, {0.7f,0.0f, 0.0f,0.0f} };
        glyphs['C'] = { {1.0f,1.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.0f,0.0f}, {0.0f,0.0f, 1.0f,0.0f} };
        glyphs['D'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.65f,1.0f}, {0.65f,1.0f, 1.0f,0.65f}, {1.0f,0.65f, 1.0f,0.35f}, {1.0f,0.35f, 0.65f,0.0f}, {0.65f,0.0f, 0.0f,0.0f} };
        glyphs['E'] = { {1.0f,1.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.0f,0.0f}, {0.0f,0.0f, 1.0f,0.0f}, {0.0f,0.5f, 0.7f,0.5f} };
        glyphs['F'] = { {1.0f,1.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.0f,0.0f}, {0.0f,0.5f, 0.7f,0.5f} };
        glyphs['G'] = { {1.0f,1.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.0f,0.0f}, {0.0f,0.0f, 1.0f,0.0f}, {1.0f,0.0f, 1.0f,0.5f}, {1.0f,0.5f, 0.5f,0.5f} };
        glyphs['H'] = { {0.0f,0.0f, 0.0f,1.0f}, {1.0f,0.0f, 1.0f,1.0f}, {0.0f,0.5f, 1.0f,0.5f} };
        glyphs['I'] = { {0.5f,0.0f, 0.5f,1.0f}, {0.2f,1.0f, 0.8f,1.0f}, {0.2f,0.0f, 0.8f,0.0f} };
        glyphs['J'] = { {0.8f,1.0f, 0.8f,0.25f}, {0.8f,0.25f, 0.55f,0.0f}, {0.55f,0.0f, 0.2f,0.0f}, {0.2f,0.0f, 0.2f,0.35f} };
        glyphs['K'] = { {0.0f,0.0f, 0.0f,1.0f}, {1.0f,1.0f, 0.0f,0.5f}, {0.0f,0.5f, 1.0f,0.0f} };
        glyphs['L'] = { {0.0f,1.0f, 0.0f,0.0f}, {0.0f,0.0f, 1.0f,0.0f} };
        glyphs['M'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.5f,0.4f}, {0.5f,0.4f, 1.0f,1.0f}, {1.0f,1.0f, 1.0f,0.0f} };
        glyphs['N'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 1.0f,0.0f}, {1.0f,0.0f, 1.0f,1.0f} };
        glyphs['O'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 1.0f,0.0f}, {1.0f,0.0f, 0.0f,0.0f} };
        glyphs['P'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 1.0f,0.5f}, {1.0f,0.5f, 0.0f,0.5f} };
        glyphs['Q'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 1.0f,0.0f}, {1.0f,0.0f, 0.0f,0.0f}, {0.55f,0.25f, 1.0f,-0.15f} };
        glyphs['R'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 1.0f,0.5f}, {1.0f,0.5f, 0.0f,0.5f}, {0.4f,0.5f, 1.0f,0.0f} };
        glyphs['S'] = { {1.0f,1.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.0f,0.5f}, {0.0f,0.5f, 1.0f,0.5f}, {1.0f,0.5f, 1.0f,0.0f}, {1.0f,0.0f, 0.0f,0.0f} };
        glyphs['T'] = { {0.0f,1.0f, 1.0f,1.0f}, {0.5f,1.0f, 0.5f,0.0f} };
        glyphs['U'] = { {0.0f,1.0f, 0.0f,0.0f}, {0.0f,0.0f, 1.0f,0.0f}, {1.0f,0.0f, 1.0f,1.0f} };
        glyphs['V'] = { {0.0f,1.0f, 0.5f,0.0f}, {0.5f,0.0f, 1.0f,1.0f} };
        glyphs['W'] = { {0.0f,1.0f, 0.2f,0.0f}, {0.2f,0.0f, 0.5f,0.6f}, {0.5f,0.6f, 0.8f,0.0f}, {0.8f,0.0f, 1.0f,1.0f} };
        glyphs['X'] = { {0.0f,0.0f, 1.0f,1.0f}, {0.0f,1.0f, 1.0f,0.0f} };
        glyphs['Y'] = { {0.0f,1.0f, 0.5f,0.5f}, {1.0f,1.0f, 0.5f,0.5f}, {0.5f,0.5f, 0.5f,0.0f} };
        glyphs['Z'] = { {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 0.0f,0.0f}, {0.0f,0.0f, 1.0f,0.0f} };

        // Digits
        glyphs['0'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 1.0f,0.0f}, {1.0f,0.0f, 0.0f,0.0f}, {0.15f,0.2f, 0.85f,0.8f} };
        glyphs['1'] = { {0.2f,0.75f, 0.5f,1.0f}, {0.5f,1.0f, 0.5f,0.0f}, {0.2f,0.0f, 0.8f,0.0f} };
        glyphs['2'] = { {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 1.0f,0.55f}, {1.0f,0.55f, 0.0f,0.0f}, {0.0f,0.0f, 1.0f,0.0f} };
        glyphs['3'] = { {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 0.4f,0.5f}, {0.4f,0.5f, 1.0f,0.5f}, {1.0f,0.5f, 1.0f,0.0f}, {1.0f,0.0f, 0.0f,0.0f} };
        glyphs['4'] = { {0.0f,1.0f, 0.0f,0.45f}, {0.0f,0.45f, 1.0f,0.45f}, {0.8f,1.0f, 0.8f,0.0f} };
        glyphs['5'] = { {1.0f,1.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.0f,0.55f}, {0.0f,0.55f, 1.0f,0.55f}, {1.0f,0.55f, 1.0f,0.0f}, {1.0f,0.0f, 0.0f,0.0f} };
        glyphs['6'] = { {1.0f,1.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.0f,0.0f}, {0.0f,0.0f, 1.0f,0.0f}, {1.0f,0.0f, 1.0f,0.5f}, {1.0f,0.5f, 0.0f,0.5f} };
        glyphs['7'] = { {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 0.35f,0.0f} };
        glyphs['8'] = { {0.0f,0.0f, 0.0f,1.0f}, {0.0f,1.0f, 1.0f,1.0f}, {1.0f,1.0f, 1.0f,0.0f}, {1.0f,0.0f, 0.0f,0.0f}, {0.0f,0.5f, 1.0f,0.5f} };
        glyphs['9'] = { {1.0f,0.0f, 1.0f,1.0f}, {1.0f,1.0f, 0.0f,1.0f}, {0.0f,1.0f, 0.0f,0.5f}, {0.0f,0.5f, 1.0f,0.5f} };

        // Symbols
        glyphs['%'] = { {0.1f,0.0f, 0.9f,1.0f}, {0.2f,0.7f, 0.4f,0.7f}, {0.4f,0.7f, 0.4f,0.9f}, {0.4f,0.9f, 0.2f,0.9f}, {0.2f,0.9f, 0.2f,0.7f},
                                                 {0.6f,0.1f, 0.8f,0.1f}, {0.8f,0.1f, 0.8f,0.3f}, {0.8f,0.3f, 0.6f,0.3f}, {0.6f,0.3f, 0.6f,0.1f} };
        glyphs[':'] = { {0.5f,0.75f, 0.5f,0.85f}, {0.5f,0.25f, 0.5f,0.35f} };
        glyphs['/'] = { {0.1f,0.0f, 0.9f,1.0f} };
        glyphs['-'] = { {0.15f,0.5f, 0.85f,0.5f} };
        glyphs['_'] = { {0.0f,0.0f, 1.0f,0.0f} };
        glyphs['+'] = { {0.5f,0.2f, 0.5f,0.8f}, {0.2f,0.5f, 0.8f,0.5f} };
        glyphs['>'] = { {0.2f,0.85f, 0.8f,0.5f}, {0.8f,0.5f, 0.2f,0.15f} };
        glyphs['<'] = { {0.8f,0.85f, 0.2f,0.5f}, {0.2f,0.5f, 0.8f,0.15f} };
        glyphs['='] = { {0.15f,0.65f, 0.85f,0.65f}, {0.15f,0.35f, 0.85f,0.35f} };
        glyphs['['] = { {0.7f,1.0f, 0.3f,1.0f}, {0.3f,1.0f, 0.3f,0.0f}, {0.3f,0.0f, 0.7f,0.0f} };
        glyphs[']'] = { {0.3f,1.0f, 0.7f,1.0f}, {0.7f,1.0f, 0.7f,0.0f}, {0.7f,0.0f, 0.3f,0.0f} };
        glyphs['('] = { {0.7f,1.0f, 0.35f,0.7f}, {0.35f,0.7f, 0.35f,0.3f}, {0.35f,0.3f, 0.7f,0.0f} };
        glyphs[')'] = { {0.3f,1.0f, 0.65f,0.7f}, {0.65f,0.7f, 0.65f,0.3f}, {0.65f,0.3f, 0.3f,0.0f} };
        glyphs['.'] = { {0.45f,0.0f, 0.55f,0.0f}, {0.55f,0.0f, 0.55f,0.1f}, {0.55f,0.1f, 0.45f,0.1f}, {0.45f,0.1f, 0.45f,0.0f} };
        glyphs[','] = { {0.55f,0.15f, 0.45f,0.0f}, {0.45f,0.0f, 0.35f,-0.15f} };

        // Checkmark character (mapped to '~')
        glyphs['~'] = { {0.1f,0.45f, 0.4f,0.1f}, {0.4f,0.1f, 0.95f,0.95f} };

        // Degree symbol '*' and code 176
        glyphs['*'] = { {0.2f,0.65f, 0.6f,0.65f}, {0.6f,0.65f, 0.6f,0.95f}, {0.6f,0.95f, 0.2f,0.95f}, {0.2f,0.95f, 0.2f,0.65f} };
        glyphs[176] = glyphs['*'];

        // UTF-8 Greater-than-or-equal character '≥' (mapped to char 126)
        glyphs[126] = { {0.15f,0.90f, 0.85f,0.60f}, {0.85f,0.60f, 0.15f,0.30f}, {0.15f,0.12f, 0.85f,0.12f} };

        // UTF-8 Em-dash character '—' (mapped to char 127)
        glyphs[127] = { {0.0f,0.50f, 1.0f,0.50f} };

        tableInitialized = true;
    }

    auto idx = static_cast<unsigned char>(c);
    return glyphs[idx];
}

void HUD::drawChar(char c, float x, float y, float sizeX, float sizeY, const glm::vec3& color) {
    const auto& strokes = getGlyphStrokes(c);
    for (const auto& s : strokes) {
        drawLine(x + s.x0 * sizeX, y + s.y0 * sizeY,
                 x + s.x1 * sizeX, y + s.y1 * sizeY,
                 color);
    }
}

void HUD::drawText(const std::string& text, float x, float y, float sizeX, float sizeY, float spacingX, const glm::vec3& color) {
    float curX = x;
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (static_cast<unsigned char>(c) == 0xE2 && i + 2 < text.size()) {
            unsigned char b1 = static_cast<unsigned char>(text[i+1]);
            unsigned char b2 = static_cast<unsigned char>(text[i+2]);
            if (b1 == 0x9C && b2 == 0x93) {
                // UTF-8 checkmark ✓
                drawChar('~', curX, y, sizeX, sizeY, color);
                curX += sizeX + spacingX;
                i += 2;
                continue;
            }
            if (b1 == 0x80 && b2 == 0x94) {
                // UTF-8 em-dash —
                drawChar(127, curX, y, sizeX, sizeY, color);
                curX += sizeX + spacingX;
                i += 2;
                continue;
            }
            if (b1 == 0x89 && b2 == 0xA5) {
                // UTF-8 greater-than-or-equal ≥
                drawChar(126, curX, y, sizeX, sizeY, color);
                curX += sizeX + spacingX;
                i += 2;
                continue;
            }
        }
        if (c == ' ') {
            curX += sizeX * 0.70f + spacingX;
            continue;
        }
        drawChar(c, curX, y, sizeX, sizeY, color);
        curX += sizeX + spacingX;
    }
}

float HUD::getTextWidth(const std::string& text, float sizeX, float spacingX) const {
    float width = 0.0f;
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (static_cast<unsigned char>(c) == 0xE2 && i + 2 < text.size()) {
            unsigned char b1 = static_cast<unsigned char>(text[i+1]);
            unsigned char b2 = static_cast<unsigned char>(text[i+2]);
            if ((b1 == 0x9C && b2 == 0x93) || (b1 == 0x80 && b2 == 0x94) || (b1 == 0x89 && b2 == 0xA5)) {
                width += sizeX + spacingX;
                i += 2;
                continue;
            }
        }
        if (c == ' ') {
            width += sizeX * 0.70f + spacingX;
            continue;
        }
        width += sizeX + spacingX;
    }
    if (width > spacingX) width -= spacingX;
    return width;
}

void HUD::drawTextCentered(const std::string& text, float centerX, float y, float sizeX, float sizeY, float spacingX, const glm::vec3& color) {
    float width = getTextWidth(text, sizeX, spacingX);
    drawText(text, centerX - width * 0.5f, y, sizeX, sizeY, spacingX, color);
}

void HUD::drawTextRight(const std::string& text, float rightX, float y, float sizeX, float sizeY, float spacingX, const glm::vec3& color) {
    float width = getTextWidth(text, sizeX, spacingX);
    drawText(text, rightX - width, y, sizeX, sizeY, spacingX, color);
}

// -----------------------------------------------------------------------------
// Component Layouts
// -----------------------------------------------------------------------------
void HUD::renderLayoutFrames(bool showTarget, float curX, float curY, float curZ,
                             float tgtX, float tgtY, float tgtZ) {
    glm::vec3 panelBorder(0.18f, 0.20f, 0.25f);
    glm::vec3 labelColor(0.55f, 0.60f, 0.68f);
    glm::vec3 amberColor(0.96f, 0.65f, 0.15f);
    glm::vec3 cyanColor(0.04f, 0.72f, 0.82f);
    glm::vec3 textWhite(0.85f, 0.90f, 0.96f);

    // Left Panel: YOUR SCULPTURE
    // x: [-0.96, -0.015], y: [-0.84, 0.77]
    drawRectOutline(-0.96f, -0.84f, -0.015f, 0.77f, panelBorder);
    drawText("YOUR SCULPTURE", -0.93f, 0.725f, 0.0115f, 0.018f, 0.0028f, labelColor);

    // Format current rotation angles: e.g. "ROT: X 0*  Y 90*  Z 0*"
    int iCurX = static_cast<int>(std::round(std::fmod(curX + 360.0f, 360.0f))) % 360;
    int iCurY = static_cast<int>(std::round(std::fmod(curY + 360.0f, 360.0f))) % 360;
    int iCurZ = static_cast<int>(std::round(std::fmod(curZ + 360.0f, 360.0f))) % 360;
    std::string curRotStr = "ROT: X " + std::to_string(iCurX) + "*  Y " +
                            std::to_string(iCurY) + "*  Z " + std::to_string(iCurZ) + "*";
    drawTextRight(curRotStr, -0.045f, 0.725f, 0.0095f, 0.015f, 0.0022f, cyanColor);

    // Right Panel: PROJECTION & TARGET (MIXED 3D) or TARGET SILHOUETTE (3D LINES ONLY)
    // x: [0.015, 0.96], y: [-0.84, 0.77]
    drawRectOutline(0.015f, -0.84f, 0.96f, 0.77f, panelBorder);

    if (showTarget) {
        drawText("TARGET SILHOUETTE (3D LINES ONLY)", 0.045f, 0.725f, 0.0115f, 0.018f, 0.0028f, amberColor);
        int iTgtX = static_cast<int>(std::round(tgtX)) % 360;
        int iTgtY = static_cast<int>(std::round(tgtY)) % 360;
        int iTgtZ = static_cast<int>(std::round(tgtZ)) % 360;
        std::string tgtStr = "TARGET: X " + std::to_string(iTgtX) + "*  Y " +
                             std::to_string(iTgtY) + "*  Z " + std::to_string(iTgtZ) + "*";
        drawTextRight(tgtStr, 0.93f, 0.725f, 0.0095f, 0.015f, 0.0022f, amberColor);
    } else {
        drawText("PROJECTION & TARGET (MIXED 3D)", 0.045f, 0.725f, 0.0115f, 0.018f, 0.0028f, cyanColor);
        drawTextRight("TARGET (AMBER) + LIVE (CYAN)  [H: TARGET ONLY]", 0.93f, 0.725f, 0.0085f, 0.0135f, 0.002f, labelColor);
    }
}

void HUD::renderBottomControls(bool debugMode, bool manipulationMode, bool canSnap) {
    (void)canSnap;
    glm::vec3 dividerColor(0.16f, 0.18f, 0.22f);
    glm::vec3 textMuted(0.50f, 0.55f, 0.62f);
    glm::vec3 textCyan(0.04f, 0.72f, 0.82f);
    glm::vec3 textAmber(0.96f, 0.62f, 0.15f);

    // Subtle divider line
    drawLine(-0.96f, -0.87f, 0.96f, -0.87f, dividerColor);

    // Centered controls: A/D, W/S, Q/E, [F] SNAP TARGET, [H] TARGET ONLY, M, R
    std::string controlsStr;
    if (manipulationMode) {
        controlsStr = "DRAG MOUSE  ROTATE (SHIFT: Z)    A/D/W/S/Q/E  KEYS    [F]  SNAP TARGET    [H]  TARGET ONLY    M  EXIT MOUSE    R  RESET";
    } else {
        controlsStr = "A/D  X ROT    W/S  Y ROT    Q/E  Z ROT    [F]  SNAP TARGET    [H]  TARGET ONLY    M  MOUSE    R  RESET";
    }
    drawTextCentered(controlsStr, 0.0f, -0.935f, 0.0085f, 0.014f, 0.002f, manipulationMode ? textCyan : textMuted);

    if (debugMode) {
        drawText("[DEBUG ON: 'F3']", 0.78f, -0.935f, 0.0085f, 0.0135f, 0.002f, textAmber);
    }
}


void HUD::renderManipulationOverlay(float rotX, float rotY, float rotZ, bool isDragging) {
    glm::vec3 panelBg(0.06f, 0.07f, 0.09f);
    glm::vec3 panelBorder(0.20f, 0.24f, 0.30f);
    glm::vec3 cyanColor(0.04f, 0.72f, 0.82f);
    glm::vec3 brightCyan(0.20f, 0.85f, 0.95f);
    glm::vec3 amberColor(0.96f, 0.65f, 0.15f);
    glm::vec3 textMuted(0.45f, 0.50f, 0.56f);
    glm::vec3 textWhite(0.85f, 0.90f, 0.96f);
    glm::vec3 trackColor(0.22f, 0.25f, 0.30f);
    glm::vec3 trackActive(0.04f, 0.55f, 0.65f);

    // 1. Background Panel for Manipulation Controls in Left Viewport
    // Positioned below sculpture: x in [-0.94, -0.035], y in [-0.82, -0.53]
    drawRect(-0.94f, -0.82f, -0.035f, -0.53f, panelBg);
    drawRectOutline(-0.94f, -0.82f, -0.035f, -0.53f, isDragging ? brightCyan : panelBorder);

    // 2. Header: MANIPULATION MODE badge and instructions
    drawText("MANIPULATION MODE", -0.92f, -0.57f, 0.009f, 0.014f, 0.0022f, isDragging ? brightCyan : cyanColor);
    drawText(isDragging ? "ACTIVE MOUSE DRAG" : "DRAG 3D MOUSE | SHIFT+DRAG: Z", -0.66f, -0.57f, 0.007f, 0.011f, 0.0018f, textMuted);

    // Reset Button [RESET] at top right of control strip
    drawRect(-0.165f, -0.595f, -0.055f, -0.545f, glm::vec3(0.12f, 0.14f, 0.18f));
    drawRectOutline(-0.165f, -0.595f, -0.055f, -0.545f, amberColor);
    drawTextCentered("RESET", -0.11f, -0.582f, 0.0075f, 0.012f, 0.0018f, amberColor);

    // Thin separator line
    drawLine(-0.92f, -0.61f, -0.055f, -0.61f, glm::vec3(0.16f, 0.18f, 0.22f));

    // 3. Three Axis Strips (X, Y, Z)
    float angles[3] = { rotX, rotY, rotZ };
    const char labels[3] = { 'X', 'Y', 'Z' };
    float yCenters[3] = { -0.665f, -0.725f, -0.785f };

    for (int i = 0; i < 3; ++i) {
        float yC = yCenters[i];
        float angle = angles[i];
        float normAngle = std::fmod(angle, 360.0f);
        if (normAngle < 0.0f) normAngle += 360.0f;
        int intAngle = static_cast<int>(std::round(normAngle)) % 360;

        // Axis Label
        drawChar(labels[i], -0.92f, yC - 0.007f, 0.010f, 0.015f, textWhite);

        // Left step button [-]
        drawRectOutline(-0.875f, yC - 0.015f, -0.835f, yC + 0.015f, panelBorder);
        drawLine(-0.865f, yC, -0.845f, yC, textWhite); // "-" sign

        // Slider Track
        float trackX0 = -0.81f;
        float trackX1 = -0.42f;
        drawLine(trackX0, yC, trackX1, yC, trackColor);
        // Track ends & center ticks
        drawLine(trackX0, yC - 0.008f, trackX0, yC + 0.008f, trackColor);
        drawLine(trackX1, yC - 0.008f, trackX1, yC + 0.008f, trackColor);
        drawLine((trackX0 + trackX1) * 0.5f, yC - 0.006f, (trackX0 + trackX1) * 0.5f, yC + 0.006f, trackColor);

        // Filled active track portion
        float frac = normAngle / 360.0f;
        float thumbX = trackX0 + frac * (trackX1 - trackX0);
        drawLine(trackX0, yC, thumbX, yC, trackActive);

        // Thumb indicator
        drawRect(thumbX - 0.005f, yC - 0.010f, thumbX + 0.005f, yC + 0.010f, cyanColor);
        drawRectOutline(thumbX - 0.005f, yC - 0.010f, thumbX + 0.005f, yC + 0.010f, brightCyan);

        // Right step button [+]
        drawRectOutline(-0.395f, yC - 0.015f, -0.355f, yC + 0.015f, panelBorder);
        drawLine(-0.385f, yC, -0.365f, yC, textWhite); // Horizontal line of "+"
        drawLine(-0.375f, yC - 0.010f, -0.375f, yC + 0.010f, textWhite); // Vertical line of "+"

        // Live Angle Value: e.g. "X:  45*"
        std::string valStr = std::string(1, labels[i]) + ": " + (intAngle < 10 ? "  " : (intAngle < 100 ? " " : "")) + std::to_string(intAngle) + "*";
        drawText(valStr, -0.32f, yC - 0.007f, 0.009f, 0.014f, 0.0022f, cyanColor);
    }
}

void HUD::renderTopHUD(int levelNumber, int totalLevels, const std::string& levelName,
                       const std::string& levelSubtitle, float matchPercentage, float threshold,
                       bool debugMode) {
    (void)levelName;
    (void)levelSubtitle;

    // Top Bar Background: Minimal dark charcoal slate
    glm::vec3 barBg(0.07f, 0.08f, 0.10f);
    glm::vec3 barBorder(0.18f, 0.20f, 0.25f);
    drawRect(-1.0f, 0.79f, 1.0f, 1.0f, barBg);
    drawLine(-1.0f, 0.79f, 1.0f, 0.79f, barBorder);

    // Color definitions
    glm::vec3 whiteText(0.96f, 0.96f, 0.98f);
    glm::vec3 subText(0.55f, 0.60f, 0.68f);
    glm::vec3 brandTeal(0.04f, 0.72f, 0.82f);
    glm::vec3 brightTeal(0.15f, 0.82f, 0.92f);
    glm::vec3 amber(0.96f, 0.62f, 0.15f);
    glm::vec3 emerald(0.10f, 0.82f, 0.48f);

    // 1. Left side: Title and Subtitle
    drawText("SHADOWFORGE 3D", -0.95f, 0.925f, 0.014f, 0.024f, 0.0035f, whiteText);
    drawText("FORGE THE SHADOW. SOLVE THE SHAPE.", -0.95f, 0.855f, 0.0095f, 0.015f, 0.0022f, subText);

    // 2. Right side: Level and Prominent Match %
    std::string levelStr = "LEVEL " + std::to_string(levelNumber) + " / " + std::to_string(totalLevels);
    drawTextRight(levelStr, 0.95f, 0.925f, 0.012f, 0.019f, 0.0028f, whiteText);

    int intScore = static_cast<int>(std::round(matchPercentage));
    int intTarget = static_cast<int>(std::round(threshold));
    bool isSuccess = (matchPercentage >= threshold);

    glm::vec3 matchColor = isSuccess ? emerald : (intScore >= 80 ? brightTeal : brandTeal);

    std::string matchStr = "MATCH " + std::to_string(intScore) + "%";
    drawTextRight(matchStr, 0.95f, 0.865f, 0.014f, 0.023f, 0.0032f, matchColor);

    std::string targetStr = "TARGET \xE2\x89\xA5 " + std::to_string(intTarget) + "%";
    drawText(targetStr, 0.62f, 0.865f, 0.010f, 0.016f, 0.0024f, subText);

    if (debugMode) {
        drawTextRight("[DEBUG ON]", 0.58f, 0.865f, 0.0085f, 0.014f, 0.002f, amber);
    }

    // Dynamic Progress Meter Bar directly below match percentage
    renderProgressBar(0.62f, 0.815f, 0.95f, 0.832f, matchPercentage, threshold, isSuccess);
}

void HUD::renderProgressBar(float x0, float y0, float x1, float y1,
                            float currentVal, float targetVal, bool isSuccess) {
    glm::vec3 trackColor(0.12f, 0.14f, 0.17f);
    glm::vec3 frameColor(0.22f, 0.25f, 0.30f);
    glm::vec3 emerald(0.10f, 0.82f, 0.48f);
    glm::vec3 brightTeal(0.15f, 0.82f, 0.92f);
    glm::vec3 teal(0.04f, 0.72f, 0.82f);
    glm::vec3 tickWhite(0.90f, 0.92f, 0.96f);

    glm::vec3 fillColor = isSuccess ? emerald : (currentVal >= 80.0f ? brightTeal : teal);

    // Track
    drawRect(x0, y0, x1, y1, trackColor);

    // Fill
    float t = glm::clamp(currentVal / 100.0f, 0.0f, 1.0f);
    float fillX = x0 + (x1 - x0) * t;
    if (fillX > x0 + 0.002f) {
        drawRect(x0, y0, fillX, y1, fillColor);
    }

    // Thin frame
    drawRectOutline(x0, y0, x1, y1, frameColor);

    // Target Threshold Marker at 95%
    float targetX = x0 + (x1 - x0) * (targetVal / 100.0f);
    drawLine(targetX, y0 - 0.004f, targetX, y1 + 0.004f, tickWhite);
}

void HUD::renderLoadingScreen(float animTime, float loadProgress) {
    glm::vec3 bgCol(0.045f, 0.052f, 0.065f);
    glm::vec3 panelBorder(0.18f, 0.22f, 0.28f);
    glm::vec3 brandTeal(0.04f, 0.72f, 0.82f);
    glm::vec3 brightCyan(0.20f, 0.88f, 0.98f);
    glm::vec3 amber(0.96f, 0.65f, 0.15f);
    glm::vec3 emerald(0.10f, 0.85f, 0.48f);
    glm::vec3 whiteText(0.96f, 0.96f, 0.98f);
    glm::vec3 subText(0.48f, 0.54f, 0.62f);
    glm::vec3 darkTrack(0.10f, 0.12f, 0.15f);

    // 1. Full Screen Deep Charcoal Studio Backdrop
    drawRect(-1.0f, -1.0f, 1.0f, 1.0f, bgCol);

    // 2. Corner Aesthetic Framing Brackets
    drawCornerTicks(-0.94f, -0.90f, 0.94f, 0.90f, 0.05f, panelBorder);
    drawCornerTicks(-0.92f, -0.88f, 0.92f, 0.88f, 0.025f, brandTeal * 0.6f);

    // 3. Technical Top Status Ribbon
    drawLine(-0.94f, 0.84f, 0.94f, 0.84f, panelBorder);
    drawText("SHADOWFORGE OS // SYSTEM BOOT // ARCH: C++17", -0.92f, 0.865f, 0.0075f, 0.012f, 0.0018f, subText);
    drawTextRight("OPENGL 3.3 CORE // 60 FPS", 0.92f, 0.865f, 0.0075f, 0.012f, 0.0018f, subText);

    // 4. Grand Main Game Title & Subtitle
    drawTextCentered("SHADOWFORGE 3D", 0.0f, 0.48f, 0.022f, 0.038f, 0.0055f, whiteText);
    // Subtle double stroke glow effect on title
    drawTextCentered("SHADOWFORGE 3D", 0.001f, 0.481f, 0.022f, 0.038f, 0.0055f, brightCyan * 0.45f);

    drawTextCentered("A 3D SPATIAL SHADOW PUZZLE", 0.0f, 0.40f, 0.011f, 0.018f, 0.0028f, brandTeal);
    drawLine(-0.35f, 0.36f, 0.35f, 0.36f, panelBorder);

    // 5. Central Animated 3D Geometric "Forging" Wireframe Artifact
    // An animated 3D dual-pyramid (octahedron) rotating live in perspective
    float centerX = 0.0f;
    float centerY = 0.08f;
    float rotY = animTime * 1.6f;
    float rotX = 0.38f; // Constant downward camera tilt

    // 6 vertices of 3D octahedron
    float radius = 0.16f + 0.008f * std::sin(animTime * 3.5f);
    glm::vec3 localPts[6] = {
        { 0.0f,  radius * 1.25f, 0.0f }, // Top
        { 0.0f, -radius * 1.25f, 0.0f }, // Bottom
        {  radius, 0.0f, 0.0f },         // Right
        { -radius, 0.0f, 0.0f },         // Left
        { 0.0f, 0.0f,  radius },         // Front
        { 0.0f, 0.0f, -radius }          // Back
    };

    glm::vec2 projPts[6];
    for (int i = 0; i < 6; ++i) {
        // Rotate around Y
        float cy = std::cos(rotY), sy = std::sin(rotY);
        float x1 = localPts[i].x * cy + localPts[i].z * sy;
        float y1 = localPts[i].y;
        float z1 = -localPts[i].x * sy + localPts[i].z * cy;

        // Rotate around X
        float cx = std::cos(rotX), sx = std::sin(rotX);
        float y2 = y1 * cx - z1 * sx;

        projPts[i] = glm::vec2(centerX + x1, centerY + y2);
    }

    // Connect equatorial loop
    int eqIndices[4] = { 2, 4, 3, 5 };
    for (int i = 0; i < 4; ++i) {
        int next = eqIndices[(i + 1) % 4];
        drawLine(projPts[eqIndices[i]].x, projPts[eqIndices[i]].y,
                 projPts[next].x, projPts[next].y, brightCyan);
        // Connect to top
        drawLine(projPts[0].x, projPts[0].y,
                 projPts[eqIndices[i]].x, projPts[eqIndices[i]].y, brandTeal);
        // Connect to bottom
        drawLine(projPts[1].x, projPts[1].y,
                 projPts[eqIndices[i]].x, projPts[eqIndices[i]].y, brandTeal * 0.75f);
    }

    // Outer rotating dashed reticle ring around center
    int ringSegments = 16;
    float ringRad = radius * 1.55f;
    float ringRot = -animTime * 0.8f;
    for (int i = 0; i < ringSegments; i += 2) {
        float a0 = ringRot + (float(i) / float(ringSegments)) * 6.28318f;
        float a1 = ringRot + (float(i + 1) / float(ringSegments)) * 6.28318f;
        drawLine(centerX + ringRad * std::cos(a0), centerY + ringRad * std::sin(a0) * 0.5f,
                 centerX + ringRad * std::cos(a1), centerY + ringRad * std::sin(a1) * 0.5f,
                 panelBorder);
    }

    // 6. Loading Progress Bar & Percentage
    float barX0 = -0.42f;
    float barX1 =  0.42f;
    float barY0 = -0.21f;
    float barY1 = -0.185f;

    // Track
    drawRect(barX0, barY0, barX1, barY1, darkTrack);
    drawRectOutline(barX0, barY0, barX1, barY1, panelBorder);

    // Active Fill
    float fillProgress = glm::clamp(loadProgress, 0.0f, 1.0f);
    float fillX = barX0 + fillProgress * (barX1 - barX0);
    if (fillX > barX0 + 0.003f) {
        glm::vec3 fillCol = (fillProgress >= 1.0f) ? emerald : brightCyan;
        drawRect(barX0, barY0, fillX, barY1, fillCol);
        // Leading glowing edge tick
        drawLine(fillX, barY0 - 0.005f, fillX, barY1 + 0.005f, whiteText);
    }

    // Percentage Readout
    int intPercent = static_cast<int>(std::round(fillProgress * 100.0f));
    std::string pctStr = std::to_string(intPercent) + "%";
    drawTextRight(pctStr, barX1, -0.155f, 0.012f, 0.018f, 0.0028f, (fillProgress >= 1.0f) ? emerald : brightCyan);

    // Dynamic Loading Subsystem Status Message
    std::string phaseMsg;
    if (fillProgress < 0.25f) {
        phaseMsg = "CALIBRATING 3D GRAPHICS PIPELINE...";
    } else if (fillProgress < 0.50f) {
        phaseMsg = "CONSTRUCTING POLYGONAL MESH SCULPTURES...";
    } else if (fillProgress < 0.75f) {
        phaseMsg = "INITIALIZING ORTHOGONAL YZX ROTATION ENGINE...";
    } else if (fillProgress < 1.0f) {
        phaseMsg = "RASTERIZING 1,156-PROBE SILHOUETTE MATCHER...";
    } else {
        phaseMsg = "FORGE READY \xE2\x9C\x93 SYSTEM ONLINE";
    }
    drawText(phaseMsg, barX0, -0.155f, 0.009f, 0.014f, 0.0022f, (fillProgress >= 1.0f) ? emerald : subText);

    // 7. Interactive Prompt Below
    if (fillProgress >= 1.0f) {
        float pulse = 0.65f + 0.35f * std::sin(animTime * 5.0f);
        drawTextCentered("PRESS [SPACE] OR CLICK TO ENTER", 0.0f, -0.29f, 0.012f, 0.019f, 0.0028f, emerald * pulse);
    } else {
        drawTextCentered("PRESS [SPACE] OR CLICK TO SKIP", 0.0f, -0.29f, 0.008f, 0.013f, 0.0020f, subText * 0.85f);
    }

    // 8. Bottom Footer Technical Specs
    drawLine(-0.94f, -0.84f, 0.94f, -0.84f, panelBorder);
    drawText("SHADOWFORGE 3D // 5 PROCEDURAL PUZZLES // IOU SIMILARITY ENGINE", -0.92f, -0.875f, 0.007f, 0.011f, 0.0016f, subText * 0.75f);
    drawTextRight("READY FOR DISCOVERY", 0.92f, -0.875f, 0.007f, 0.011f, 0.0016f, subText * 0.75f);
}

void HUD::renderMenuOverlay(float animTime) {
    // Centered minimalist menu card
    float cx = 0.0f;
    float cy = 0.0f;
    float w  = 0.62f;
    float h  = 0.54f;

    float x0 = cx - w * 0.5f;
    float x1 = cx + w * 0.5f;
    float y0 = cy - h * 0.5f;
    float y1 = cy + h * 0.5f;

    glm::vec3 cardBg(0.07f, 0.08f, 0.10f);
    glm::vec3 cardBorder(0.20f, 0.24f, 0.30f);
    glm::vec3 brandTeal(0.04f, 0.72f, 0.82f);
    glm::vec3 whiteText(0.96f, 0.96f, 0.98f);
    glm::vec3 softWhite(0.85f, 0.88f, 0.92f);
    glm::vec3 subText(0.55f, 0.60f, 0.68f);
    glm::vec3 dividerCol(0.16f, 0.18f, 0.22f);

    // Solid card backdrop and subtle border
    drawRect(x0, y0, x1, y1, cardBg);
    drawRectOutline(x0, y0, x1, y1, cardBorder);

    // Title & Tagline
    drawTextCentered("SHADOWFORGE 3D", cx, cy + 0.18f, 0.016f, 0.026f, 0.004f, whiteText);
    drawTextCentered("FORGE THE SHADOW.", cx, cy + 0.11f, 0.0115f, 0.018f, 0.0028f, softWhite);
    drawTextCentered("SOLVE THE SHAPE.", cx, cy + 0.06f, 0.0115f, 0.018f, 0.0028f, subText);

    drawLine(cx - 0.22f, cy + 0.02f, cx + 0.22f, cy + 0.02f, dividerCol);

    // Pulsing Start Prompt
    float pulse = 0.70f + 0.30f * std::sin(animTime * 4.0f);
    glm::vec3 promptColor = brandTeal * pulse;
    drawTextCentered("PRESS SPACE TO BEGIN", cx, cy - 0.04f, 0.0125f, 0.020f, 0.003f, promptColor);

    drawLine(cx - 0.22f, cy - 0.09f, cx + 0.22f, cy - 0.09f, dividerCol);

    // Controls
    drawText("A / D   X ROTATE", cx - 0.18f, cy - 0.14f, 0.0095f, 0.015f, 0.0024f, subText);
    drawText("W / S   Y ROTATE", cx - 0.18f, cy - 0.18f, 0.0095f, 0.015f, 0.0024f, subText);
    drawText("Q / E   Z ROTATE", cx - 0.18f, cy - 0.22f, 0.0095f, 0.015f, 0.0024f, subText);
    drawText("H       SHOW TARGET", cx + 0.03f, cy - 0.14f, 0.0095f, 0.015f, 0.0024f, subText);
    drawText("R       RESTART",     cx + 0.03f, cy - 0.18f, 0.0095f, 0.015f, 0.0024f, subText);
    drawText("F3      DEBUG",       cx + 0.03f, cy - 0.22f, 0.0095f, 0.015f, 0.0024f, subText);
}

void HUD::renderSuccessCard(int levelNumber, float matchScore, float animTime) {
    (void)matchScore;

    // Compact floating card centered between panels
    float cx = 0.0f;
    float cy = 0.0f;
    float w  = 0.46f;
    float h  = 0.22f;

    float x0 = cx - w * 0.5f;
    float x1 = cx + w * 0.5f;
    float y0 = cy - h * 0.5f;
    float y1 = cy + h * 0.5f;

    glm::vec3 cardBg(0.07f, 0.11f, 0.08f);
    glm::vec3 cardBorder(0.12f, 0.65f, 0.36f);
    glm::vec3 emerald(0.10f, 0.85f, 0.48f);
    glm::vec3 whiteText(0.94f, 0.96f, 0.96f);

    drawRect(x0, y0, x1, y1, cardBg);
    drawRectOutline(x0, y0, x1, y1, cardBorder);

    // Title with checkmark ✓
    drawTextCentered("SHADOW FORGED \xE2\x9C\x93", cx, cy + 0.055f, 0.016f, 0.025f, 0.0035f, emerald);

    std::string completeStr = "LEVEL " + std::to_string(levelNumber) + " COMPLETE";
    drawTextCentered(completeStr, cx, cy + 0.005f, 0.0115f, 0.018f, 0.0028f, whiteText);

    // Advance Prompt
    float pulse = 0.70f + 0.30f * std::sin(animTime * 4.0f);
    glm::vec3 promptColor = emerald * pulse;
    drawTextCentered("PRESS SPACE / CLICK TO CONTINUE", cx, cy - 0.055f, 0.0095f, 0.015f, 0.0022f, promptColor);
}

void HUD::renderCompleteCard(float animTime) {
    // Compact victory card
    float cx = 0.0f;
    float cy = 0.0f;
    float w  = 0.54f;
    float h  = 0.30f;

    float x0 = cx - w * 0.5f;
    float x1 = cx + w * 0.5f;
    float y0 = cy - h * 0.5f;
    float y1 = cy + h * 0.5f;

    glm::vec3 cardBg(0.07f, 0.09f, 0.11f);
    glm::vec3 cardBorder(0.20f, 0.24f, 0.30f);
    glm::vec3 brandTeal(0.04f, 0.72f, 0.82f);
    glm::vec3 emerald(0.10f, 0.85f, 0.48f);
    glm::vec3 whiteText(0.96f, 0.96f, 0.98f);
    glm::vec3 subText(0.55f, 0.60f, 0.68f);
    glm::vec3 dividerCol(0.16f, 0.18f, 0.22f);

    drawRect(x0, y0, x1, y1, cardBg);
    drawRectOutline(x0, y0, x1, y1, cardBorder);

    drawTextCentered("SHADOWFORGE COMPLETE", cx, cy + 0.08f, 0.016f, 0.026f, 0.0035f, whiteText);
    drawTextCentered("ALL 5 SHADOWS FORGED \xE2\x9C\x93", cx, cy + 0.02f, 0.0125f, 0.020f, 0.003f, emerald);
    drawLine(cx - 0.20f, cy - 0.02f, cx + 0.20f, cy - 0.02f, dividerCol);

    float pulse = 0.70f + 0.30f * std::sin(animTime * 4.0f);
    drawTextCentered("PRESS R TO PLAY AGAIN", cx, cy - 0.07f, 0.011f, 0.017f, 0.0026f, brandTeal * pulse);
    drawTextCentered("ESC TO EXIT", cx, cy - 0.11f, 0.0095f, 0.0145f, 0.0022f, subText);
}

void HUD::render(GLint modelLoc, GLint viewLoc, GLint projLoc, GLint renderModeLoc, GLint alphaLoc) {
    if (!m_isInitialized) return;

    // Use Identity MVP matrices since HUD coordinates are directly in NDC [-1, 1]
    glm::mat4 identity(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(viewLoc,  1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(projLoc,  1, GL_FALSE, glm::value_ptr(identity));

    // Ensure unlit rendering mode for HUD elements
    if (renderModeLoc >= 0) {
        glUniform1i(renderModeLoc, 0);
    }
    if (alphaLoc >= 0) {
        glUniform1f(alphaLoc, 1.0f);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Draw Quads / Solid cards (GL_TRIANGLES)
    if (!m_triVertices.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_triVbo);
        glBufferData(GL_ARRAY_BUFFER, m_triVertices.size() * sizeof(Vertex), m_triVertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(m_triVao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_triVertices.size()));
        glBindVertexArray(0);
    }

    // 2. Draw Lines & Typography strokes (GL_LINES)
    if (!m_lineVertices.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
        glBufferData(GL_ARRAY_BUFFER, m_lineVertices.size() * sizeof(Vertex), m_lineVertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(m_lineVao);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_lineVertices.size()));
        glBindVertexArray(0);
    }
}
