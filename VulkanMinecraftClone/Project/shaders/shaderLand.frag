#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoords;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

const vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5)); // Example light direction
const vec3 ambientColor = vec3(0.5, 0.4, 0.3); // Warm ambient light color

void main() {
    vec3 normal = normalize(fragNormal);
    float lightIntensity = max(dot(normal, lightDir), 0.0);
    vec3 baseColor = texture(texSampler, fragTexCoords).rgb;
    vec3 litColor = baseColor * lightIntensity;
    vec3 finalColor = ambientColor * baseColor + litColor;
    outColor = vec4(finalColor, 1.0);
}
