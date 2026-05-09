#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <cmath>

struct Triangle {
    glm::vec2 vertices[3];
    glm::vec3 color;
};

static int windowWidth = 800;
static int windowHeight = 600;

static std::vector<glm::vec2> currentPoints;
static std::vector<Triangle> triangles;

GLuint shaderProgram = 0;
GLuint VAO = 0;
GLuint VBO = 0;

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Erro ao compilar shader:\n" << infoLog << std::endl;
    }
    return shader;
}

static GLuint createShaderProgram() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 uProjection;

        void main() {
            gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;

        void main() {
            FragColor = vec4(uColor, 1.0);
        }
    )";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Erro ao linkar shader program:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

static glm::vec3 generateColor(int index) {
    float r = std::sin(index * 1.7f + 0.0f) * 0.5f + 0.5f;
    float g = std::sin(index * 1.7f + 2.1f) * 0.5f + 0.5f;
    float b = std::sin(index * 1.7f + 4.2f) * 0.5f + 0.5f;
    return glm::vec3(r, g, b);
}

static void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Converte para coordenadas da janela (origem no canto inferior esquerdo)
        float x = static_cast<float>(xpos);
        float y = static_cast<float>(windowHeight - ypos);

        currentPoints.push_back(glm::vec2(x, y));

        if (currentPoints.size() == 3) {
            Triangle tri;
            tri.vertices[0] = currentPoints[0];
            tri.vertices[1] = currentPoints[1];
            tri.vertices[2] = currentPoints[2];
            tri.color = generateColor(static_cast<int>(triangles.size()));

            triangles.push_back(tri);
            currentPoints.clear();
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Vivencial 1 - Triangulos", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao carregar GLAD\n";
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glViewport(0, 0, windowWidth, windowHeight);

    shaderProgram = createShaderProgram();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 3, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowWidth),
                                      0.0f, static_cast<float>(windowHeight));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Atualiza a projeção caso a janela tenha sido redimensionada
        projection = glm::ortho(0.0f, static_cast<float>(windowWidth),
                                0.0f, static_cast<float>(windowHeight));

        GLint projLoc = glGetUniformLocation(shaderProgram, "uProjection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        for (const auto& tri : triangles) {
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * 3, tri.vertices);

            GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
            glUniform3f(colorLoc, tri.color.r, tri.color.g, tri.color.b);

            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}