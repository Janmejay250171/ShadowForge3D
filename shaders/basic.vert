#version 330 core

// Vertex attributes from VBO
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

// Output to fragment shader
out vec3 vColor;
out vec3 vFragPos;

// Transformation matrices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    // Compute world-space position
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    gl_Position = uProjection * uView * worldPos;
    
    // Pass interpolated vertex color and world position to fragment shader
    vColor = aColor;
    vFragPos = worldPos.xyz;
}

