#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragView;
layout(location = 4) in vec3 fragLight;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D texSampler;

void main()
{
    outColor = texture(texSampler, fragTexCoord);
}