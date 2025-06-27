#include "Scene.h"
#include <iostream>
#include <fstream>
#include <sstream>
#define TINYOBJLOADER_IMPLEMENTATION 
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Scene::Scene() : basePath("../assets/") {}

bool Scene::loadConfig(const std::string& configFilePath) {
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open scene configuration file: " << configFilePath << std::endl;
        return false;
    }

    nlohmann::json jsonConfig;
    try {
        file >> jsonConfig;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }

    if (jsonConfig.contains("camera")) {
        const auto& cam = jsonConfig["camera"];
        cameraInitialPos   = glm::vec3(cam["position"][0], cam["position"][1], cam["position"][2]);
        cameraInitialFront = glm::vec3(cam["front"][0], cam["front"][1], cam["front"][2]);
        cameraInitialUp    = glm::vec3(cam["up"][0], cam["up"][1], cam["up"][2]);
        cameraFov          = cam["fov"];
        cameraAspectRatio  = cam["aspect_ratio"];
        cameraNearPlane    = cam["near_plane"];
        cameraFarPlane     = cam["far_plane"];
    }

    if (jsonConfig.contains("light_sources")) {
        for (const auto& light : jsonConfig["light_sources"]) {
            lightSources.push_back({
                glm::vec3(light["position"][0], light["position"][1], light["position"][2]),
                glm::vec3(light["ambient"][0], light["ambient"][1], light["ambient"][2]),
                glm::vec3(light["diffuse"][0], light["diffuse"][1], light["diffuse"][2]),
                glm::vec3(light["specular"][0], light["specular"][1], light["specular"][2]),
                light["intensity"]
            });
        }
    }

    if (jsonConfig.contains("objects")) {
        for (const auto& obj : jsonConfig["objects"]) {
            ObjectConfig objectConfig;
            objectConfig.name = obj["name"];
            objectConfig.obj_path = basePath + obj["obj_path"].get<std::string>();
            objectConfig.mtl_path = basePath + obj["mtl_path"].get<std::string>();
            objectConfig.texture_path = basePath + obj["texture_path"].get<std::string>();

            const auto& transform = obj["initial_transform"];
            objectConfig.initial_transform = {
                glm::vec3(transform["position"][0], transform["position"][1], transform["position"][2]),
                transform["rotation_angle"],
                glm::vec3(transform["rotation_axis"][0], transform["rotation_axis"][1], transform["rotation_axis"][2]),
                transform["scale"]
            };

            if (obj.contains("animation")) {
                const auto& anim = obj["animation"];
                objectConfig.animation.type = anim["type"];
                if (anim.contains("control_points")) {
                    for (const auto& cp : anim["control_points"]) {
                        objectConfig.animation.control_points.push_back(glm::vec3(cp[0], cp[1], cp[2]));
                    }
                }
                objectConfig.animation.speed = anim.value("speed", 0.01f);
                objectConfig.animation.follow_trajectory = anim.value("follow_trajectory", false);
            } else {
                objectConfig.animation.type = "none";
            }
            objects.push_back(objectConfig);
        }
    }

    return true;
}

void Scene::loadMaterials(const std::string& mtlFilePath, glm::vec3& Ka, glm::vec3& Kd, glm::vec3& Ks, float& Ns) {
    std::ifstream mtlFile(mtlFilePath);
    if (!mtlFile.is_open()) {
        std::cerr << "Erro ao abrir arquivo MTL: " << mtlFilePath << std::endl;
        Ka = glm::vec3(0.1f, 0.1f, 0.1f);
        Kd = glm::vec3(0.7f, 0.7f, 0.7f);
        Ks = glm::vec3(1.0f, 1.0f, 1.0f);
        Ns = 32.0f;
        return;
    }

    std::string line;
    while (std::getline(mtlFile, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "Ka") {
            iss >> Ka.x >> Ka.y >> Ka.z;
        } else if (prefix == "Kd") {
            iss >> Kd.x >> Kd.y >> Kd.z;
        } else if (prefix == "Ks") {
            iss >> Ks.x >> Ks.y >> Ks.z;
        } else if (prefix == "Ns") {
            iss >> Ns;
        }
    }
    mtlFile.close();
}

