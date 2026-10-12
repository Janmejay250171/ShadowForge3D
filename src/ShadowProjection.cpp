#include "ShadowProjection.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <glm/gtc/type_ptr.hpp>

ShadowProjection::~ShadowProjection() {
    destroy();
}

static void setupVAO(GLuint& vao, GLuint& vbo) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Attribute 0: Position vec3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Attribute 1: Color vec3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Quantized coordinate key for robust 2D boundary edge detection
struct QuantizedPoint {
    int x;
    int y;
    bool operator==(const QuantizedPoint& o) const { return x == o.x && y == o.y; }
    bool operator<(const QuantizedPoint& o) const {
        if (x != o.x) return x < o.x;
        return y < o.y;
    }
};

struct EdgeKey {
    QuantizedPoint p1;
    QuantizedPoint p2;
    bool operator<(const EdgeKey& o) const {
        if (!(p1 == o.p1)) return p1 < o.p1;
        return p2 < o.p2;
    }
};

void ShadowProjection::projectGeometry(const std::vector<Vertex>& inVertices,
                                      const Matrix4& modelMatrix,
                                      const glm::mat4& viewMatrix,
                                      const glm::mat4& projMatrix,
                                      const glm::vec3& fillColor,
                                      const glm::vec3& lineColor,
                                      const glm::vec3& debugLineColor,
                                      std::vector<Vertex>& outFillVertices,
                                      std::vector<Vertex>& outCleanLineVertices,
                                      std::vector<Vertex>& outDebugLineVertices) {
    outFillVertices.clear();
    outCleanLineVertices.clear();
    outDebugLineVertices.clear();

    if (inVertices.size() < 3) return;

    outFillVertices.reserve(inVertices.size());
    outDebugLineVertices.reserve(inVertices.size() * 2);

    std::map<EdgeKey, int> edgeCount;
    std::map<EdgeKey, std::pair<glm::vec3, glm::vec3>> edgeCoords;

    // Iterate through triangles (every 3 consecutive vertices)
    for (size_t i = 0; i + 2 < inVertices.size(); i += 3) {
        glm::vec3 ndcTri[3];

        for (int j = 0; j < 3; ++j) {
            const Vertex& v = inVertices[i + j];

            // 1. Local vertex -> Model space (using custom Matrix4)
            Vector4 localP(v.position.x, v.position.y, v.position.z, 1.0f);
            Vector4 worldP = modelMatrix.transformPoint(localP);

            // 2. World space -> View (Camera) space
            glm::vec4 viewP = viewMatrix * glm::vec4(worldP.x, worldP.y, worldP.z, 1.0f);

            // 3. View space -> Clip space
            glm::vec4 clipP = projMatrix * viewP;

            // 4. Clip space -> Perspective divide by w -> Normalized Device Coordinates (NDC)
            float invW = (std::abs(clipP.w) > 1e-5f) ? (1.0f / clipP.w) : 1.0f;
            ndcTri[j] = glm::vec3(clipP.x * invW, clipP.y * invW, 0.0f);
        }

        // Add 2D projected triangle (EXACTLY preserved for mathematical similarity in ShadowMatcher)
        outFillVertices.push_back(Vertex(ndcTri[0], fillColor));
        outFillVertices.push_back(Vertex(ndcTri[1], fillColor));
        outFillVertices.push_back(Vertex(ndcTri[2], fillColor));

        // Register 2D edges for clean boundary & 3D wireframe extraction
        auto registerEdge = [&](const glm::vec3& a, const glm::vec3& b) {
            QuantizedPoint qa{ static_cast<int>(std::round(a.x * 2000.0f)), static_cast<int>(std::round(a.y * 2000.0f)) };
            QuantizedPoint qb{ static_cast<int>(std::round(b.x * 2000.0f)), static_cast<int>(std::round(b.y * 2000.0f)) };
            if (qa == qb) return;
            EdgeKey key = (qa < qb) ? EdgeKey{qa, qb} : EdgeKey{qb, qa};
            edgeCount[key]++;
            edgeCoords[key] = {a, b};
        };

        registerEdge(ndcTri[0], ndcTri[1]);
        registerEdge(ndcTri[1], ndcTri[2]);
        registerEdge(ndcTri[2], ndcTri[0]);
    }

    // Build unique 3D structural lines for wireframe rendering
    outDebugLineVertices.clear();
    outDebugLineVertices.reserve(edgeCoords.size() * 2);
    for (const auto& kv : edgeCoords) {
        int count = edgeCount[kv.first];
        // Silhouette boundary edges receive a slightly brighter tone, interior 3D lines remain crisp
        glm::vec3 col = (count == 1) ? glm::clamp(debugLineColor * 1.15f, 0.0f, 1.0f) : debugLineColor;
        outDebugLineVertices.push_back(Vertex(kv.second.first, col));
        outDebugLineVertices.push_back(Vertex(kv.second.second, col));
    }

    // Extract clean outer perimeter boundary edges:
    // Sub-segment candidate boundary edges at intersection points and eliminate all interior edges
    struct SegmentCandidate {
        glm::vec2 p0;
        glm::vec2 p1;
        glm::vec3 orig0;
        glm::vec3 orig1;
    };

    std::vector<SegmentCandidate> candidates;
    for (const auto& kv : edgeCount) {
        if (kv.second == 1) {
            candidates.push_back({
                glm::vec2(edgeCoords[kv.first].first.x, edgeCoords[kv.first].first.y),
                glm::vec2(edgeCoords[kv.first].second.x, edgeCoords[kv.first].second.y),
                edgeCoords[kv.first].first,
                edgeCoords[kv.first].second
            });
        }
    }

    size_t numCand = candidates.size();
    std::vector<std::vector<float>> splits(numCand);
    for (size_t i = 0; i < numCand; ++i) {
        splits[i].push_back(0.0f);
        splits[i].push_back(1.0f);
    }

    // Intersect candidate boundary edges with each other
    for (size_t i = 0; i < numCand; ++i) {
        const glm::vec2& a = candidates[i].p0;
        const glm::vec2& b = candidates[i].p1;
        glm::vec2 u = b - a;

        for (size_t j = i + 1; j < numCand; ++j) {
            const glm::vec2& c = candidates[j].p0;
            const glm::vec2& d = candidates[j].p1;
            glm::vec2 v = d - c;

            float det = u.x * v.y - u.y * v.x;
            if (std::abs(det) < 1e-6f) continue;

            glm::vec2 diff = c - a;
            float t = (diff.x * v.y - diff.y * v.x) / det;
            float s = (diff.x * u.y - diff.y * u.x) / det;

            if (t > 1e-4f && t < 1.0f - 1e-4f && s > 1e-4f && s < 1.0f - 1e-4f) {
                splits[i].push_back(t);
                splits[j].push_back(s);
            }
        }
    }

    // Test each sub-segment: discard if strictly inside any triangle
    std::map<QuantizedPoint, bool> visitedMidpoints;

    for (size_t i = 0; i < numCand; ++i) {
        auto& sList = splits[i];
        std::sort(sList.begin(), sList.end());

        const glm::vec2& a = candidates[i].p0;
        const glm::vec2& b = candidates[i].p1;
        glm::vec2 u = b - a;

        for (size_t k = 0; k + 1 < sList.size(); ++k) {
            float t0 = sList[k];
            float t1 = sList[k+1];
            if (t1 - t0 < 1e-4f) continue;

            glm::vec2 subA = a + t0 * u;
            glm::vec2 subB = a + t1 * u;
            glm::vec2 mid = (subA + subB) * 0.5f;

            QuantizedPoint qm { static_cast<int>(std::round(mid.x * 2000.0f)), static_cast<int>(std::round(mid.y * 2000.0f)) };
            if (visitedMidpoints.find(qm) != visitedMidpoints.end()) {
                continue;
            }

            bool insideAny = false;
            for (size_t t = 0; t + 2 < outFillVertices.size(); t += 3) {
                glm::vec2 tri0(outFillVertices[t].position.x,   outFillVertices[t].position.y);
                glm::vec2 tri1(outFillVertices[t+1].position.x, outFillVertices[t+1].position.y);
                glm::vec2 tri2(outFillVertices[t+2].position.x, outFillVertices[t+2].position.y);

                float minX = std::min({tri0.x, tri1.x, tri2.x});
                float maxX = std::max({tri0.x, tri1.x, tri2.x});
                float minY = std::min({tri0.y, tri1.y, tri2.y});
                float maxY = std::max({tri0.y, tri1.y, tri2.y});

                if (mid.x < minX - 1e-4f || mid.x > maxX + 1e-4f || mid.y < minY - 1e-4f || mid.y > maxY + 1e-4f) {
                    continue;
                }

                float d1 = (mid.x - tri1.x) * (tri0.y - tri1.y) - (tri0.x - tri1.x) * (mid.y - tri1.y);
                float d2 = (mid.x - tri2.x) * (tri1.y - tri2.y) - (tri1.x - tri2.x) * (mid.y - tri2.y);
                float d3 = (mid.x - tri0.x) * (tri2.y - tri0.y) - (tri2.x - tri0.x) * (mid.y - tri0.y);

                float eps = 1.5e-4f;
                bool allPos = (d1 > eps && d2 > eps && d3 > eps);
                bool allNeg = (d1 < -eps && d2 < -eps && d3 < -eps);

                if (allPos || allNeg) {
                    insideAny = true;
                    break;
                }
            }

            if (!insideAny) {
                visitedMidpoints[qm] = true;
                outCleanLineVertices.push_back(Vertex(glm::vec3(subA.x, subA.y, 0.0f), lineColor));
                outCleanLineVertices.push_back(Vertex(glm::vec3(subB.x, subB.y, 0.0f), lineColor));
            }
        }
    }

    // Robust fallback if no outer segments survived
    if (outCleanLineVertices.empty()) {
        for (const auto& kv : edgeCount) {
            if (kv.second == 1) {
                outCleanLineVertices.push_back(Vertex(edgeCoords[kv.first].first, lineColor));
                outCleanLineVertices.push_back(Vertex(edgeCoords[kv.first].second, lineColor));
            }
        }
    }
}





