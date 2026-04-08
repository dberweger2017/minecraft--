#version 450

layout(push_constant) uniform PushConstants {
  vec2 offset;
  vec2 scale;
} pushConstants;

layout(location = 0) out vec3 fragColor;

void main() {
  const vec2 positions[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0)
  );

  const vec3 colors[6] = vec3[](
    vec3(0.99, 0.75, 0.25),
    vec3(0.96, 0.43, 0.26),
    vec3(0.87, 0.26, 0.35),
    vec3(0.99, 0.75, 0.25),
    vec3(0.87, 0.26, 0.35),
    vec3(0.53, 0.79, 0.91)
  );

  gl_Position = vec4(positions[gl_VertexIndex] * pushConstants.scale + pushConstants.offset, 0.0, 1.0);
  fragColor = colors[gl_VertexIndex];
}
