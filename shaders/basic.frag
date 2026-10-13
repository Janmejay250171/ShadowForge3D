#version 330 core

// Interpolated vertex attributes
in vec3 vColor;
in vec3 vFragPos;

// Final output color to the framebuffer
out vec4 FragColor;

// Rendering Mode: 0 = Unlit (for 2D silhouettes, HUD, UI), 1 = Lit 3D geometry
uniform int uRenderMode;
uniform float uAlpha;

void main()
{
    if (uRenderMode == 1) {
        // Compute faceted face normal using screen-space partial derivatives
        vec3 normal = normalize(cross(dFdx(vFragPos), dFdy(vFragPos)));
        if (!gl_FrontFacing) {
            normal = -normal;
        }

        // Clean technical laboratory dual-light rig:
        // Key light: crisp cool white from top-front-right
        vec3 keyDir = normalize(vec3(0.5, 0.9, 0.7));
        float diffKey = max(dot(normal, keyDir), 0.0);

        // Fill light: subtle cool tint from bottom-back-left
        vec3 fillDir = normalize(vec3(-0.5, -0.2, -0.6));
        float diffFill = max(dot(normal, fillDir), 0.0) * 0.25;

        // Ambient base
        float ambient = 0.42;

        vec3 litColor = vColor * (ambient + diffKey * 0.50 + diffFill);
        FragColor = vec4(litColor, 1.0);
    } else {
        // Direct unlit rendering for 2D UI and silhouettes with alpha support
        float alpha = (uAlpha > 0.0001) ? uAlpha : 1.0;
        FragColor = vec4(vColor, alpha);
    }
}

