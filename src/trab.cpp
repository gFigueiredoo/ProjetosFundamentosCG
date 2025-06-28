/*
 * m6_restructured.cpp - Adaptado para trabalho PreparacaoGrauB
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

class Application {
private:
    GLFWwindow* window;
    Camera camera;
    std::vector<Mesh> meshes;
    std::vector<Bezier> bezierCurves;
    Scene scene;

    bool rotateX = false;
    bool rotateY = false;
    bool rotateZ = false;

    int selectedObjectIndex = 0;

    Shader* objectShader = nullptr;
    Shader* curveShader = nullptr;

    std::vector<float> trajectoryProgress;

public:
    Application() : window(nullptr) {}

    void run() {
        setupWindow();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        Shader objShader("../shaders/object.vs", "../shaders/object.fs");
        Shader curShader("../shaders/curve.vs", "../shaders/curve.fs");
        objectShader = &objShader;
        curveShader = &curShader;

        if (!scene.loadConfig("../assets/scene_config.json")) {
            cerr << "Falha ao carregar configuração da cena. Saindo." << endl;
            glfwTerminate();
            return;
        }

        camera.initialize(objectShader, WINDOW_WIDTH, WINDOW_HEIGHT);

        scene.setupScene(window, objectShader, &camera, meshes, bezierCurves);

        camera.setCameraPosInitial(scene.cameraInitialPos);
        camera.setCameraFrontInitial(scene.cameraInitialFront);
        camera.setCameraUpInitial(scene.cameraInitialUp);
        camera.setProjection(scene.cameraFov, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, scene.cameraNearPlane, scene.cameraFarPlane);

        objectShader->Use();
        if (!scene.lightSources.empty()) {
            objectShader->setVec3("light.position", scene.lightSources[0].position);
            objectShader->setVec3("light.ambient", scene.lightSources[0].ambient);
            objectShader->setVec3("light.diffuse", scene.lightSources[0].diffuse);
            objectShader->setVec3("light.specular", scene.lightSources[0].specular);
        }

        trajectoryProgress.resize(bezierCurves.size(), 0.0f);

        double lastFrameTime = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            double currentFrameTime = glfwGetTime();
            double deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;

            glfwPollEvents();

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            objectShader->Use();
            camera.update();

            updateBezierAnimations(deltaTime);

            updateAndDrawMeshes();

            drawBezierCurves();

            glfwSwapBuffers(window);
        }

        cleanup();

        glfwTerminate();
    }

private:
    void updateBezierAnimations(double deltaTime) {
        for (size_t i = 0; i < meshes.size(); ++i) {
            if (i < bezierCurves.size() && bezierCurves[i].getFollowTrajectory()) {
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
    }

    void updateAndDrawMeshes() {
        for (size_t i = 0; i < meshes.size(); ++i) {
            bool currentObjectRotationX = (i == selectedObjectIndex) ? rotateX : false;
            bool currentObjectRotationY = (i == selectedObjectIndex) ? rotateY : false;
            bool currentObjectRotationZ = (i == selectedObjectIndex) ? rotateZ : false;

            meshes[i].update(currentObjectRotationX, currentObjectRotationY, currentObjectRotationZ);
            meshes[i].draw();
        }
    }

    void drawBezierCurves() {
        for (size_t i = 0; i < bezierCurves.size(); ++i) {
            if (bezierCurves[i].getNbCurvePoints() > 0) {
                bezierCurves[i].setShader(curveShader);
                bezierCurves[i].drawCurve(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
        }
    }

    void cleanup() {
        for (const auto& mesh : meshes) {
            glDeleteVertexArrays(1, &mesh.VAO);
        }
    }

    void setupWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PreparacaoGrauB - Gabriel", nullptr, nullptr);
        glfwMakeContextCurrent(window);

        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, keyCallbackStatic);
        glfwSetCursorPosCallback(window, mouseCallbackStatic);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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

    // Callbacks need to be static to match GLFW signature, so we forward to instance methods
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mode) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) app->keyCallback(window, key, scancode, action, mode);
    }

    static void mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) app->mouseCallback(window, xpos, ypos);
    }

    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
        float scaleStep = 0.05f;
        float translateStep = 0.1f;

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

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

    void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        camera.mouseCallback(window, xpos, ypos);
    }
};

int main() {
    Application app;
    app.run();
    return 0;
}