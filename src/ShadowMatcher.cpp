#include "ShadowMatcher.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

ShadowMatcher::ShadowMatcher() {
    generateSamplePoints();
}

ShadowMatcher::~ShadowMatcher() {
    destroy();
}

void ShadowMatcher::generateSamplePoints() {
    m_samplePoints.clear();
    m_samplePoints.reserve(GRID_RESOLUTION * GRID_RESOLUTION);

    float dx = (m_domainMaxX - m_domainMinX) / static_cast<float>(GRID_RESOLUTION - 1);
    float dy = (m_domainMaxY - m_domainMinY) / static_cast<float>(GRID_RESOLUTION - 1);

    for (int j = 0; j < GRID_RESOLUTION; ++j) {
        float y = m_domainMinY + j * dy;
        for (int i = 0; i < GRID_RESOLUTION; ++i) {
            float x = m_domainMinX + i * dx;
            m_samplePoints.push_back(glm::vec2(x, y));
        }
    }
}

bool ShadowMatcher::isPointInsideTriangles(const glm::vec2& p, const std::vector<Vertex>& triangles) {
    if (triangles.size() < 3) return false;

    // Test each 2D projected triangle
    for (size_t i = 0; i + 2 < triangles.size(); i += 3) {
        glm::vec2 a(triangles[i].position.x,     triangles[i].position.y);
        glm::vec2 b(triangles[i + 1].position.x, triangles[i + 1].position.y);
        glm::vec2 c(triangles[i + 2].position.x, triangles[i + 2].position.y);

        // Quick AABB rejection
        float minX = std::min({a.x, b.x, c.x});
        float maxX = std::max({a.x, b.x, c.x});
        float minY = std::min({a.y, b.y, c.y});
        float maxY = std::max({a.y, b.y, c.y});

        if (p.x < minX || p.x > maxX || p.y < minY || p.y > maxY) {
            continue;
        }

        // 2D Edge function / cross-product orientation test
        float d1 = (p.x - b.x) * (a.y - b.y) - (a.x - b.x) * (p.y - b.y);
        float d2 = (p.x - c.x) * (b.y - c.y) - (b.x - c.x) * (p.y - c.y);
        float d3 = (p.x - a.x) * (c.y - a.y) - (c.x - a.x) * (p.y - a.y);

        bool has_neg = (d1 < -1e-6f) || (d2 < -1e-6f) || (d3 < -1e-6f);
        bool has_pos = (d1 >  1e-6f) || (d2 >  1e-6f) || (d3 >  1e-6f);

        // If all edge signs are consistent (all non-negative or all non-positive), p is inside
        if (!(has_neg && has_pos)) {
            return true;
        }
    }
    return false;
}

