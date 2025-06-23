#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h" 
#include <GLFW/glfw3.h> // <--- Adicione esta linha AQUI

class Camera
{
public:
    Camera();

    void initialize(Shader* shader, int width, int height);
    void update();
    void setCameraPos(int key);
    void mouseCallback(GLFWwindow* window, double xpos, double ypos);

    glm::vec3 getCameraPos() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

private:
    Shader* shader;
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float cameraSpeed;

    float lastX, lastY;
    float yaw;
    float pitch;
    bool firstMouse;
    float sensitivity;

    int windowWidth;
    int windowHeight;

    void updateCameraVectors();
};