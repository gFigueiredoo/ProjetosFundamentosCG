#include "Mesh.h"
#include <GLFW/glfw3.h>

void Mesh::initialize(GLuint VAO_in, int nVertices_in, Shader* shader_in)
{
    this->VAO = VAO_in;
    this->nVertices = nVertices_in;
    this->shader = shader_in;
}

// Update the model matrix based on internal state and rotation flags
void Mesh::update(bool rotateX, bool rotateY, bool rotateZ)
{
    glm::mat4 model = glm::mat4(1);
    
    model = glm::translate(model, position_);

    float current_angle = rotation_angle_; // Use initial angle as base

    if (rotateX)
    {
        current_angle += (GLfloat)glfwGetTime(); 
        model = glm::rotate(model, current_angle, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else if (rotateY)
    {
        current_angle += (GLfloat)glfwGetTime(); 
        model = glm::rotate(model, current_angle, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (rotateZ)
    {
        current_angle += (GLfloat)glfwGetTime(); 
        model = glm::rotate(model, current_angle, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else { // Apply initial rotation if no continuous rotation is selected
        model = glm::rotate(model, glm::radians(rotation_angle_), rotation_axis_);
    }
    
    model = glm::scale(model, glm::vec3(scale_, scale_, scale_));

    shader->setMat4("model", model);
}

void Mesh::draw()
{
    shader->setVec3("material.Ka", Ka);
    shader->setVec3("material.Kd", Kd);
    shader->setVec3("material.Ks", Ks);
    shader->setFloat("material.Ns", Ns);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, nVertices);
    glBindVertexArray(0);
}