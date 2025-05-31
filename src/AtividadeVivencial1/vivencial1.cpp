#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// Protótipos das funções
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
GLuint setupGeometry(bool isYellow); // Alterado para retornar GLuint

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

// Estrutura para armazenar os atributos de um objeto 3D
struct Object3D {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    GLuint VAO; // Cada objeto terá seu próprio VAO
    bool selected;
};

// Lista de objetos 3D
vector<Object3D> objects;
int selectedObjectIndex = 0;

int main() {
    glfwInit();

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Visualizador 3D - Objetos", nullptr, nullptr);
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

    // Compilação dos shaders
    GLuint shaderID = setupShader();
    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    glEnable(GL_DEPTH_TEST);

    // Adiciona dois objetos iniciais na cena (um amarelo e um vermelho)
    objects.push_back({ glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f), setupGeometry(true), true });  // Cubo amarelo
    objects.push_back({ glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f), setupGeometry(false), false }); // Cubo vermelho

    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t i = 0; i < objects.size(); ++i) {
            Object3D& obj = objects[i];

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, obj.position);
            model = glm::rotate(model, obj.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, obj.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, obj.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, obj.scale);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(obj.VAO);

            // Desenho do objeto
            if (obj.selected) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Preenchido
            }
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    for (auto& obj : objects) {
        glDeleteVertexArrays(1, &obj.VAO);
    }
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Seleção de objetos
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        objects[selectedObjectIndex].selected = false;
        selectedObjectIndex = (selectedObjectIndex + 1) % objects.size();
        objects[selectedObjectIndex].selected = true;
    }

    // Rotação
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        objects[selectedObjectIndex].rotation.x += glm::radians(10.0f);
        objects[selectedObjectIndex].rotation.y += glm::radians(10.0f);
        objects[selectedObjectIndex].rotation.z += glm::radians(10.0f);
    }

    // Translação
    if (key == GLFW_KEY_W) objects[selectedObjectIndex].position.z -= 0.1f;
    if (key == GLFW_KEY_S) objects[selectedObjectIndex].position.z += 0.1f;
    if (key == GLFW_KEY_A) objects[selectedObjectIndex].position.x -= 0.1f;
    if (key == GLFW_KEY_D) objects[selectedObjectIndex].position.x += 0.1f;
    if (key == GLFW_KEY_I) objects[selectedObjectIndex].position.y += 0.1f;
    if (key == GLFW_KEY_K) objects[selectedObjectIndex].position.y -= 0.1f;

    // Escala
    if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
        objects[selectedObjectIndex].scale *= 0.9f; // Diminui a escala
    }
    if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
        objects[selectedObjectIndex].scale *= 1.1f; // Aumenta a escala
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

GLuint setupGeometry(bool isYellow) { // Alterado para retornar GLuint
    GLfloat vertices[] = {
        // Frente
        -0.5, -0.5,  0.5, isYellow ? 1.0f : 1.0f, isYellow ? 1.0f : 0.0f, 0.0f,
         0.5, -0.5,  0.5, isYellow ? 1.0f : 1.0f, isYellow ? 1.0f : 0.0f, 0.0f,
         0.5,  0.5,  0.5, isYellow ? 1.0f : 1.0f, isYellow ? 1.0f : 0.0f, 0.0f,
        -0.5, -0.5,  0.5, isYellow ? 1.0f : 1.0f, isYellow ? 1.0f : 0.0f, 0.0f,
         0.5,  0.5,  0.5, isYellow ? 1.0f : 1.0f, isYellow ? 1.0f : 0.0f, 0.0f,
        -0.5,  0.5,  0.5, isYellow ? 1.0f : 1.0f, isYellow ? 1.0f : 0.0f, 0.0f,
        // Outros lados...
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