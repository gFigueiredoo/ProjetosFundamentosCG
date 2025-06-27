#version 330 core
out vec4 FragColor;

struct Material {
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Ns;
}; 

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

void main()
{    
    // Ambient
    vec3 ambient = light.ambient * material.Ka;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.Kd);

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Ns);
    vec3 specular = light.specular * (spec * material.Ks);  

    vec3 result = (ambient + diffuse + specular) * texture(texture_diffuse1, TexCoord).rgb;
    FragColor = vec4(result, 1.0);
}