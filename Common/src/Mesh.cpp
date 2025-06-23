#include "Mesh.h"
#include <GLFW/glfw3.h>

void Mesh::initialize(GLuint VAO_in, int nVertices_in, Shader* shader_in)
{
    this->VAO = VAO_in;
    this->nVertices = nVertices_in;
    this->shader = shader_in;
}

void Mesh::update(glm::vec3 position, bool rotateX, bool rotateY, bool rotateZ, float scale_val)
{
    glm::mat4 model = glm::mat4(1);
    
    // Aplica a translação recebida
    model = glm::translate(model, position);

    // Aplica rotação com base no tempo
    float angle = (GLfloat)glfwGetTime(); 

    if (rotateX)
    {
        model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else if (rotateY)
    {
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (rotateZ)
    {
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    // Aplica a escala recebida
    model = glm::scale(model, glm::vec3(scale_val, scale_val, scale_val));

    shader->setMat4("model", model);
}

void Mesh::draw(GLuint textureID)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, nVertices);
    glBindVertexArray(0);
}