void ShadowMatcher::init(const std::vector<Vertex>& targetProjectedVertices) {
    generateSamplePoints();

    // Precompute target occupancy for all grid probe points
    m_targetOccupancy.resize(m_samplePoints.size());
    int targetCoveredCount = 0;

    for (size_t k = 0; k < m_samplePoints.size(); ++k) {
        bool inside = isPointInsideTriangles(m_samplePoints[k], targetProjectedVertices);
        m_targetOccupancy[k] = inside;
        if (inside) targetCoveredCount++;
    }
    m_lastTargetCount = targetCoveredCount;

    // Initialize Match Meter GPU buffers
    if (m_meterVao == 0) {
        glGenVertexArrays(1, &m_meterVao);
        glGenBuffers(1, &m_meterVbo);

        glBindVertexArray(m_meterVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_meterVbo);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    m_isInitialized = true;
    m_currentMatch = 0.0f;
    m_isMatched = false;
    m_lastLiveCount = 0;
    m_lastIntersection = 0;
    m_lastUnion = 0;

    std::cout << "[ShadowMatcher] Target silhouette rasterized across " 
              << m_samplePoints.size() << " probes (" << targetCoveredCount 
              << " target coverage points)." << std::endl;
}

float ShadowMatcher::evaluateSimilarity(const std::vector<Vertex>& liveProjectedVertices) {
    if (!m_isInitialized || m_samplePoints.empty()) {
        return 0.0f;
    }

    int intersectionCount = 0;
    int unionCount = 0;
    int liveCount = 0;

    // Evaluate live geometry against sample probes and compare with cached target
    for (size_t k = 0; k < m_samplePoints.size(); ++k) {
        bool inTarget = m_targetOccupancy[k];
        bool inLive   = isPointInsideTriangles(m_samplePoints[k], liveProjectedVertices);

        if (inLive) liveCount++;
        if (inTarget && inLive) {
            intersectionCount++;
        }
        if (inTarget || inLive) {
            unionCount++;
        }
    }

    m_lastLiveCount = liveCount;
    m_lastIntersection = intersectionCount;
    m_lastUnion = unionCount;

    // Jaccard similarity index (Intersection over Union)
    if (unionCount > 0) {
        m_currentMatch = (static_cast<float>(intersectionCount) / static_cast<float>(unionCount)) * 100.0f;
    } else {
        m_currentMatch = 0.0f;
    }

    // Clamp to [0, 100]
    m_currentMatch = glm::clamp(m_currentMatch, 0.0f, 100.0f);

    // Trigger success when reaching the 95% threshold (or rounding to 95% on HUD)
    if (m_currentMatch >= (SUCCESS_THRESHOLD - 0.5f) || std::round(m_currentMatch) >= SUCCESS_THRESHOLD) {
        m_isMatched = true;
    }

    return m_currentMatch;
}

void ShadowMatcher::renderMatchMeter(GLint modelLoc, GLint viewLoc, GLint projLoc) {
    if (m_meterVao == 0) return;

    // Use Identity matrices for 2D UI overlay
    glm::mat4 identity(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(viewLoc,  1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(projLoc,  1, GL_FALSE, glm::value_ptr(identity));

    // Meter bar dimensions (bottom of projection screen in NDC)
    float left   = -0.75f;
    float right  =  0.75f;
    float bottom = -0.92f;
    float top    = -0.84f;
    float totalWidth = right - left;

    // Background track color
    glm::vec3 trackColor(0.20f, 0.22f, 0.28f);
    glm::vec3 frameColor(0.50f, 0.54f, 0.62f);

    // Fill color: Warm Amber during puzzle, Glowing Emerald Green on success
    glm::vec3 fillColor = m_isMatched ? glm::vec3(0.12f, 0.88f, 0.38f) : glm::vec3(0.96f, 0.58f, 0.12f);
    glm::vec3 tickColor(1.0f, 1.0f, 1.0f);

    std::vector<Vertex> meterGeometry;

    // 1. Background Track (Quad = 2 triangles)
    meterGeometry.push_back(Vertex(glm::vec3(left,  bottom, 0.0f), trackColor));
    meterGeometry.push_back(Vertex(glm::vec3(right, bottom, 0.0f), trackColor));
    meterGeometry.push_back(Vertex(glm::vec3(right, top,    0.0f), trackColor));

    meterGeometry.push_back(Vertex(glm::vec3(left,  bottom, 0.0f), trackColor));
    meterGeometry.push_back(Vertex(glm::vec3(right, top,    0.0f), trackColor));
    meterGeometry.push_back(Vertex(glm::vec3(left,  top,    0.0f), trackColor));

    // 2. Active Progress Fill
    float fillRight = left + totalWidth * (m_currentMatch / 100.0f);
    if (fillRight > left + 0.005f) {
        meterGeometry.push_back(Vertex(glm::vec3(left,      bottom, 0.0f), fillColor));
        meterGeometry.push_back(Vertex(glm::vec3(fillRight, bottom, 0.0f), fillColor));
        meterGeometry.push_back(Vertex(glm::vec3(fillRight, top,    0.0f), fillColor));

        meterGeometry.push_back(Vertex(glm::vec3(left,      bottom, 0.0f), fillColor));
        meterGeometry.push_back(Vertex(glm::vec3(fillRight, top,    0.0f), fillColor));
        meterGeometry.push_back(Vertex(glm::vec3(left,      top,    0.0f), fillColor));
    }

    size_t triangleVertexCount = meterGeometry.size();

    // 3. Track Border Frame (Lines)
    meterGeometry.push_back(Vertex(glm::vec3(left,  bottom, 0.0f), frameColor));
    meterGeometry.push_back(Vertex(glm::vec3(right, bottom, 0.0f), frameColor));

    meterGeometry.push_back(Vertex(glm::vec3(right, bottom, 0.0f), frameColor));
    meterGeometry.push_back(Vertex(glm::vec3(right, top,    0.0f), frameColor));

    meterGeometry.push_back(Vertex(glm::vec3(right, top,    0.0f), frameColor));
    meterGeometry.push_back(Vertex(glm::vec3(left,  top,    0.0f), frameColor));

    meterGeometry.push_back(Vertex(glm::vec3(left,  top,    0.0f), frameColor));
    meterGeometry.push_back(Vertex(glm::vec3(left,  bottom, 0.0f), frameColor));

    // 4. Threshold Indicator Tick at 95%
    float threshX = left + totalWidth * (SUCCESS_THRESHOLD / 100.0f);
    meterGeometry.push_back(Vertex(glm::vec3(threshX, bottom - 0.015f, 0.0f), tickColor));
    meterGeometry.push_back(Vertex(glm::vec3(threshX, top    + 0.015f, 0.0f), tickColor));

    // Upload meter geometry to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_meterVbo);
    glBufferData(GL_ARRAY_BUFFER, meterGeometry.size() * sizeof(Vertex), meterGeometry.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Draw solid track & fill
    glBindVertexArray(m_meterVao);
    if (triangleVertexCount > 0) {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(triangleVertexCount));
    }

    // Draw border lines & threshold tick
    size_t lineVertexCount = meterGeometry.size() - triangleVertexCount;
    if (lineVertexCount > 0) {
        glDrawArrays(GL_LINES, static_cast<GLint>(triangleVertexCount), static_cast<GLsizei>(lineVertexCount));
    }

    glBindVertexArray(0);
}

void ShadowMatcher::reset() {
    m_currentMatch = 0.0f;
    m_isMatched = false;
    std::cout << "[ShadowMatcher] Puzzle reset. Rotate the sculpture to forge the shadow." << std::endl;
}

void ShadowMatcher::destroy() {
    if (m_meterVbo != 0) {
        glDeleteBuffers(1, &m_meterVbo);
        m_meterVbo = 0;
    }
    if (m_meterVao != 0) {
        glDeleteVertexArrays(1, &m_meterVao);
        m_meterVao = 0;
    }
    m_targetOccupancy.clear();
    m_samplePoints.clear();
    m_isInitialized = false;
}
