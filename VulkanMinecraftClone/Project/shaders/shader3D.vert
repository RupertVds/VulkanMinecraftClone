#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
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
} mesh;


void main() 
{
    // Construct translation matrix
    mat4 translationMatrix = mat4(1.0); // Identity matrix
    translationMatrix[3].xyz = mesh.translation; // Set translation part
    //translationMatrix[3].xyz = mesh.model[3].xyz; // Set translation part

    gl_Position = ubo.proj * ubo.view * translationMatrix  * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}