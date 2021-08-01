#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragView;
layout(location = 4) in vec3 fragLight;

layout(location = 0) out vec4 outColor;

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

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    // Phong-Shading computation
    vec3 normal     = normalize(fragNormal);
    vec3 light      = normalize(fragLight);
    vec3 view       = normalize(fragView);
    vec3 reflection = reflect(light, normal);

    // ambient
    vec3 ambient = ubo.ambient * fragColor;

    // diffuse
    vec3 diffuse = max(dot(normal, light), 0.0) * ubo.diffuse * fragColor;

    // specular
    float spec = pow(max(dot(reflection, view), 0.0), ubo.shininess);
    vec3 specular = ubo.specular * spec;// * vec3(1.35f);// * ubo.lightColor;

    // vec3 ambient = vec3(0.0);
    // vec3 diffuse = vec3(0.0);
    // vec3 specular = vec3(0.0);

    // outColor = vec4( (ambient + diffuse + specular) * fragColor, 1.0);
    outColor = vec4( vec4(ambient + diffuse + specular, 1.0) * texture(texSampler, fragTexCoord) );
}
