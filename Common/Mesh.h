#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h" 

class Mesh
{
public:
    Mesh() {}
    ~Mesh() {}
    void initialize(GLuint VAO, int nVertices, Shader* shader); // Simplificado o init
    void update(glm::vec3 position, bool rotateX, bool rotateY, bool rotateZ, float scale_val);
    void draw(GLuint textureID);

protected:
    GLuint VAO;
    int nVertices;
    Shader* shader;
};