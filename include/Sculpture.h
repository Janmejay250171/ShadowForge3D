#ifndef SHADOWFORGE_SCULPTURE_H
#define SHADOWFORGE_SCULPTURE_H

#include "Mesh.h"

/**
 * Procedural ShadowForge Sculpture:
 * A single composite 3D game object constructed from procedural geometric primitives.
 * Supports distinct procedural configurations for all 5 levels.
 */
class Sculpture {
private:
    Mesh m_mesh;
    int m_currentLevel = 1;

public:
    Sculpture() = default;
    ~Sculpture() = default;

    // Generates procedural sculpture geometry for a specific level (1 to 5)
    void buildLevel(int levelNumber);

    // Default builder (Level 1)
    void buildProcedural() { buildLevel(1); }

    // Renders the entire sculpture as a single object
    void draw() const;

    // Releases GPU buffers
    void destroy();

    int getCurrentLevel() const { return m_currentLevel; }
    size_t getVertexCount() const { return m_mesh.getVertexCount(); }
    size_t getTriangleCount() const { return m_mesh.getTriangleCount(); }
    const std::vector<Vertex>& getVertices() const { return m_mesh.getVertices(); }
};

#endif // SHADOWFORGE_SCULPTURE_H
