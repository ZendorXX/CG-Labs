#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>
#include <vector>

// Параметры сферы
float sphereRadius = 1.0f;
int stacks = 20;
int slices = 20;

// Параметры камеры
float cameraDistance = 5.0f;
float cameraAngle = 0.0f;
float cameraHeight = 0.0f;

// Генерация сферы (вершины и индексы)
std::vector<float> vertices;
std::vector<unsigned int> indices;

void generateSphere(float radius, int stacks, int slices) {
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * M_PI * j / slices;

            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

// Настройка перспективной проекции
void setupPerspective(float aspectRatio) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

// Настройка камеры
void setupCamera() {
    float camX = cameraDistance * cos(cameraAngle);
    float camZ = cameraDistance * sin(cameraAngle);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, cameraHeight, camZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

// Отрисовка сферы
void drawSphere() {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
}

int main() {
    // Создание окна
    sf::Window window(sf::VideoMode(800, 600), "3D Sphere", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Генерация данных для сферы
    generateSphere(sphereRadius, stacks, slices);

    // Основной цикл приложения
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Add:
                        sphereRadius += 0.1f;
                        generateSphere(sphereRadius, stacks, slices);
                        break;
                    case sf::Keyboard::Subtract:
                        if (sphereRadius > 0.1f) {
                            sphereRadius -= 0.1f;
                            generateSphere(sphereRadius, stacks, slices);
                        }
                        break;
                    case sf::Keyboard::Left:
                        cameraAngle -= 0.05f;
                        break;
                    case sf::Keyboard::Right:
                        cameraAngle += 0.05f;
                        break;
                    case sf::Keyboard::Up:
                        cameraHeight += 0.1f;
                        break;
                    case sf::Keyboard::Down:
                        cameraHeight -= 0.1f;
                        break;
                    default:
                        break;
                }
            }
        }

        // Очистка буферов
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Настройка проекции и камеры
        float aspectRatio = static_cast<float>(window.getSize().x) / window.getSize().y;
        setupPerspective(aspectRatio);
        setupCamera();

        // Отрисовка сферы
        drawSphere();

        // Обновление окна
        window.display();
    }

    return 0;
}