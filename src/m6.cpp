#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <random>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Bezier.h"

vector<GLfloat> totalvertices;

glm::vec3 Ka;
glm::vec3 Kd;
glm::vec3 Ks;
float Ns;

string mtlFilePath = "";
string textureFilePath = "";
string basePath = "../assets/";

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;

float objectScale = 0.5f;

int verticesToDraw = 0;

vector<glm::vec3> trajectoryPoints;
int currentTrajectoryPointIndex = 0;
float trajectoryProgress = 0.0f;
float trajectorySpeed = 0.005f;

glm::vec3 currentObjectPosition;
bool followTrajectory = false;

Camera camera;
Mesh objectMesh;
Bezier bezierCurve;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void setupWindow(GLFWwindow*& window);
void setupShader(Shader& shader);
void readFromMtl(string path);
int setupGeometry();
int loadTexture(string path);
void readFromObj(string path);
void saveTrajectoryPoints(const string& filename);
void loadTrajectoryPoints(const string& filename);
void updateObjectPositionFromInput(int key);

void setupShader(Shader& shader) {
    shader.setVec3("material.Ka", Ka);
    shader.setVec3("material.Kd", Kd);
    shader.setVec3("material.Ks", Ks);
    shader.setFloat("material.Ns", Ns);

    shader.setVec3("light.position", 1.0f, 1.0f, 1.0f);
    shader.setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
    shader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
}

void readFromMtl(string path)
{
    string line, readValue;
    ifstream mtlFile(path);

    if (!mtlFile.is_open()) {
        cerr << "Erro ao abrir arquivo MTL: " << path << endl;
        return;
    }

    Ka = glm::vec3(0.1f, 0.1f, 0.1f);
    Kd = glm::vec3(0.7f, 0.7f, 0.7f);
    Ks = glm::vec3(1.0f, 1.0f, 1.0f);
    Ns = 32.0f;

    while (!mtlFile.eof())
    {
        getline(mtlFile, line);
        istringstream iss(line);
        string prefix;
        iss >> prefix;

        if (prefix == "map_Kd")
        {
            iss >> textureFilePath;
        }
        else if (prefix == "Ka")
        {
            iss >> Ka.x >> Ka.y >> Ka.z;
        }
        else if (prefix == "Kd")
        {
            iss >> Kd.x >> Kd.y >> Kd.z;
        }
        else if (prefix == "Ks")
        {
            iss >> Ks.x >> Ks.y >> Ks.z;
        }
        else if (prefix == "Ns")
        {
            iss >> Ns;
        }
    }
    mtlFile.close();
}

int setupGeometry()
{
    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, totalvertices.size() * sizeof(GLfloat), totalvertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    return VAO;
}

void readFromObj(string path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Falha ao abrir o arquivo OBJ: " << path << std::endl;
        return;
    }

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_textures;
    std::vector<glm::vec3> temp_normais;

    std::string line;

    while (std::getline(file, line)) {
        if (line.length() > 0) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                glm::vec3 values;
                iss >> values.x >> values.y >> values.z;
                temp_vertices.push_back(values);
            }
            else if (prefix == "vt")
            {
                glm::vec2 values;
                iss >> values.x >> values.y;
                temp_textures.push_back(values);
            }
            else if (prefix == "vn")
            {
                glm::vec3 values;
                iss >> values.x >> values.y >> values.z;
                temp_normais.push_back(values);
            }
            else if (prefix == "f")
            {
                for (int i = 0; i < 3; ++i)
                {
                    unsigned int vertexIndex, textIndex, normalIndex;
                    char slash;

                    iss >> vertexIndex >> slash >> textIndex >> slash >> normalIndex;

                    totalvertices.push_back(temp_vertices[vertexIndex - 1].x);
                    totalvertices.push_back(temp_vertices[vertexIndex - 1].y);
                    totalvertices.push_back(temp_vertices[vertexIndex - 1].z);

                    totalvertices.push_back(temp_textures[textIndex - 1].x);
                    totalvertices.push_back(1.0f - temp_textures[textIndex - 1].y); 

                    totalvertices.push_back(temp_normais[normalIndex - 1].x);
                    totalvertices.push_back(temp_normais[normalIndex - 1].y);
                    totalvertices.push_back(temp_normais[normalIndex - 1].z);
                }
            } 
            else if (prefix == "mtllib")
            {
                iss >> mtlFilePath;
            }
        }
    }

    file.close();

    verticesToDraw = totalvertices.size() / 8; 
}

int loadTexture(string path)
{
    GLuint texID;

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Falha ao carregar textura: " << path << endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.mouseCallback(window, xpos, ypos);
}

