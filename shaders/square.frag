#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 sunDir;
layout(location = 3) in vec3 sunCol;

layout(location = 0) out vec4 outColor;

void main() {
    // Ambient light
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * sunCol;

    // Diffuse light
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(-sunDir); // sunDir is direction *from* sun
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sunCol;

    vec3 result = (ambient + diffuse) * fragColor;
    outColor = vec4(result, 1.0);
}
