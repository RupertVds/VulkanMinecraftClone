#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {
    ivec3 translation;
    float time;
} mesh;

// Constant offset to lower the water faces
const float waterOffset = -0.15; // Adjust this value as needed

void main() 
{
    // Construct translation matrix
    mat4 translationMatrix = mat4(1.0); // Identity matrix
    translationMatrix[3].xyz = mesh.translation; // Set translation part

    // Define the displacement factor for the sine wave, using time to animate
    // Use global position for consistent displacement across adjacent faces
    float displacementFactor = sin(mesh.time * 2.0 + inPosition.x * 0.5 + inPosition.z * 0.5);

    // Displace the vertex position along the y-axis based on the sine wave
    // Add the constant offset to lower the water faces
    vec3 displacedPosition = inPosition + vec3(0.0, waterOffset + displacementFactor * 0.1, 0.0);

    // Apply transformations and pass to output
    gl_Position = ubo.proj * ubo.view * translationMatrix * vec4(displacedPosition, 1.0);
    fragColor = inNormal;
    fragTexCoord = inTexCoord;
}
