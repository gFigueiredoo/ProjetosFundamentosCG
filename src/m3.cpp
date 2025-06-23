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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void setupWindow(GLFWwindow*& window);
void resetAllRotate();
void setupTransformacoes(glm::mat4& model);

void readFromMtl(string path);
int setupGeometry();
int loadTexture(string path);
void readFromObj(string path);

vector<GLfloat> vertices;
vector<GLfloat> textures;
string mtlFilePath = "";
string textureFilePath = "";

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;

float scaleLevel = 0.5f;

int verticesToDraw = 0;

GLfloat translateX = 0.0f;
GLfloat translateY = 0.0f;
GLfloat translateZ = 0.0f;

struct ObjectInstance {
    glm::vec3 position;
    glm::vec3 scale;
    float rotationAngle;
    glm::vec3 rotationAxis;

    ObjectInstance() : position(0.0f), scale(1.0f), rotationAngle(0.0f), rotationAxis(0.0f, 1.0f, 0.0f) {}
};

std::vector<ObjectInstance> objects;

int main()
{
    GLFWwindow* window;

    setupWindow(window);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Shader shader("../shaders/sprite.vs", "../shaders/sprite.fs");

    readFromObj("../assets/Modelos3D/Suzanne.obj"); 
    
    readFromMtl("../assets/Modelos3D/Suzanne.mtl");

    GLuint texID = loadTexture("../assets/Modelos3D/Suzanne.png"); 
    
    GLuint VAO = setupGeometry();

    glUseProgram(shader.ID);

    glUniform1i(glGetUniformLocation(shader.ID, "tex_buffer"), 0);

    glm::mat4 projection = glm::mat4(1);
    projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    projection = glm::translate(projection, glm::vec3(0.0f, 0.0f, -3.0f));

    GLint projLoc = glGetUniformLocation(shader.ID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    objects.push_back(ObjectInstance());

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
        glViewport(0, 0, currentWidth, currentHeight);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(1);
        glPointSize(1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);

        glBindVertexArray(VAO);

        for (size_t i = 0; i < objects.size(); ++i) {
            glm::mat4 model = glm::mat4(1);
            
            model = glm::translate(model, objects[i].position);
            model = glm::scale(model, objects[i].scale);

            if (i == 0) {
                if (rotateX) {
                    objects[i].rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
                    objects[i].rotationAngle += 0.05f;
                } else if (rotateY) {
                    objects[i].rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
                    objects[i].rotationAngle += 0.05f;
                } else if (rotateZ) {
                    objects[i].rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
                    objects[i].rotationAngle += 0.05f;
                }
            }
            model = glm::rotate(model, objects[i].rotationAngle, objects[i].rotationAxis);
            
            GLint modelLoc = glGetUniformLocation(shader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, verticesToDraw);
        }

        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}

void readFromMtl(string path)
{
    string line, readValue;
    ifstream mtlFile(path);

    if (!mtlFile.is_open()) {
        cerr << "Erro ao abrir arquivo MTL: " << path << endl;
        return;
    }

    while (!mtlFile.eof())
    {
        getline(mtlFile, line);

        istringstream iss(line);

        if (line.find("map_Kd") == 0)
        {
            iss >> readValue >> textureFilePath;
        }
    }
    mtlFile.close();
}

int setupGeometry()
{
    GLuint VAO, VBO[2];

    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(GLfloat), textures.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

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

                    vertices.push_back(temp_vertices[vertexIndex - 1].x);
                    vertices.push_back(temp_vertices[vertexIndex - 1].y);
                    vertices.push_back(temp_vertices[vertexIndex - 1].z);

                    textures.push_back(temp_textures[textIndex - 1].x);
                    textures.push_back(1.0f - temp_textures[textIndex - 1].y);
                }
            } 
            else if (prefix == "mtllib")
            {
                iss >> mtlFilePath;
            }
        }
    }

    file.close();

    verticesToDraw = vertices.size() / 3;

    std::cout << "OBJ carregado. Total de vertices para desenho: " << verticesToDraw << std::endl;
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

void setupWindow(GLFWwindow*& window) {
    glfwInit();

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Modulo 3 - Gabriel", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;
}

void setupTransformacoes(glm::mat4& model) {
    model = glm::mat4(1);

    model = glm::translate(model, glm::vec3(translateX, translateY, translateZ));
    
    model = glm::scale(model, glm::vec3(scaleLevel, scaleLevel, scaleLevel));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    const float scaleStep = 0.1f;
    const float translateStep = 0.1f;

    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        objects[0].scale += scaleStep;
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        objects[0].scale -= scaleStep;
        if (objects[0].scale.x < 0.1f) objects[0].scale = glm::vec3(0.1f);
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if ((key == GLFW_KEY_X) && action == GLFW_PRESS)
    {
        resetAllRotate();
        rotateX = true;
    }

    if ((key == GLFW_KEY_Y) && action == GLFW_PRESS)
    {
        resetAllRotate();
        rotateY = true;
    }

    if ((key == GLFW_KEY_Z) && action == GLFW_PRESS)
    {
        resetAllRotate();
        rotateZ = true;
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_A:
            objects[0].position.x -= translateStep;
            break;
        case GLFW_KEY_D:
            objects[0].position.x += translateStep;
            break;
        case GLFW_KEY_W:
            objects[0].position.y += translateStep;
            break;
        case GLFW_KEY_S:
            objects[0].position.y -= translateStep;
            break;
        case GLFW_KEY_I:
            objects[0].position.z -= translateStep;
            break;
        case GLFW_KEY_J:
            objects[0].position.z += translateStep;
            break;
        case GLFW_KEY_N:
            if (action == GLFW_PRESS) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dis_pos(-1.5, 1.5);
                std::uniform_real_distribution<> dis_scale(0.2, 0.7);
                std::uniform_real_distribution<> dis_angle(0.0, glm::radians(360.0f));

                ObjectInstance newObj;
                newObj.position = glm::vec3(dis_pos(gen), dis_pos(gen), dis_pos(gen));
                newObj.scale = glm::vec3(dis_scale(gen));
                newObj.rotationAngle = dis_angle(gen);
                newObj.rotationAxis = glm::normalize(glm::vec3(dis_pos(gen), dis_pos(gen), dis_pos(gen)));
                if (newObj.rotationAxis == glm::vec3(0.0f)) newObj.rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
                
                objects.push_back(newObj);
            }
            break;
        default:
            break;
        }
    }

    if ((key == GLFW_KEY_P) && action == GLFW_PRESS)
    {
        resetAllRotate();
        objects[0].position = glm::vec3(0.0f);
        objects[0].scale = glm::vec3(0.5f);
        objects[0].rotationAngle = 0.0f;
        objects[0].rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

void resetAllRotate() {
    rotateX = false;
    rotateY = false;
    rotateZ = false;
}