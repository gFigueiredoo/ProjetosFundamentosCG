/*
 * m6.cpp - Adaptado para trabalho PreparacaoGrauB
 * 
 * Funcionalidades:
 * - Carrega múltiplos objetos da cena via arquivo JSON
 * - Suporta seleção de objetos e transformações (translação, rotação, escala)
 * - Controle de câmera com mouse e teclado
 * - Animação por curvas de Bezier
 * - Iluminação Phong com materiais do arquivo MTL
 */

#include <iostream>
#include <vector>
#include <string>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Bezier.h"
#include "Scene.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

bool rotateX = false;
bool rotateY = false;
bool rotateZ = false;

int selectedObjectIndex = 0;

Camera camera;
std::vector<Mesh> meshes;
std::vector<Bezier> bezierCurves;
Scene scene;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void setupWindow(GLFWwindow*& window);
void resetAllRotate();

int main()
{
    GLFWwindow* window;
    setupWindow(window);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Shader objectShader("../shaders/object.vs", "../shaders/object.fs");
    Shader curveShader("../shaders/curve.vs", "../shaders/curve.fs");

    // Carrega configuração da cena (arquivo JSON)
    if (!scene.loadConfig("../assets/scene_config.json")) {
        cerr << "Falha ao carregar configuração da cena. Saindo." << endl;
        glfwTerminate();
        return -1;
    }

    // Inicializa câmera com shader e janela
    camera.initialize(&objectShader, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Configura a cena: carrega objetos, materiais, texturas, trajetórias
    scene.setupScene(window, &objectShader, &camera, meshes, bezierCurves);

    // Configura posição e orientação inicial da câmera
    camera.setCameraPosInitial(scene.cameraInitialPos);
    camera.setCameraFrontInitial(scene.cameraInitialFront);
    camera.setCameraUpInitial(scene.cameraInitialUp);
    camera.setProjection(scene.cameraFov, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, scene.cameraNearPlane, scene.cameraFarPlane);

    // Configura iluminação Phong no shader com a primeira fonte de luz da cena
    objectShader.Use();
    if (!scene.lightSources.empty()) {
        objectShader.setVec3("light.position", scene.lightSources[0].position);
        objectShader.setVec3("light.ambient", scene.lightSources[0].ambient);
        objectShader.setVec3("light.diffuse", scene.lightSources[0].diffuse);
        objectShader.setVec3("light.specular", scene.lightSources[0].specular);
    }

    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        objectShader.Use();
        camera.update();

        // Atualiza posição dos objetos animados por curvas de Bezier
        for (size_t i = 0; i < meshes.size(); ++i) {
            if (i < bezierCurves.size() && bezierCurves[i].getFollowTrajectory()) {
                static std::vector<float> trajectoryProgress(bezierCurves.size(), 0.0f);
                int numCurvePoints = bezierCurves[i].getNbCurvePoints();
                if (numCurvePoints > 0) {
                    trajectoryProgress[i] += bezierCurves[i].getSpeed() * deltaTime * 100.0f;
                    if (trajectoryProgress[i] >= 1.0f) {
                        trajectoryProgress[i] = 0.0f;
                    }
                    int curveIndex = static_cast<int>(trajectoryProgress[i] * numCurvePoints);
                    curveIndex = glm::min(curveIndex, numCurvePoints - 1);
                    glm::vec3 newPos = bezierCurves[i].getPointOnCurve(curveIndex);
                    meshes[i].setCurrentPosition(newPos);
                }
            }
        }

        // Atualiza e desenha os objetos
        for (size_t i = 0; i < meshes.size(); ++i) {
            bool currentObjectRotationX = (i == selectedObjectIndex) ? rotateX : false;
            bool currentObjectRotationY = (i == selectedObjectIndex) ? rotateY : false;
            bool currentObjectRotationZ = (i == selectedObjectIndex) ? rotateZ : false;

            meshes[i].update(currentObjectRotationX, currentObjectRotationY, currentObjectRotationZ);
            meshes[i].draw();
        }

        // Desenha as curvas de Bezier
        for (size_t i = 0; i < bezierCurves.size(); ++i) {
            if (bezierCurves[i].getNbCurvePoints() > 0) {
                bezierCurves[i].setShader(&curveShader);
                bezierCurves[i].drawCurve(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
        }

        glfwSwapBuffers(window);
    }

    // Limpa VAOs dos meshes
    for (const auto& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    float scaleStep = 0.05f;
    float translateStep = 0.1f;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Seleção de objeto com teclas 1 a 9
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9 && action == GLFW_PRESS) {
        int index = key - GLFW_KEY_1;
        if (index < meshes.size()) {
            selectedObjectIndex = index;
            resetAllRotate();
            cout << "Objeto selecionado: " << scene.objects[selectedObjectIndex].name << endl;
        }
    }

    if (selectedObjectIndex < meshes.size()) {
        glm::vec3 currentObjectPos = meshes[selectedObjectIndex].getPosition();
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key) {
                case GLFW_KEY_LEFT_BRACKET:
                    meshes[selectedObjectIndex].setScale(meshes[selectedObjectIndex].scale_ * (1.0f - scaleStep));
                    break;
                case GLFW_KEY_RIGHT_BRACKET:
                    meshes[selectedObjectIndex].setScale(meshes[selectedObjectIndex].scale_ * (1.0f + scaleStep));
                    break;
                case GLFW_KEY_X:
                    resetAllRotate();
                    rotateX = true;
                    break;
                case GLFW_KEY_Y:
                    resetAllRotate();
                    rotateY = true;
                    break;
                case GLFW_KEY_Z:
                    resetAllRotate();
                    rotateZ = true;
                    break;
                case GLFW_KEY_P:
                    resetAllRotate();
                    meshes[selectedObjectIndex].setPosition(scene.objects[selectedObjectIndex].initial_transform.position);
                    meshes[selectedObjectIndex].setRotation(scene.objects[selectedObjectIndex].initial_transform.rotation_angle, scene.objects[selectedObjectIndex].initial_transform.rotation_axis);
                    meshes[selectedObjectIndex].setScale(scene.objects[selectedObjectIndex].initial_transform.scale);
                    cout << "Transformações do objeto " << scene.objects[selectedObjectIndex].name << " resetadas." << endl;
                    break;
                case GLFW_KEY_UP:
                    currentObjectPos.z -= translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_DOWN:
                    currentObjectPos.z += translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_LEFT:
                    currentObjectPos.x -= translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_RIGHT:
                    currentObjectPos.x += translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_PAGE_UP:
                    currentObjectPos.y += translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_PAGE_DOWN:
                    currentObjectPos.y -= translateStep;
                    meshes[selectedObjectIndex].setCurrentPosition(currentObjectPos);
                    break;
                case GLFW_KEY_V:
                    if (selectedObjectIndex < bezierCurves.size()) {
                        if (selectedObjectIndex < scene.objects.size() && scene.objects[selectedObjectIndex].animation.type == "bezier") {
                            bezierCurves[selectedObjectIndex].setFollowTrajectory(!bezierCurves[selectedObjectIndex].getFollowTrajectory());
                            cout << "Trajetória para " << scene.objects[selectedObjectIndex].name << " "
                                 << (bezierCurves[selectedObjectIndex].getFollowTrajectory() ? "ativada." : "desativada.") << endl;
                        } else {
                            cout << "Nenhuma trajetória Bezier definida para o objeto selecionado." << endl;
                        }
                    } else {
                        cout << "Nenhum dado de trajetória disponível para o objeto selecionado." << endl;
                    }
                    break;
            }
        }
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        camera.setCameraPos(key);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.mouseCallback(window, xpos, ypos);
}

void setupWindow(GLFWwindow*& window) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PreparacaoGrauB - Gabriel", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Falha ao inicializar GLAD" << endl;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    glEnable(GL_DEPTH_TEST);
}

void resetAllRotate() {
    rotateX = false;
    rotateY = false;
    rotateZ = false;
}