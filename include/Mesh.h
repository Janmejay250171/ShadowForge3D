#ifndef SHADOWFORGE_MESH_H
#define SHADOWFORGE_MESH_H

#include <vector>
#include "GLCommon.h"
#include <glm/glm.hpp>

/**
 * Vertex data structure representing a single point in 3D space with color.
 * Memory layout matches GLSL layout (location = 0) vec3, (location = 1) vec3.
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;

    Vertex() = default;
    Vertex(const glm::vec3& pos, const glm::vec3& col) : position(pos), color(col) {}
};

/**
 * Lightweight procedural mesh container.
 * Manages VAO and VBO buffers and provides helper functions to append
 * procedural primitive shapes (boxes, pyramids, etc.).
 */
class Mesh {
private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    std::vector<Vertex> m_vertices;
    bool m_isUploaded = false;

public:
    Mesh() = default;
    ~Mesh();

    // Prevent accidental copying of GPU buffer IDs
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Allow move semantics
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // Procedural primitive builders
    void addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);
    void addQuad(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& v3);
    void addBox(const glm::vec3& center, const glm::vec3& size, const glm::vec3& baseColor);
    void addPyramid(const glm::vec3& baseCenter, const glm::vec2& baseSize, float height, const glm::vec3& baseColor);

    // GPU buffer management
    void upload();
    void draw() const;
    void destroy();

    size_t getVertexCount() const { return m_vertices.size(); }
    size_t getTriangleCount() const { return m_vertices.size() / 3; }
    const std::vector<Vertex>& getVertices() const { return m_vertices; }
};

#endif // SHADOWFORGE_MESH_H
