#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTextCoord;
layout(location = 3) in vec2 inAmbtCoord;
layout(location = 4) in vec2 inDiffCoord;
layout(location = 5) in vec2 inSpecCoord;
layout(location = 6) in vec2 inNrmlCoord;
layout(location = 7) in vec2 inAlphCoord;
layout(location = 8) in vec3 inNormal;


layout(location = 0) out vec3 fragColor;

layout(location = 1) out vec2 fragTextCoord;
layout(location = 2) out vec2 fragAmbtCoord;
layout(location = 3) out vec2 fragDiffCoord;
layout(location = 4) out vec2 fragSpecCoord;
layout(location = 5) out vec2 fragNrmlCoord;
layout(location = 6) out vec2 fragAlphCoord;

layout(location = 7) out vec3 fragNormal;
layout(location = 8) out vec3 fragView;
layout(location = 9) out vec3 fragLight;
layout(location = 10) out vec3 fragLightColor;


// ! Vertex Shader only shall access those
// * Common Descriptor Sets
// MVP generic matrix
layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

// ! Vertex and Fragment Shaders only shall access those
// Env. Light. (Environmental Light :P)
layout(set = 0, binding = 1) uniform EL {
    vec3 pos;
    vec3 color;
} envLight;

// ! Fragment Shader only shall access those
/*
// * Per Model Specific Descriptor Sets
// Material
layout(set = 1, binding = 0) uniform Material {
    float  shininess;
    float  ior;        // index of refraction
    float  opacity;    // varies from 0 up to 1 
} material;

// Texture
layout(set = 2, binding = 0) uniform sampler2D TextSampler;
// Lighting Maps
// Ambiant Map
layout(set = 2, binding = 1) uniform sampler2D AmbtSampler;
// Diffuse Map
layout(set = 2, binding = 2) uniform sampler2D DiffSampler;
// Specular Map
layout(set = 2, binding = 3) uniform sampler2D SpecSampler;
// Normal Map
layout(set = 2, binding = 4) uniform sampler2D NrmlSampler;
*/


void main()
{
    gl_Position     = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);

/*
    fragColor       = inColor;
    fragTextCoord   = inTextCoord;
    fragNormal      = inNormal;
    fragView        = vec3(1.0f);
    fragLight       = envLight.pos;
    fragLightColor  = envLight.color;
*/

    fragColor       = inColor * envLight.color;

    fragTextCoord   = inTextCoord;
    fragAmbtCoord   = inAmbtCoord;
    fragDiffCoord   = inDiffCoord;
    fragSpecCoord   = inSpecCoord;
    fragNrmlCoord   = inNrmlCoord;
    fragAlphCoord   = inAlphCoord;


    fragNormal      = mat3(mvp.model) * inNormal;

    vec4 worldPos   = mvp.model * vec4(inPosition, 1.0);
    fragView        = (mvp.view * worldPos).xyz;
    fragLight       = (envLight.pos - vec3(worldPos));
    fragLightColor  = envLight.color;
}
