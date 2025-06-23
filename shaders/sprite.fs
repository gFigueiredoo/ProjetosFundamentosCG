#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D tex_buffer;

uniform vec3 viewPos; // Posição da câmera (observador) no espaço do mundo

struct Material {
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Ns;
};
uniform Material material;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    // 1. Componente Ambiente
    vec3 ambient = light.ambient * material.Ka;

    // 2. Componente Difusa
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.Kd);

    // 3. Componente Especular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = normalize(reflect(-lightDir, norm));

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Ns);
    vec3 specular = light.specular * (spec * material.Ks);

    vec4 texColor = texture(tex_buffer, TexCoord);

    FragColor = texColor * vec4(ambient + diffuse + specular, 1.0);
}