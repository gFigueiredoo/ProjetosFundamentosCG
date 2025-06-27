#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h" 

class Mesh
{
public:
    Mesh() : VAO(0), nVertices(0), shader(nullptr), textureID(0), 
             position_(0.0f), rotation_angle_(0.0f), rotation_axis_(0.0f, 1.0f, 0.0f), scale_(1.0f),
             Ka(0.0f), Kd(0.0f), Ks(0.0f), Ns(0.0f) {}

    ~Mesh() {}
    void initialize(GLuint VAO, int nVertices, Shader* shader); 
    void update(bool rotateX, bool rotateY, bool rotateZ); 
    void draw(); 

    void setPosition(glm::vec3 pos) { position_ = pos; }
    void setRotation(float angle, glm::vec3 axis) { rotation_angle_ = angle; rotation_axis_ = axis; }
    void setScale(float s) { scale_ = s; }
    void setTextureID(GLuint id) { textureID = id; }
    void setMaterialProperties(glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float ns) {
        Ka = ka; Kd = kd; Ks = ks; Ns = ns;
    }
    void setCurrentPosition(glm::vec3 pos) { position_ = pos; } 
    glm::vec3 getPosition() const { return position_; } 
    
public: 
    GLuint VAO;
    float scale_; 
protected: 
    int nVertices;
    Shader* shader;
    GLuint textureID; 

    glm::vec3 position_;
    float rotation_angle_;
    glm::vec3 rotation_axis_;

    glm::vec3 Ka;
    glm::vec3 Kd;
    glm::vec3 Ks;
    float Ns;
};