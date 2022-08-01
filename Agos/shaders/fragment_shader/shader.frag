#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 1) in vec2 fragTextCoord;
layout(location = 2) in vec2 fragAmbtCoord;
layout(location = 3) in vec2 fragDiffCoord;
layout(location = 4) in vec2 fragSpecCoord;
layout(location = 5) in vec2 fragNrmlCoord;
layout(location = 6) in vec2 fragAlphCoord;

layout(location = 7) in vec3 fragNormal;
layout(location = 8) in vec3 fragView;
layout(location = 9) in vec3 fragLight;
layout(location = 10) in vec3 fragLightColor;

layout(location = 0) out vec4 outColor;

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


void main()
{
    // Phong-Shading computation
    // vec3 normal     = normalize (fragNormal     );
    vec3 normal     = texture(NrmlSampler, fragNrmlCoord).xyz;
    vec3 light      = normalize (fragLight      );
    vec3 view       = normalize (fragView       );
    vec3 reflection = reflect   (light, normal  );

    // ambient
    vec3 ambient = texture(AmbtSampler, fragAmbtCoord).xyz * fragColor;

    // diffuse
    vec3 diffuse = max(dot(normal, light), 0.0) * texture(DiffSampler, fragDiffCoord).xyz * fragColor;

    // specular
    float spec = pow(max(dot(reflection, view), 0.0), material.shininess);
    vec3 specular = texture(SpecSampler, fragSpecCoord).xyz * spec * fragLightColor;// * vec3(1.35f);// * mvp.lightColor;

    // vec3 ambient = vec3(0.0);
    // vec3 diffuse = vec3(0.0);
    // vec3 specular = vec3(0.0);

    // outColor = vec4( (ambient + diffuse + specular) * fragColor, 1.0);
    // outColor = vec4( vec4(ambient + diffuse + specular, 1.0) * texture(TextSampler, fragTextCoord) );
    outColor = texture(TextSampler, fragTextCoord) * vec4(envLight.color, 1.0);
    // outColor = texture(TextSampler, fragTextCoord) * vec4(1.0, 1.0, 1.0, 1.0);
}