GLuint Scene::loadTexture(const std::string& filePath) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;
        else {
            std::cerr << "Unsupported number of channels for texture: " << filePath << std::endl;
            stbi_image_free(data);
            return 0;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

int Scene::loadOBJ(const std::string& filePath, std::vector<GLfloat>& out_vertices, std::vector<GLfloat>& out_textures, std::vector<GLfloat>& out_normals) {
    tinyobj::ObjReaderConfig reader_config;
    size_t last_slash_idx = filePath.rfind('/');
    if (std::string::npos == last_slash_idx) {
        last_slash_idx = filePath.rfind('\\');
    }
    std::string obj_dir = "";
    if (std::string::npos != last_slash_idx) {
        obj_dir = filePath.substr(0, last_slash_idx);
    }
    reader_config.mtl_search_path = obj_dir;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filePath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        return -1;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib(); 
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    out_vertices.clear();
    out_textures.clear();
    out_normals.clear();

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                out_vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                out_vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                out_vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                if (idx.normal_index >= 0) {
                    out_normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    out_normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    out_normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
                } else {
                    out_normals.push_back(0.0f); out_normals.push_back(0.0f); out_normals.push_back(0.0f);
                }

                if (idx.texcoord_index >= 0) {
                    out_textures.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                    out_textures.push_back(1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]);
                } else {
                    out_textures.push_back(0.0f); out_textures.push_back(0.0f);
                }
            }
            index_offset += fv;
        }
    }
    return out_vertices.size() / 3;
}


void Scene::setupScene(GLFWwindow* window, Shader* shader, Camera* camera, std::vector<Mesh>& meshes, std::vector<Bezier>& bezierCurves) {
    camera->setCameraPosInitial(cameraInitialPos);
    camera->setCameraFrontInitial(cameraInitialFront);
    camera->setCameraUpInitial(cameraInitialUp);
    camera->setProjection(cameraFov, cameraAspectRatio, cameraNearPlane, cameraFarPlane);

    if (!lightSources.empty()) {
        shader->Use();
        shader->setVec3("light.position", lightSources[0].position);
        shader->setVec3("light.ambient", lightSources[0].ambient);
        shader->setVec3("light.diffuse", lightSources[0].diffuse);
        shader->setVec3("light.specular", lightSources[0].specular);
    }

    for (const auto& objConfig : objects) {
        std::vector<GLfloat> obj_vertices;
        std::vector<GLfloat> obj_texcoords;
        std::vector<GLfloat> obj_normals;
        int nVertices;

        nVertices = loadOBJ(objConfig.obj_path, obj_vertices, obj_texcoords, obj_normals);
        if (nVertices == -1) {
            std::cerr << "Error loading OBJ: " << objConfig.obj_path << std::endl;
            continue;
        }

        glm::vec3 Ka, Kd, Ks;
        float Ns;
        loadMaterials(objConfig.mtl_path, Ka, Kd, Ks, Ns);

        GLuint objTexID = loadTexture(objConfig.texture_path);

        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        std::vector<GLfloat> interleaved_data;
        interleaved_data.reserve(nVertices * 8); 

        for (int i = 0; i < nVertices; ++i) {
            interleaved_data.push_back(obj_vertices[i * 3 + 0]);
            interleaved_data.push_back(obj_vertices[i * 3 + 1]);
            interleaved_data.push_back(obj_vertices[i * 3 + 2]);
            interleaved_data.push_back(obj_normals[i * 3 + 0]);
            interleaved_data.push_back(obj_normals[i * 3 + 1]);
            interleaved_data.push_back(obj_normals[i * 3 + 2]);
            interleaved_data.push_back(obj_texcoords[i * 2 + 0]);
            interleaved_data.push_back(obj_texcoords[i * 2 + 1]);
        }
        
        glBufferData(GL_ARRAY_BUFFER, interleaved_data.size() * sizeof(GLfloat), interleaved_data.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        Mesh mesh;
        mesh.initialize(VAO, nVertices, shader);
        mesh.setPosition(objConfig.initial_transform.position);
        mesh.setRotation(objConfig.initial_transform.rotation_angle, objConfig.initial_transform.rotation_axis);
        mesh.setScale(objConfig.initial_transform.scale);
        mesh.setTextureID(objTexID);
        mesh.setMaterialProperties(Ka, Kd, Ks, Ns);
        meshes.push_back(mesh);

        if (objConfig.animation.type == "bezier" && objConfig.animation.control_points.size() >= 4) {
            Bezier bezier;
            bezier.setShader(shader);
            bezier.setControlPoints(objConfig.animation.control_points);
            bezier.generateCurve(100);
            bezier.setSpeed(objConfig.animation.speed);
            bezier.setFollowTrajectory(objConfig.animation.follow_trajectory);
            bezierCurves.push_back(bezier);
        } else {
            Bezier bezier;
            bezier.setFollowTrajectory(false);
            bezierCurves.push_back(bezier);
        }
    }
}