void ShadowProjection::init(const std::vector<Vertex>& modelVertices,
                           const Matrix4& targetModelMatrix,
                           const glm::mat4& viewMatrix,
                           const glm::mat4& projMatrix) {
    destroy();

    // 1. Build Laboratory Canvas Backdrop Frame
    std::vector<Vertex> canvasLines;
    glm::vec3 frameBorderColor(0.24f, 0.30f, 0.40f);
    glm::vec3 gridColor(0.12f, 0.16f, 0.22f);
    glm::vec3 reticleColor(0.35f, 0.45f, 0.58f);

    float b = 0.85f; // Outer boundary

    // Outer border rectangle
    canvasLines.push_back(Vertex(glm::vec3(-b, -b, 0.0f), frameBorderColor));
    canvasLines.push_back(Vertex(glm::vec3( b, -b, 0.0f), frameBorderColor));

    canvasLines.push_back(Vertex(glm::vec3( b, -b, 0.0f), frameBorderColor));
    canvasLines.push_back(Vertex(glm::vec3( b,  b, 0.0f), frameBorderColor));

    canvasLines.push_back(Vertex(glm::vec3( b,  b, 0.0f), frameBorderColor));
    canvasLines.push_back(Vertex(glm::vec3(-b,  b, 0.0f), frameBorderColor));

    canvasLines.push_back(Vertex(glm::vec3(-b,  b, 0.0f), frameBorderColor));
    canvasLines.push_back(Vertex(glm::vec3(-b, -b, 0.0f), frameBorderColor));

    // Subtle technical grid lines
    float gridSteps[] = { -0.55f, -0.28f, 0.28f, 0.55f };
    for (float g : gridSteps) {
        canvasLines.push_back(Vertex(glm::vec3(-b, g, 0.0f), gridColor));
        canvasLines.push_back(Vertex(glm::vec3( b, g, 0.0f), gridColor));

        canvasLines.push_back(Vertex(glm::vec3(g, -b, 0.0f), gridColor));
        canvasLines.push_back(Vertex(glm::vec3(g,  b, 0.0f), gridColor));
    }

    // Center alignment crosshairs
    float c = 0.08f;
    canvasLines.push_back(Vertex(glm::vec3(-c, 0.0f, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3( c, 0.0f, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(0.0f, -c, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(0.0f,  c, 0.0f), reticleColor));

    // Corner alignment brackets
    float bk = 0.05f;
    canvasLines.push_back(Vertex(glm::vec3(-b, b - bk, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(-b, b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(-b, b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(-b + bk, b, 0.0f), reticleColor));

    canvasLines.push_back(Vertex(glm::vec3(b - bk, b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(b, b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(b, b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(b, b - bk, 0.0f), reticleColor));

    canvasLines.push_back(Vertex(glm::vec3(-b, -b + bk, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(-b, -b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(-b, -b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(-b + bk, -b, 0.0f), reticleColor));

    canvasLines.push_back(Vertex(glm::vec3(b - bk, -b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(b, -b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(b, -b, 0.0f), reticleColor));
    canvasLines.push_back(Vertex(glm::vec3(b, -b + bk, 0.0f), reticleColor));

    setupVAO(m_canvasVao, m_canvasVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_canvasVbo);
    glBufferData(GL_ARRAY_BUFFER, canvasLines.size() * sizeof(Vertex), canvasLines.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_canvasVertexCount = canvasLines.size();

    // 2. Precompute Fixed Target Silhouette:
    // Warm Amber/Gold 3D wireframe (#f59e0b) and subtle translucent amber ghost fill
    glm::vec3 targetFillColor(0.96f, 0.62f, 0.15f);
    glm::vec3 targetLineColor(0.98f, 0.68f, 0.15f);
    glm::vec3 targetDebugColor(1.00f, 0.74f, 0.18f);

    std::vector<Vertex> targetCleanLines, targetDebugLines;
    projectGeometry(modelVertices, targetModelMatrix, viewMatrix, projMatrix,
                    targetFillColor, targetLineColor, targetDebugColor,
                    m_targetFillVertices, targetCleanLines, targetDebugLines);

    setupVAO(m_targetFillVao, m_targetFillVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_targetFillVbo);
    glBufferData(GL_ARRAY_BUFFER, m_targetFillVertices.size() * sizeof(Vertex), m_targetFillVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_targetFillVertexCount = m_targetFillVertices.size();

    // Clean outline VAO
    setupVAO(m_targetLineVao, m_targetLineVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_targetLineVbo);
    glBufferData(GL_ARRAY_BUFFER, targetCleanLines.size() * sizeof(Vertex), targetCleanLines.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_targetLineVertexCount = targetCleanLines.size();

    // 3D structural wireframe lines VAO
    setupVAO(m_targetDebugLineVao, m_targetDebugLineVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_targetDebugLineVbo);
    glBufferData(GL_ARRAY_BUFFER, targetDebugLines.size() * sizeof(Vertex), targetDebugLines.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_targetDebugLineVertexCount = targetDebugLines.size();

    // 3. Setup Dynamic Live Silhouette VAOs/VBOs
    setupVAO(m_liveFillVao, m_liveFillVbo);
    setupVAO(m_liveLineVao, m_liveLineVbo);
    setupVAO(m_liveDebugLineVao, m_liveDebugLineVbo);

    m_isInitialized = true;
    std::cout << "[ShadowProjection] Initialized projection canvas and target silhouette ("
              << m_targetFillVertexCount / 3 << " triangles, cleanLines=" << targetCleanLines.size()
              << ", 3DLines=" << targetDebugLines.size() / 2 << ")." << std::endl;

}

void ShadowProjection::updateLiveProjection(const std::vector<Vertex>& modelVertices,
                                           const Matrix4& currentModelMatrix,
                                           const glm::mat4& viewMatrix,
                                           const glm::mat4& projMatrix,
                                           bool isMatched) {
    if (!m_isInitialized) return;

    // Live Silhouette Colors:
    // Normal: Vibrant Electric Cyan 3D lines (#06b6d4) & soft translucent cyan fill
    // Matched (Victory): Radiant Emerald Green 3D lines (#10b981) & emerald fill
    glm::vec3 liveFillColor  = isMatched ? glm::vec3(0.08f, 0.85f, 0.45f) : glm::vec3(0.04f, 0.72f, 0.85f);
    glm::vec3 liveLineColor  = isMatched ? glm::vec3(0.12f, 0.95f, 0.50f) : glm::vec3(0.08f, 0.88f, 0.98f);
    glm::vec3 liveDebugColor = isMatched ? glm::vec3(0.15f, 1.00f, 0.55f) : glm::vec3(0.10f, 0.88f, 0.98f);

    std::vector<Vertex> liveCleanLines, liveDebugLines;
    projectGeometry(modelVertices, currentModelMatrix, viewMatrix, projMatrix,
                    liveFillColor, liveLineColor, liveDebugColor,
                    m_liveFillVertices, liveCleanLines, liveDebugLines);

    // Upload fill vertices to GPU (triangles)
    glBindBuffer(GL_ARRAY_BUFFER, m_liveFillVbo);
    glBufferData(GL_ARRAY_BUFFER, m_liveFillVertices.size() * sizeof(Vertex), m_liveFillVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_liveFillVertexCount = m_liveFillVertices.size();

    // Upload clean boundary line vertices to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_liveLineVbo);
    glBufferData(GL_ARRAY_BUFFER, liveCleanLines.size() * sizeof(Vertex), liveCleanLines.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_liveLineVertexCount = liveCleanLines.size();

    // Upload 3D structural wireframe line vertices to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_liveDebugLineVbo);
    glBufferData(GL_ARRAY_BUFFER, liveDebugLines.size() * sizeof(Vertex), liveDebugLines.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_liveDebugLineVertexCount = liveDebugLines.size();
}

void ShadowProjection::render(GLint modelLoc, GLint viewLoc, GLint projLoc, GLint alphaLoc,
                             bool showTarget, bool debugMode, bool isVictory) {
    if (!m_isInitialized) return;

    // Use Identity matrices since 2D projected coordinates are ALREADY in NDC [-1, 1]
    glm::mat4 identity(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(viewLoc,  1, GL_FALSE, glm::value_ptr(identity));
    glUniformMatrix4fv(projLoc,  1, GL_FALSE, glm::value_ptr(identity));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_STENCIL_TEST);

    // 1. Draw Laboratory Canvas Backdrop Frame and Subtle Grid
    if (m_canvasVao != 0 && m_canvasVertexCount > 0) {
        if (alphaLoc >= 0) glUniform1f(alphaLoc, 0.35f);
        glLineWidth(1.0f);
        glBindVertexArray(m_canvasVao);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_canvasVertexCount));
    }

    // 2. Debug Mode ONLY (F3): Full wireframe developer view
    if (debugMode) {
        if (alphaLoc >= 0) glUniform1f(alphaLoc, 1.0f);
        glLineWidth(2.0f);
        if (m_targetDebugLineVao != 0 && m_targetDebugLineVertexCount > 0) {
            glBindVertexArray(m_targetDebugLineVao);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_targetDebugLineVertexCount));
        }
        if (m_liveDebugLineVao != 0 && m_liveDebugLineVertexCount > 0) {
            glBindVertexArray(m_liveDebugLineVao);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_liveDebugLineVertexCount));
        }
        glLineWidth(1.0f);
        glBindVertexArray(0);
        return;
    }

    // 3. TARGET STUDY MODE (H KEY TOGGLED ON): SHOW TARGET SILHOUETTE IN 3D LINES ONLY
    if (showTarget) {
        if (m_targetDebugLineVao != 0 && m_targetDebugLineVertexCount > 0) {
            if (alphaLoc >= 0) glUniform1f(alphaLoc, 1.0f);
            glLineWidth(2.0f);
            glBindVertexArray(m_targetDebugLineVao);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_targetDebugLineVertexCount));
        }
        glLineWidth(1.0f);
        glBindVertexArray(0);
        return;
    }

    // 4. MIXED MODE (NORMAL GAMEPLAY): BOTH PROJECTED AND TARGET TOGETHER IN 3D LINES
    // A. Target Silhouette: Soft ghost silhouette fill + Crisp Amber/Gold 3D Lines
    if (m_targetFillVao != 0 && m_targetFillVertexCount > 0) {
        if (alphaLoc >= 0) glUniform1f(alphaLoc, 0.16f); // Soft amber ghost
        glBindVertexArray(m_targetFillVao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_targetFillVertexCount));
    }

    if (m_targetDebugLineVao != 0 && m_targetDebugLineVertexCount > 0) {
        if (alphaLoc >= 0) glUniform1f(alphaLoc, 0.88f); // Crisp amber 3D wireframe
        glLineWidth(2.0f);
        glBindVertexArray(m_targetDebugLineVao);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_targetDebugLineVertexCount));
    }

    // B. Live Projected Silhouette: Soft translucent fill + Vibrant 3D Lines (Cyan or Emerald)
    if (m_liveFillVao != 0 && m_liveFillVertexCount > 0) {
        float liveAlpha = isVictory ? 0.35f : 0.22f; // Soft cyan or victory emerald glow
        if (alphaLoc >= 0) glUniform1f(alphaLoc, liveAlpha);
        glBindVertexArray(m_liveFillVao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_liveFillVertexCount));
    }

    if (m_liveDebugLineVao != 0 && m_liveDebugLineVertexCount > 0) {
        if (alphaLoc >= 0) glUniform1f(alphaLoc, 1.0f); // Bright electric cyan or radiant emerald 3D lines
        glLineWidth(2.0f);
        glBindVertexArray(m_liveDebugLineVao);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_liveDebugLineVertexCount));
    }

    glLineWidth(1.0f);
    glBindVertexArray(0);
}

void ShadowProjection::destroy() {
    if (m_liveFillVbo != 0)        { glDeleteBuffers(1, &m_liveFillVbo);        m_liveFillVbo = 0; }
    if (m_liveFillVao != 0)        { glDeleteVertexArrays(1, &m_liveFillVao);   m_liveFillVao = 0; }

    if (m_liveLineVbo != 0)        { glDeleteBuffers(1, &m_liveLineVbo);        m_liveLineVbo = 0; }
    if (m_liveLineVao != 0)        { glDeleteVertexArrays(1, &m_liveLineVao);   m_liveLineVao = 0; }

    if (m_liveDebugLineVbo != 0)   { glDeleteBuffers(1, &m_liveDebugLineVbo);   m_liveDebugLineVbo = 0; }
    if (m_liveDebugLineVao != 0)   { glDeleteVertexArrays(1, &m_liveDebugLineVao); m_liveDebugLineVao = 0; }

    if (m_targetFillVbo != 0)      { glDeleteBuffers(1, &m_targetFillVbo);      m_targetFillVbo = 0; }
    if (m_targetFillVao != 0)      { glDeleteVertexArrays(1, &m_targetFillVao); m_targetFillVao = 0; }

    if (m_targetLineVbo != 0)      { glDeleteBuffers(1, &m_targetLineVbo);      m_targetLineVbo = 0; }
    if (m_targetLineVao != 0)      { glDeleteVertexArrays(1, &m_targetLineVao); m_targetLineVao = 0; }

    if (m_targetDebugLineVbo != 0) { glDeleteBuffers(1, &m_targetDebugLineVbo); m_targetDebugLineVbo = 0; }
    if (m_targetDebugLineVao != 0) { glDeleteVertexArrays(1, &m_targetDebugLineVao); m_targetDebugLineVao = 0; }

    if (m_canvasVbo != 0)          { glDeleteBuffers(1, &m_canvasVbo);          m_canvasVbo = 0; }
    if (m_canvasVao != 0)          { glDeleteVertexArrays(1, &m_canvasVao);     m_canvasVao = 0; }

    m_liveFillVertexCount = 0;
    m_liveLineVertexCount = 0;
    m_liveDebugLineVertexCount = 0;
    m_targetFillVertexCount = 0;
    m_targetLineVertexCount = 0;
    m_targetDebugLineVertexCount = 0;
    m_canvasVertexCount = 0;
    m_targetFillVertices.clear();
    m_liveFillVertices.clear();
    m_isInitialized = false;
}

