#include "Camera.h"
#include <GLFW/glfw3.h> 

Camera::Camera() : 
    cameraPos(glm::vec3(0.0f, 0.0f, 3.0f)), 
    cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)), 
    cameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),    
    cameraSpeed(0.05f),
    lastX(0.0f), lastY(0.0f),
    yaw(-90.0f), pitch(0.0f), 
    firstMouse(true),
    sensitivity(0.1f),
    windowWidth(0), windowHeight(0),
    fov_(45.0f), aspect_(4.0f/3.0f), near_(0.1f), far_(100.0f),
    shader(nullptr)
{
}

void Camera::initialize(Shader* shader, int width, int height)
{
    this->shader = shader;
    this->windowWidth = width;
    this->windowHeight = height;
    lastX = width / 2.0f;
    lastY = height / 2.0f;
}

void Camera::update()
{
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    shader->setMat4("view", view);

    shader->setVec3("viewPos", cameraPos);

    glm::mat4 projection = glm::perspective(glm::radians(fov_), aspect_, near_, far_);
    shader->setMat4("projection", projection);
}

void Camera::setCameraPos(int key)
{
    if (key == GLFW_KEY_W)
        cameraPos += cameraSpeed * cameraFront;
    if (key == GLFW_KEY_S)
        cameraPos -= cameraSpeed * cameraFront;
    if (key == GLFW_KEY_A)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (key == GLFW_KEY_D)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void Camera::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

glm::vec3 Camera::getCameraPos() const
{
    return cameraPos;
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov_), aspect_, near_, far_);
}