#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D tex_buffer;

struct Material {
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Ns;
};
uniform Material material;

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};

uniform PointLight keyLight;
uniform PointLight fillLight;
uniform PointLight backLight;

uniform vec3 viewPos;

vec3 calculateLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    if (!light.enabled) {
        return vec3(0.0);
    }

    vec3 ambient = light.ambient * material.Ka;

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.Kd);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Ns);
    vec3 specular = light.specular * (spec * material.Ks);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}


void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    result += calculateLight(keyLight, norm, FragPos, viewDir);
    result += calculateLight(fillLight, norm, FragPos, viewDir);
    result += calculateLight(backLight, norm, FragPos, viewDir);

    FragColor = vec4(result, 1.0) * texture(tex_buffer, TexCoords);
}