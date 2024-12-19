#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include <iostream>

// Константы
const int numSegments = 50; // Количество сегментов для сферы
float radius = 1.0f;        // Начальный радиус сферы
float cameraDistance = 5.0f; // Начальное расстояние камеры
float cameraTheta = 0.0f;   // Угол поворота камеры по вертикали
float cameraPhi = 0.0f;     // Угол поворота камеры по горизонтали

// Функция для генерации вершин сферы
std::vector<float> generateSphereVertices(float radius, int numSegments) {
    std::vector<float> vertices;
    for (int i = 0; i <= numSegments; ++i) {
        float theta = i * M_PI / numSegments;
        for (int j = 0; j <= numSegments; ++j) {
            float phi = j * 2 * M_PI / numSegments;
            float x = radius * sin(theta) * cos(phi);
            float y = radius * sin(theta) * sin(phi);
            float z = radius * cos(theta);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }
    return vertices;
}

// Функция для вычисления матрицы перспективной проекции
void setPerspectiveProjection(float fov, float aspect, float zNear, float zFar) {
    float f = 1.0f / tan(fov / 2.0f * M_PI / 180.0f);
    float rangeInv = 1.0f / (zNear - zFar);

    float projectionMatrix[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (zNear + zFar) * rangeInv, -1,
        0, 0, 2 * zNear * zFar * rangeInv, 0
    };

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projectionMatrix);
}
// Функция для установки позиции камеры
void setCamera(float distance, float theta, float phi) {
    float x = distance * sin(theta) * cos(phi);
    float y = distance * sin(theta) * sin(phi);
    float z = distance * cos(theta);

    float upX = 0.0f;
    float upY = 1.0f;
    float upZ = 0.0f;

    float viewMatrix[16];
    float eye[3] = {x, y, z};
    float center[3] = {0.0f, 0.0f, 0.0f};
    float up[3] = {upX, upY, upZ};

    // Вычисление матрицы вида (LookAt)
    float f[3] = {center[0] - eye[0], center[1] - eye[1], center[2] - eye[2]};
    float fLength = sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    f[0] /= fLength;
    f[1] /= fLength;
    f[2] /= fLength;

    float s[3] = {f[1] * up[2] - f[2] * up[1], f[2] * up[0] - f[0] * up[2], f[0] * up[1] - f[1] * up[0]};
    float sLength = sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    s[0] /= sLength;
    s[1] /= sLength;
    s[2] /= sLength;

    float u[3] = {s[1] * f[2] - s[2] * f[1], s[2] * f[0] - s[0] * f[2], s[0] * f[1] - s[1] * f[0]};

    viewMatrix[0] = s[0];
    viewMatrix[1] = u[0];
    viewMatrix[2] = -f[0];
    viewMatrix[3] = 0.0f;

    viewMatrix[4] = s[1];
    viewMatrix[5] = u[1];
    viewMatrix[6] = -f[1];
    viewMatrix[7] = 0.0f;

    viewMatrix[8] = s[2];
    viewMatrix[9] = u[2];
    viewMatrix[10] = -f[2];
    viewMatrix[11] = 0.0f;

    viewMatrix[12] = -eye[0] * s[0] - eye[1] * s[1] - eye[2] * s[2];
    viewMatrix[13] = -eye[0] * u[0] - eye[1] * u[1] - eye[2] * u[2];
    viewMatrix[14] = eye[0] * f[0] + eye[1] * f[1] + eye[2] * f[2];
    viewMatrix[15] = 1.0f;

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewMatrix);
}

int main() {
    // Настройка SFML окна
    sf::ContextSettings settings;
    settings.depthBits = 24;
    sf::Window window(sf::VideoMode(800, 600), "3D Sphere", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    // Инициализация GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Ошибка инициализации GLEW" << std::endl;
        return -1;
    }

    // Генерация вершин сферы
    std::vector<float> vertices = generateSphereVertices(radius, numSegments);
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Включение теста глубины
    glEnable(GL_DEPTH_TEST);

    // Основной цикл приложения
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Обработка ввода
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            cameraTheta += 0.01f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            cameraTheta -= 0.01f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            cameraPhi -= 0.01f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            cameraPhi += 0.01f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            radius += 0.01f;
            vertices = generateSphereVertices(radius, numSegments);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            radius -= 0.01f;
            if (radius < 0.01f) radius = 0.01f;
            vertices = generateSphereVertices(radius, numSegments);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            cameraDistance -= 0.05f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            cameraDistance += 0.05f;
        }

        // Очистка экрана
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Установка перспективной проекции
        setPerspectiveProjection(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

        // Установка позиции камеры
        setCamera(cameraDistance, cameraTheta, cameraPhi);

        // Отрисовка сферы
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexPointer(3, GL_FLOAT, 0, (void*)0);
        glEnableClientState(GL_VERTEX_ARRAY);

        for (int i = 0; i < numSegments; ++i) {
            glDrawArrays(GL_TRIANGLE_STRIP, i * (numSegments + 1), (numSegments + 1) * 2);
        }

        glDisableClientState(GL_VERTEX_ARRAY);

        // Обновление окна
        window.display();
    }

    // Удаление буфера
    glDeleteBuffers(1, &vbo);

    return 0;
}