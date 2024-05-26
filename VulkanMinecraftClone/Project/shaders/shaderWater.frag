#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoords;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

const vec3 ambientColor = vec3(0.5, 0.4, 0.3); // Warm ambient light color

void main() {
    vec3 baseColor = texture(texSampler, fragTexCoords).rgb;
    vec3 finalColor = ambientColor * baseColor; // Water doesn't receive direct lighting
    outColor = vec4(finalColor, 0.5); // Adjust alpha for transparency
}

