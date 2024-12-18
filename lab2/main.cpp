#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include <iostream>

// Структура для вершин
struct Vertex {
    float x, y, z;
};

// Структура для матрицы 4x4
struct Matrix4 {
    float m[16];
};

// Функция для генерации вершин сферы
std::vector<Vertex> generateSphereVertices(float radius, int sectors, int stacks) {
    std::vector<Vertex> vertices;
    for (int i = 0; i <= stacks; ++i) {
        float lat0 = M_PI * (-0.5 + static_cast<float>(i) / stacks);
        float z0 = sin(lat0);
        float zr0 = cos(lat0);

        for (int j = 0; j <= sectors; ++j) {
            float lng = 2 * M_PI * static_cast<float>(j) / sectors;
            float x = cos(lng);
            float y = sin(lng);

            vertices.push_back({radius * x * zr0, radius * y * zr0, radius * z0});
        }
    }
    return vertices;
}

// Функция для генерации индексов сферы
std::vector<unsigned int> generateSphereIndices(int sectors, int stacks) {
    std::vector<unsigned int> indices;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            int current = i * (sectors + 1) + j;
            int next = current + sectors + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(next);
            indices.push_back(next + 1);
            indices.push_back(current + 1);
        }
    }
    return indices;
}

// Функция для умножения матриц
Matrix4 multiply(const Matrix4& a, const Matrix4& b) {
    Matrix4 result = {};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                result.m[i * 4 + j] += a.m[i * 4 + k] * b.m[k * 4 + j];
            }
        }
    }
    return result;
}

// Функция для создания матрицы масштабирования
Matrix4 scaleMatrix(float scaleX, float scaleY, float scaleZ) {
    Matrix4 mat = {};
    mat.m[0] = scaleX;
    mat.m[5] = scaleY;
    mat.m[10] = scaleZ;
    mat.m[15] = 1.0f;
    return mat;
}

// Функция для создания матрицы перспективной проекции
Matrix4 perspectiveMatrix(float fov, float aspect, float near, float far) {
    Matrix4 mat = {};
    float f = 1.0f / tan(fov / 2.0f);
    mat.m[0] = f / aspect;
    mat.m[5] = f;
    mat.m[10] = (far + near) / (near - far);
    mat.m[11] = -1.0f;
    mat.m[14] = (2.0f * far * near) / (near - far);
    return mat;
}

// Функция для создания матрицы вида (lookAt)
Matrix4 lookAtMatrix(const float* eye, const float* center, const float* up) {
    float f[3] = { center[0] - eye[0], center[1] - eye[1], center[2] - eye[2] };
    float fLength = sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    f[0] /= fLength;
    f[1] /= fLength;
    f[2] /= fLength;

    float s[3] = { f[1] * up[2] - f[2] * up[1], f[2] * up[0] - f[0] * up[2], f[0] * up[1] - f[1] * up[0] };
    float sLength = sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    s[0] /= sLength;
    s[1] /= sLength;
    s[2] /= sLength;

    float u[3] = { s[1] * f[2] - s[2] * f[1], s[2] * f[0] - s[0] * f[2], s[0] * f[1] - s[1] * f[0] };

    Matrix4 mat = {};
    mat.m[0] = s[0];
    mat.m[4] = s[1];
    mat.m[8] = s[2];
    mat.m[1] = u[0];
    mat.m[5] = u[1];
    mat.m[9] = u[2];
    mat.m[2] = -f[0];
    mat.m[6] = -f[1];
    mat.m[10] = -f[2];
    mat.m[12] = -(s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2]);
    mat.m[13] = -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]);
    mat.m[14] = f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2];
    mat.m[15] = 1.0f;
    return mat;
}

// Функция для компиляции шейдеров
GLuint compileShaders(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vShaderCode = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fShaderCode = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    // Настройка SFML окна
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    sf::Window window(sf::VideoMode(800, 600), "Lab2", sf::Style::Default, settings);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Включение глубины
    glEnable(GL_DEPTH_TEST);

    // Генерация сферы
    std::vector<Vertex> vertices = generateSphereVertices(1.0f, 36, 18);
    std::vector<unsigned int> indices = generateSphereIndices(36, 18);

    // Настройка VAO, VBO, EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Компиляция шейдеров
    const std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const std::string fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        void main() {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0); // Белый цвет
        }
    )";

    GLuint shaderProgram = compileShaders(vertexShaderSource, fragmentShaderSource);

    // Переменные для масштабирования и камеры
    float scale = 1.0f;
    float scaleSpeed = 0.01f; // Скорость изменения масштаба
    float cameraSpeed = 0.05f; // Скорость перемещения камеры
    float cameraDistance = 3.0f; // Расстояние камеры до сферы
    float cameraAngle = 0.0f; // Угол поворота камеры

    // Координаты камеры
    float eye[3] = { 0.0f, 0.0f, cameraDistance };
    float center[3] = { 0.0f, 0.0f, 0.0f };
    float up[3] = { 0.0f, 1.0f, 0.0f };

    // Основной цикл
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized) {
                glViewport(0, 0, event.size.width, event.size.height);
            }
        }

        // Очистка буфера
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // Использование шейдеров
        glUseProgram(shaderProgram);

        // Обновление матриц
        Matrix4 model = scaleMatrix(scale, scale, scale);
        Matrix4 projection = perspectiveMatrix(M_PI / 4.0f, (float)window.getSize().x / window.getSize().y, 0.1f, 100.0f);
        Matrix4 view = lookAtMatrix(eye, center, up);

        // Передача матриц в шейдер
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, model.m);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, view.m);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection.m);

        // Отрисовка сферы
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Обработка пользовательского ввода
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            scale += scaleSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            scale -= scaleSpeed;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            eye[2] -= cameraSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            eye[2] += cameraSpeed;

        // Перемещение камеры влево-вправо
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            eye[0] -= cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            eye[0] += cameraSpeed;
        }

        // Вывод информации в консоль
        std::cout << "Sphere Scale: " << scale << std::endl;
        std::cout << "Camera Position: (" << eye[0] << ", " << eye[1] << ", " << eye[2] << ")" << std::endl;

        // Обновление окна
        window.display();
    }

    // Очистка ресурсов
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    return 0;
}