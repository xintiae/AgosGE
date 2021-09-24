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


// ! Fragment and Vertex Shaders only shall access those
// * Common Descriptor Sets
// MVP generic matrix
layout(set = 0, binding = 0) uniform MVP {;
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

// Env. Light. (Environmental Light :P)
layout(set = 0, binding = 1) uniform EL {
    vec3 pos;
    vec3 color;
} envLight;

// ! Fragment Shader only shall access those
// * Per Model Specific Descriptor Sets
// Material
layout(set = 1, binding = 1) uniform Material {
    float  shininess;
    float  ior;        // index of refraction
    float  opacity;    // varies from 0 up to 1 
} material;

// Texture
layout(set = 2, binding = 0) uniform sampler2D TexSampler;
// Lighting Maps
// Ambiant Map
layout(set = 2, binding = 1) uniform sampler2D AmbSampler;
// Specular Map
layout(set = 2, binding = 2) uniform sampler2D SpecSampler;
// Diffuse Map
layout(set = 2, binding = 3) uniform sampler2D DiffSampler;
// Normal Map
layout(set = 2, binding = 4) uniform sampler2D NormalSampler;



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
