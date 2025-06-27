#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h" 
#include <GLFW/glfw3.h> 

class Camera
{
public:
    Camera();

    void initialize(Shader* shader, int width, int height);
    void update();
    void setCameraPos(int key);
    void mouseCallback(GLFWwindow* window, double xpos, double ypos);

    void setCameraPosInitial(glm::vec3 pos) { cameraPos = pos; }
    void setCameraFrontInitial(glm::vec3 front) { cameraFront = front; }
    void setCameraUpInitial(glm::vec3 up) { cameraUp = up; }
    void setProjection(float fov, float aspect, float near, float far) { 
        fov_ = fov; aspect_ = aspect; near_ = near; far_ = far; 
    }


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
    float fov_;
    float aspect_;
    float near_;
    float far_;

    void updateCameraVectors();
};