void updateObjectPositionFromInput(int key) {
    float moveSpeed = 0.1f;
    if (key == GLFW_KEY_UP)
        currentObjectPosition.z -= moveSpeed;
    if (key == GLFW_KEY_DOWN)
        currentObjectPosition.z += moveSpeed;
    if (key == GLFW_KEY_LEFT)
        currentObjectPosition.x -= moveSpeed;
    if (key == GLFW_KEY_RIGHT)
        currentObjectPosition.x += moveSpeed;
    if (key == GLFW_KEY_PAGE_UP)
        currentObjectPosition.y += moveSpeed;
    if (key == GLFW_KEY_PAGE_DOWN)
        currentObjectPosition.y -= moveSpeed;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float scaleIncrement = 0.1f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = true;

    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        objectScale -= scaleIncrement;
        if (objectScale < 0.1f) objectScale = 0.1f;
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        objectScale += scaleIncrement;
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        trajectoryPoints.clear();
        currentTrajectoryPointIndex = 0;
        currentObjectPosition = glm::vec3(0.0f); 
        followTrajectory = false;
        cout << "Trajetoria limpa." << endl;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { 
        trajectoryPoints.push_back(camera.getCameraPos()); 
        cout << "Ponto de controle adicionado: (" << camera.getCameraPos().x << ", " 
             << camera.getCameraPos().y << ", " << camera.getCameraPos().z << ")" << endl;
        bezierCurve.setControlPoints(trajectoryPoints);
        if (trajectoryPoints.size() >= 4) { 
            bezierCurve.generateCurve(100); 
        }
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        followTrajectory = !followTrajectory;
        if (followTrajectory && !trajectoryPoints.empty()) {
            currentTrajectoryPointIndex = 0; 
            trajectoryProgress = 0.0f; 
            cout << "Trajetoria " << (followTrajectory ? "ativada." : "desativada.") << endl;
        } else if (followTrajectory && trajectoryPoints.empty()) {
            cout << "Erro: Nao ha pontos na trajetoria para ativar." << endl;
            followTrajectory = false;
        } else {
            cout << "Trajetoria " << (followTrajectory ? "ativada." : "desativada.") << endl;
        }
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        loadTrajectoryPoints(basePath + "trajectories/trajectory_points.txt");
        bezierCurve.setControlPoints(trajectoryPoints);
        if (trajectoryPoints.size() >= 4) {
            bezierCurve.generateCurve(100);
        }
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        saveTrajectoryPoints(basePath + "trajectories/trajectory_points.txt");
    }

    camera.setCameraPos(key); 
    if (!followTrajectory) { 
        updateObjectPositionFromInput(key); 
    }
}

void saveTrajectoryPoints(const string& filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Erro ao abrir arquivo para salvar: " << filename << endl;
        return;
    }
    for (const auto& point : trajectoryPoints) {
        outFile << point.x << ", " << point.y << ", " << point.z << endl;
        cout << "Ponto salvo no arquivo: " << point.x << ", " << point.y << ", " << point.z << endl; 
    }
    outFile.close();
    cout << "Trajetoria salva em: " << filename << endl;
}

void loadTrajectoryPoints(const string& filename) {
    trajectoryPoints.clear();
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Erro ao abrir arquivo para carregar: " << filename << endl;
        return;
    }
    string line;
    while (getline(inFile, line)) {
        stringstream ss(line);
        float x, y, z;
        char comma;
        if (ss >> x >> comma >> y >> comma >> z) {
            trajectoryPoints.push_back(glm::vec3(x, y, z));
        }
    }
    inFile.close();
    if (!trajectoryPoints.empty()) {
        currentTrajectoryPointIndex = 0;
        trajectoryProgress = 0.0f;
        currentObjectPosition = trajectoryPoints[0];
        followTrajectory = false;
        cout << "Trajetoria carregada de: " << filename << ". Total de pontos: " << trajectoryPoints.size() << endl;
    } else {
        cout << "Arquivo de trajetoria vazio ou invalido: " << filename << endl;
    }
}

void setupWindow(GLFWwindow*& window) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Modulo 6 - Gabriel", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback); 

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;
}


int main()
{
    GLFWwindow* window;

    setupWindow(window);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height); 
    glViewport(0, 0, width, height);

    Shader shader("../shaders/sprite.vs", "../shaders/sprite.fs");

    readFromObj(basePath + "Modelos3D/Suzanne.obj"); 
    readFromMtl(basePath + "Modelos3D/Suzanne.mtl"); 
    GLuint texID = loadTexture(basePath + "Modelos3D/Suzanne.png"); 
    
    GLuint VAO = setupGeometry();

    glUseProgram(shader.ID);

    shader.setInt("tex_buffer", 0); 

    camera.initialize(&shader, WINDOW_WIDTH, WINDOW_HEIGHT);
    bezierCurve.setShader(&shader); 

    objectMesh.initialize(VAO, verticesToDraw, &shader);
    currentObjectPosition = glm::vec3(0.0f, 0.0f, 0.0f); 

    setupShader(shader);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
        glViewport(0, 0, currentWidth, currentHeight);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.update();

        if (followTrajectory && !trajectoryPoints.empty()) {
            if (trajectoryPoints.size() < 4) { 
                glm::vec3 targetPoint = trajectoryPoints[(currentTrajectoryPointIndex + 1) % trajectoryPoints.size()];
                glm::vec3 direction = targetPoint - currentObjectPosition;
                float distance = glm::length(direction);

                if (distance < trajectorySpeed) {
                    currentTrajectoryPointIndex = (currentTrajectoryPointIndex + 1) % trajectoryPoints.size();
                    currentObjectPosition = trajectoryPoints[currentTrajectoryPointIndex];
                } else {
                    currentObjectPosition += glm::normalize(direction) * trajectorySpeed;
                }
            } else { 
                int numCurvePoints = bezierCurve.getNbCurvePoints();
                if (numCurvePoints > 0) {
                    int curveIndex = (int)(trajectoryProgress * numCurvePoints);
                    curveIndex = glm::min(curveIndex, numCurvePoints - 1); 
                    currentObjectPosition = bezierCurve.getPointOnCurve(curveIndex);
                    
                    trajectoryProgress += trajectorySpeed * 10; 
                    if (trajectoryProgress >= 1.0f) {
                        trajectoryProgress = 0.0f; 
                    }
                }
            }
        }

        if (trajectoryPoints.size() >= 4) {
            bezierCurve.drawCurve(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); 
        }

        objectMesh.update(currentObjectPosition, rotateX, rotateY, rotateZ, objectScale);
        objectMesh.draw(texID);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}