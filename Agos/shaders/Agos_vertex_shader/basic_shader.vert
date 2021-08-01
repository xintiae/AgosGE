#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;


layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragView;
layout(location = 4) out vec3 fragLight;


layout(binding = 0) uniform ModelViewProjectionBufferObject {
    // mvp
    mat4 model;
    mat4 view;
    mat4 proj;
    // light
    vec3 lightPos;
    vec3 lightColor;
    // material
    vec3    ambient;
    vec3    diffuse;
    vec3    specular;
    float   shininess;
} ubo;


void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor * ubo.lightColor;
    fragTexCoord = inTexCoord;

    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);

    fragNormal = mat3(ubo.model) * inNormal;
    fragView = (ubo.view * worldPos).xyz;
    fragLight = (ubo.lightPos - vec3(worldPos));
}
