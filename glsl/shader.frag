#version 330 core

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

struct Light {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define LIGHT_COUNT 1

in vec2 TexCoords;
in vec3 FragPosition;
in vec3 Normal;

out vec4 color;

uniform vec3 viewPosition;
uniform Light lights[LIGHT_COUNT];
uniform Material material;

vec3 CalcLight(Light light, Material mat, vec3 normal, vec3 fragPosition, vec3 viewDirection)
{
    vec3 lightDir = normalize(light.position - FragPosition);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), mat.shininess);
    // Attenuation
    float distance = length(light.position - FragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // Combine results
    vec3 ambient = light.ambient * vec3(texture(mat.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(mat.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(mat.texture_specular1, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{
    vec3 result;
    vec3 viewDirection = normalize(viewPosition - FragPosition);
    vec3 normal = normalize(Normal);

    for(int i = 0; i < LIGHT_COUNT; i++)
        result += CalcLight(lights[i], material, normal, FragPosition, viewDirection);

    color = vec4(result, 1.0);
}