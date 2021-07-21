#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 3) in vec3 fragView;
layout(location = 4) in vec3 fragLight;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    // outColor = vec4(fragColor, 1.0f);
    // outColor = texture(texSampler, fragTexCoord);

    // Phong-Shading computation
    vec3 normal     = normalize(fragNormal);
    vec3 light      = normalize(fragLight);
    vec3 view       = normalize(fragView);
    vec3 reflection = reflect(light, normal);

    vec3 ambient = (fragColor * 0.125) - 0.135;
    vec3 diffuse = max(dot(normal, light), 0.0) * fragColor * (1/pow(2, 15));
    vec3 specular = pow(max(dot(reflection, view), 0.0), 20.0) * vec3(1.5);

    // vec3 ambient = vec3(0.0);
    // vec3 diffuse = vec3(0.0);
    // vec3 specular = vec3(0.0);

    // outColor = vec4( ambient + diffuse + specular, 1.0);
    outColor = vec4( vec4(ambient + diffuse + specular, 1.0) + texture(texSampler, fragTexCoord) );
}
