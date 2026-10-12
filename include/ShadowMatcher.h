#ifndef SHADOWFORGE_SHADOW_MATCHER_H
#define SHADOWFORGE_SHADOW_MATCHER_H

#include <vector>
#include "GLCommon.h"
#include <glm/glm.hpp>
#include "Mesh.h"

/**
 * ShadowMatcher Module:
 * Evaluates geometric silhouette similarity (0% - 100%) between the player's
 * live projected 2D silhouette and the target 2D silhouette.
 * 
 * Algorithm:
 * - 2D Area Occupancy & Intersection-over-Union (Jaccard Index)
 * - Evaluates uniform probe samples over the normalized 2D projection domain
 * - Operates purely on 2D projected geometry (NOT rotation angles)
 * - Threshold: >= 95% indicates successful SHADOW FORGED match
 */
class ShadowMatcher {
public:
    static constexpr float SUCCESS_THRESHOLD = 95.0f; // 95% threshold for victory
    static constexpr int GRID_RESOLUTION = 34;        // 34x34 = 1156 sampling probes

private:
    float m_currentMatch = 0.0f;
    bool m_isMatched = false;

    // Normalization bounding bounds in NDC space
    float m_domainMinX = -0.75f;
    float m_domainMaxX =  0.75f;
    float m_domainMinY = -0.75f;
    float m_domainMaxY =  0.75f;

    // Cached precomputed target occupancy across the sampling grid
    std::vector<bool> m_targetOccupancy;
    std::vector<glm::vec2> m_samplePoints;

    // UI Match Meter GPU buffers
    GLuint m_meterVao = 0;
    GLuint m_meterVbo = 0;
    bool m_isInitialized = false;

    // Diagnostic occupancy metrics
    int m_lastTargetCount = 0;
    int m_lastLiveCount = 0;
    int m_lastIntersection = 0;
    int m_lastUnion = 0;

public:
    ShadowMatcher();
    ~ShadowMatcher();

    // Prevent copying of OpenGL handles
    ShadowMatcher(const ShadowMatcher&) = delete;
    ShadowMatcher& operator=(const ShadowMatcher&) = delete;

    // Initialize matcher with target 2D projected geometry
    void init(const std::vector<Vertex>& targetProjectedVertices);

    // Evaluate live geometric similarity percentage against the target
    float evaluateSimilarity(const std::vector<Vertex>& liveProjectedVertices);

    // Render the visual Match Meter bar on the 2D projection canvas
    void renderMatchMeter(GLint modelLoc, GLint viewLoc, GLint projLoc);

    // Reset game state (triggered by pressing 'R')
    void reset();

    float getMatchPercentage() const { return m_currentMatch; }
    bool isSuccess() const { return m_isMatched; }

    int getTargetOccupancyCount() const { return m_lastTargetCount; }
    int getLastLiveOccupancyCount() const { return m_lastLiveCount; }
    int getLastIntersectionCount() const { return m_lastIntersection; }
    int getLastUnionCount() const { return m_lastUnion; }

    void destroy();

private:
    void generateSamplePoints();
    static bool isPointInsideTriangles(const glm::vec2& p, const std::vector<Vertex>& triangles);
};

#endif // SHADOWFORGE_SHADOW_MATCHER_H
