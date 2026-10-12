#include "Mesh.h"
#include <algorithm>

Mesh::~Mesh() {
    destroy();
}

Mesh::Mesh(Mesh&& other) noexcept 
    : m_vao(other.m_vao),
      m_vbo(other.m_vbo),
      m_vertices(std::move(other.m_vertices)),
      m_isUploaded(other.m_isUploaded) {
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_isUploaded = false;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        destroy();
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_vertices = std::move(other.m_vertices);
        m_isUploaded = other.m_isUploaded;
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_isUploaded = false;
    }
    return *this;
}

void Mesh::addTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) {
    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
}

void Mesh::addQuad(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& v3) {
    // Two triangles forming a quad: (0, 1, 2) and (0, 2, 3)
    addTriangle(v0, v1, v2);
    addTriangle(v0, v2, v3);
}

void Mesh::addBox(const glm::vec3& center, const glm::vec3& size, const glm::vec3& baseColor) {
    float hx = size.x * 0.5f;
    float hy = size.y * 0.5f;
    float hz = size.z * 0.5f;

    // Apply directional face shading to enhance 3D depth perception
    auto tint = [](const glm::vec3& c, float f) {
        return glm::clamp(c * f, 0.0f, 1.0f);
    };

    glm::vec3 frontCol  = tint(baseColor, 1.00f);
    glm::vec3 backCol   = tint(baseColor, 0.72f);
    glm::vec3 topCol    = tint(baseColor, 1.15f);
    glm::vec3 bottomCol = tint(baseColor, 0.60f);
    glm::vec3 rightCol  = tint(baseColor, 0.90f);
    glm::vec3 leftCol   = tint(baseColor, 0.80f);

    float cx = center.x;
    float cy = center.y;
    float cz = center.z;

    // 1. Front face (+Z)
    addQuad(
        Vertex(glm::vec3(cx - hx, cy - hy, cz + hz), frontCol),
        Vertex(glm::vec3(cx + hx, cy - hy, cz + hz), frontCol),
        Vertex(glm::vec3(cx + hx, cy + hy, cz + hz), frontCol),
        Vertex(glm::vec3(cx - hx, cy + hy, cz + hz), frontCol)
    );

    // 2. Back face (-Z)
    addQuad(
        Vertex(glm::vec3(cx + hx, cy - hy, cz - hz), backCol),
        Vertex(glm::vec3(cx - hx, cy - hy, cz - hz), backCol),
        Vertex(glm::vec3(cx - hx, cy + hy, cz - hz), backCol),
        Vertex(glm::vec3(cx + hx, cy + hy, cz - hz), backCol)
    );

    // 3. Top face (+Y)
    addQuad(
        Vertex(glm::vec3(cx - hx, cy + hy, cz + hz), topCol),
        Vertex(glm::vec3(cx + hx, cy + hy, cz + hz), topCol),
        Vertex(glm::vec3(cx + hx, cy + hy, cz - hz), topCol),
        Vertex(glm::vec3(cx - hx, cy + hy, cz - hz), topCol)
    );

    // 4. Bottom face (-Y)
    addQuad(
        Vertex(glm::vec3(cx - hx, cy - hy, cz - hz), bottomCol),
        Vertex(glm::vec3(cx + hx, cy - hy, cz - hz), bottomCol),
        Vertex(glm::vec3(cx + hx, cy - hy, cz + hz), bottomCol),
        Vertex(glm::vec3(cx - hx, cy - hy, cz + hz), bottomCol)
    );

    // 5. Right face (+X)
    addQuad(
        Vertex(glm::vec3(cx + hx, cy - hy, cz + hz), rightCol),
        Vertex(glm::vec3(cx + hx, cy - hy, cz - hz), rightCol),
        Vertex(glm::vec3(cx + hx, cy + hy, cz - hz), rightCol),
        Vertex(glm::vec3(cx + hx, cy + hy, cz + hz), rightCol)
    );

    // 6. Left face (-X)
    addQuad(
        Vertex(glm::vec3(cx - hx, cy - hy, cz - hz), leftCol),
        Vertex(glm::vec3(cx - hx, cy - hy, cz + hz), leftCol),
        Vertex(glm::vec3(cx - hx, cy + hy, cz + hz), leftCol),
        Vertex(glm::vec3(cx - hx, cy + hy, cz - hz), leftCol)
    );
}

void Mesh::addPyramid(const glm::vec3& baseCenter, const glm::vec2& baseSize, float height, const glm::vec3& baseColor) {
    float hx = baseSize.x * 0.5f;
    float hz = baseSize.y * 0.5f;
    float cx = baseCenter.x;
    float cy = baseCenter.y;
    float cz = baseCenter.z;

    glm::vec3 apex(cx, cy + height, cz);

    auto tint = [](const glm::vec3& c, float f) {
        return glm::clamp(c * f, 0.0f, 1.0f);
    };

    glm::vec3 frontCol  = tint(baseColor, 1.05f);
    glm::vec3 rightCol  = tint(baseColor, 0.95f);
    glm::vec3 backCol   = tint(baseColor, 0.75f);
    glm::vec3 leftCol   = tint(baseColor, 0.85f);
    glm::vec3 bottomCol = tint(baseColor, 0.55f);

    glm::vec3 p0(cx - hx, cy, cz + hz); // Front-Left
    glm::vec3 p1(cx + hx, cy, cz + hz); // Front-Right
    glm::vec3 p2(cx + hx, cy, cz - hz); // Back-Right
    glm::vec3 p3(cx - hx, cy, cz - hz); // Back-Left

    // Bottom base quad (pointing down)
    addQuad(
        Vertex(p0, bottomCol),
        Vertex(p3, bottomCol),
        Vertex(p2, bottomCol),
        Vertex(p1, bottomCol)
    );

    // 4 Triangular side faces
    addTriangle(Vertex(p0, frontCol), Vertex(p1, frontCol), Vertex(apex, frontCol));
    addTriangle(Vertex(p1, rightCol), Vertex(p2, rightCol), Vertex(apex, rightCol));
    addTriangle(Vertex(p2, backCol),  Vertex(p3, backCol),  Vertex(apex, backCol));
    addTriangle(Vertex(p3, leftCol),  Vertex(p0, leftCol),  Vertex(apex, leftCol));
}

void Mesh::upload() {
    if (m_vertices.empty()) return;

    // Free existing GPU buffers if already allocated
    if (m_vao != 0) {
        destroy();
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    // Attribute 0: Position vec3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Attribute 1: Color vec3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_isUploaded = true;
}

void Mesh::draw() const {
    if (m_vao != 0 && m_isUploaded && !m_vertices.empty()) {
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
        glBindVertexArray(0);
    }
}

void Mesh::destroy() {
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    m_vertices.clear();
    m_isUploaded = false;
}
