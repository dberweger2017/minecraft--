#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 sunDir;
layout(location = 3) in vec3 sunCol;
layout(location = 4) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    // Ambient light
    float ambientStrength = 0.8;
    vec3 ambient = ambientStrength * sunCol;

    // Diffuse light
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(-sunDir); // sunDir is direction *from* sun
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sunCol;

    vec4 baseColor = vec4(ambient + diffuse, 1.0) * fragColor;

    // Draw a black outline based on UV coordinates
    float thickness = 0.05;
    if (fragUV.x < thickness || fragUV.x > (1.0 - thickness) ||
        fragUV.y < thickness || fragUV.y > (1.0 - thickness)) {
        baseColor.rgb *= 0.0; // Black border
    }

    outColor = baseColor;
}
