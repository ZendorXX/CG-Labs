#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

// Вершины пирамиды
const GLfloat pyramidVertices[] = {
    // Основание (квадрат)
    -1.0f, -1.0f, -1.0f, // Вершина 0
     1.0f, -1.0f, -1.0f, // Вершина 1
     1.0f, -1.0f,  1.0f, // Вершина 2
    -1.0f, -1.0f,  1.0f, // Вершина 3
    // Вершина
     0.0f,  1.0f,  0.0f  // Вершина 4
};

// Цвета для каждой вершины
const GLfloat pyramidColors[] = {
    1.0f, 0.0f, 0.0f, // Красный (вершина 0)
    0.0f, 1.0f, 0.0f, // Зеленый (вершина 1)
    0.0f, 0.0f, 1.0f, // Синий (вершина 2)
    1.0f, 1.0f, 0.0f, // Желтый (вершина 3)
    1.0f, 0.0f, 1.0f  // Фиолетовый (вершина 4)
};

// Индексы для отрисовки треугольников
const GLuint pyramidIndices[] = {
    0, 1, 2, // Основание
    2, 3, 0, // Основание
    0, 1, 4, // Боковая грань 1
    1, 2, 4, // Боковая грань 2
    2, 3, 4, // Боковая грань 3
    3, 0, 4  // Боковая грань 4
};

// Глобальные переменные для VAO и масштабирования
GLuint VAO, VBO, CBO, EBO;
float scale = 1.0f;
const float minScale = 0.5f;
const float maxScale = 2.0f;

// Матрицы для камеры
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

// Шейдеры
GLuint shaderProgram;

// Камера
glm::vec3 cameraPosition = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.05f;

// Функция для компиляции шейдеров
GLuint compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }
    return shader;
}

// Функция для создания шейдерной программы
void createShaderProgram() {
    // Вершинный шейдер
    std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;
        out vec3 ourColor;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            ourColor = aColor;
        }
    )";

    // Фрагментный шейдер
    std::string fragmentShaderSource = R"(
        #version 330 core
        in vec3 ourColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(ourColor, 1.0f);
        }
    )";

    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Создание шейдеров
    createShaderProgram();

    // Настройка матриц
    viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);
    projectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
}

void drawPyramid() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void processInput(sf::Window& window) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        scale += 0.01f;
        scale = std::min(scale, maxScale);
        std::cout << "Current scale: " << scale << std::endl;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        scale -= 0.01f;
        scale = std::max(scale, minScale);
        std::cout << "Current scale: " << scale << std::endl;
    }

    // Управление камерой
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        cameraPosition += cameraSpeed * glm::normalize(cameraTarget - cameraPosition);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        cameraPosition -= cameraSpeed * glm::normalize(cameraTarget - cameraPosition);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        cameraPosition -= glm::normalize(glm::cross(cameraTarget - cameraPosition, cameraUp)) * cameraSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        cameraPosition += glm::normalize(glm::cross(cameraTarget - cameraPosition, cameraUp)) * cameraSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        cameraPosition.y += cameraSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        cameraPosition.y -= cameraSpeed;

    // Обновление матрицы вида
    viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

    // Вывод положения камеры в консоль
    std::cout << "Camera position: (" << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << ")" << std::endl;
}

int main() {
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    sf::Window window(sf::VideoMode(800, 600), "Lab 3", sf::Style::Default, settings);
    window.setActive(true);

    glewInit();
    initOpenGL();

    // Настройка VAO, VBO, CBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Вершины
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Цвета
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidColors), pyramidColors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);

    // Индексы
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Применение масштабирования
        glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));

        // Установка матриц в шейдеры
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

        drawPyramid();

        window.display();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &CBO);
    glDeleteBuffers(1, &EBO);

    return 0;
}