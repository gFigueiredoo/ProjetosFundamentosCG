#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// Protótipos das funções
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
int setupGeometry();

// Dimensões da janela
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

// Código fonte do Fragment Shader
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateX = false, rotateY = false, rotateZ = false;
float scaleFactor = 1.0f;

// Estrutura para representar um cubo
struct Cube {
    glm::vec3 position;
    glm::vec3 scale;
};

// Vetor de cubos
vector<Cube> cubes = { {glm::vec3(0.0f), glm::vec3(1.0f)} };

// Índice do cubo atualmente selecionado
int selectedCubeIndex = 0;

int main() {
    glfwInit();

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cubo 3D - Gabriel Figueiredo!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Callback de teclado
    glfwSetKeyCallback(window, key_callback);

    // Inicialização do GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Informações de versão
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    // Configuração da viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilação dos shaders e configuração da geometria
    GLuint shaderID = setupShader();
    GLuint VAO = setupGeometry();

    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");

    glEnable(GL_DEPTH_TEST);

    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        glBindVertexArray(VAO);

        for (size_t i = 0; i < cubes.size(); ++i) {
            Cube& cube = cubes[i];

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cube.position);
            model = glm::scale(model, cube.scale * scaleFactor);

            if (i == selectedCubeIndex) {
                // Aplica rotação apenas ao cubo selecionado
                if (rotateX) model = glm::rotate(model, (GLfloat)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));
                if (rotateY) model = glm::rotate(model, (GLfloat)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
                if (rotateZ) model = glm::rotate(model, (GLfloat)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
            }

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        rotateX = false;
        rotateY = false;
        rotateZ = true;
    }

    // Movimentação do cubo selecionado
    if (key == GLFW_KEY_W && action == GLFW_PRESS) cubes[selectedCubeIndex].position.z -= 0.1f;
    if (key == GLFW_KEY_S && action == GLFW_PRESS) cubes[selectedCubeIndex].position.z += 0.1f;
    if (key == GLFW_KEY_A && action == GLFW_PRESS) cubes[selectedCubeIndex].position.x -= 0.1f;
    if (key == GLFW_KEY_D && action == GLFW_PRESS) cubes[selectedCubeIndex].position.x += 0.1f;
    if (key == GLFW_KEY_I && action == GLFW_PRESS) cubes[selectedCubeIndex].position.y += 0.1f;
    if (key == GLFW_KEY_J && action == GLFW_PRESS) cubes[selectedCubeIndex].position.y -= 0.1f;

    // Escala do cubo selecionado
    if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) cubes[selectedCubeIndex].scale *= 0.9f;
    if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) cubes[selectedCubeIndex].scale *= 1.1f;

    // Adicionar um novo cubo
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        cubes.push_back({ glm::vec3(0.0f), glm::vec3(1.0f) });
    }

    // Alternar entre os cubos
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        selectedCubeIndex = (selectedCubeIndex + 1) % cubes.size();
    }
}

int setupShader() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int setupGeometry() {
    GLfloat vertices[] = {
        // Frente
        -0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
         0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
         0.5,  0.5,  0.5, 1.0, 0.0, 0.0,
        -0.5, -0.5,  0.5, 1.0, 0.0, 0.0,
         0.5,  0.5,  0.5, 1.0, 0.0, 0.0,
        -0.5,  0.5,  0.5, 1.0, 0.0, 0.0,

        // Trás
        -0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
        -0.5,  0.5, -0.5, 0.0, 1.0, 0.0,
         0.5,  0.5, -0.5, 0.0, 1.0, 0.0,
        -0.5, -0.5, -0.5, 0.0, 1.0, 0.0,
         0.5,  0.5, -0.5, 0.0, 1.0, 0.0,
         0.5, -0.5, -0.5, 0.0, 1.0, 0.0,

        // Esquerda
        -0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
        -0.5, -0.5,  0.5, 0.0, 0.0, 1.0,
        -0.5,  0.5,  0.5, 0.0, 0.0, 1.0,
        -0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
        -0.5,  0.5,  0.5, 0.0, 0.0, 1.0,
        -0.5,  0.5, -0.5, 0.0, 0.0, 1.0,

        // Direita
         0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
         0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
         0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
         0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
         0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
         0.5,  0.5,  0.5, 1.0, 1.0, 0.0,

        // Topo
        -0.5,  0.5, -0.5, 1.0, 0.0, 1.0,
        -0.5,  0.5,  0.5, 1.0, 0.0, 1.0,
         0.5,  0.5,  0.5, 1.0, 0.0, 1.0,
        -0.5,  0.5, -0.5, 1.0, 0.0, 1.0,
         0.5,  0.5,  0.5, 1.0, 0.0, 1.0,
         0.5,  0.5, -0.5, 1.0, 0.0, 1.0,

        // Base
        -0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
         0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
        -0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
        -0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
         0.5, -0.5, -0.5, 0.0, 1.0, 1.0,
         0.5, -0.5,  0.5, 0.0, 1.0, 1.0,
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}