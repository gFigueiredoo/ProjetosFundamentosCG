#include "Curve.h"
#include <glad/glad.h>

Curve::Curve() : VAO_Curve(0), VBO_Curve(0), shader(nullptr)
{
}

void Curve::setShader(Shader* shader_in)
{
    this->shader = shader_in;
}

void Curve::setupCurveGeometry()
{
    if (curvePoints.empty()) return;

    if (VAO_Curve != 0) glDeleteVertexArrays(1, &VAO_Curve);
    if (VBO_Curve != 0) glDeleteBuffers(1, &VBO_Curve);

    glGenVertexArrays(1, &VAO_Curve);
    glGenBuffers(1, &VBO_Curve);

    glBindVertexArray(VAO_Curve);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Curve);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Curve::drawCurve(glm::vec4 color)
{
    if (curvePoints.empty() || VAO_Curve == 0) return;

    if (!shader) {
        std::cerr << "Curve shader not set!" << std::endl;
        return;
    }
    shader->Use();
    shader->setVec4("colorOverride", color.r, color.g, color.b, color.a); 

    glm::mat4 identityModel = glm::mat4(1.0f);
    shader->setMat4("model", identityModel); 

    glBindVertexArray(VAO_Curve);
    glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());
    glDrawArrays(GL_POINTS, 0, curvePoints.size()); 
    glBindVertexArray(0);
}