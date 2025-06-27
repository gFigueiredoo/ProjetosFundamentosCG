#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"
#include "Bezier.h"

struct LightSourceConfig {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float intensity;
};

struct ObjectTransformConfig {
    glm::vec3 position;
    float rotation_angle;
    glm::vec3 rotation_axis;
    float scale;
};

struct ObjectAnimationConfig {
    std::string type;
    std::vector<glm::vec3> control_points;
    float speed;
    bool follow_trajectory;
};

struct ObjectConfig {
    std::string name;
    std::string obj_path;
    std::string mtl_path;
    std::string texture_path;
    ObjectTransformConfig initial_transform;
    ObjectAnimationConfig animation;
};

class Scene {
public:
    Scene();
    bool loadConfig(const std::string& configFilePath);
    void setupScene(GLFWwindow* window, Shader* shader, Camera* camera, std::vector<Mesh>& meshes, std::vector<Bezier>& bezierCurves);
    
    glm::vec3 cameraInitialPos;
    glm::vec3 cameraInitialFront;
    glm::vec3 cameraInitialUp;
    float cameraFov;
    float cameraAspectRatio;
    float cameraNearPlane;
    float cameraFarPlane;

    std::vector<LightSourceConfig> lightSources;
    std::vector<ObjectConfig> objects;

private:
    void loadMaterials(const std::string& mtlFilePath, glm::vec3& Ka, glm::vec3& Kd, glm::vec3& Ks, float& Ns);
    GLuint loadTexture(const std::string& filePath);
    int loadOBJ(const std::string& filePath, std::vector<GLfloat>& out_vertices, std::vector<GLfloat>& out_textures, std::vector<GLfloat>& out_normals);
    std::string basePath;
};