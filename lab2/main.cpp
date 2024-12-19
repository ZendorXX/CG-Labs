#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

GLuint VAO, VBO, CBO, EBO;
float scale = 1.0f;
const float minScale = 0.05f;
const float maxScale = 6.0f;

glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

GLuint shaderProgram;

glm::vec3 cameraPosition = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.01f;


float yaw = -90.0f; 
float pitch = 0.0f; 
float rotationSpeed = 0.01f; 

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


void createShaderProgram() {
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

glm::mat4 scaleMatrix(float scaleX, float scaleY, float scaleZ) {
    glm::mat4 scale = glm::mat4(1.0f);
    scale[0][0] = scaleX;
    scale[1][1] = scaleY;
    scale[2][2] = scaleZ;
    return scale;
}

glm::mat4 translateMatrix(const glm::vec3& translation) {
    glm::mat4 translationMatrix = glm::mat4(1.0f);
    translationMatrix[3][0] = translation.x;
    translationMatrix[3][1] = translation.y;
    translationMatrix[3][2] = translation.z;
    return translationMatrix;
}

glm::mat4 lookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up) {
    glm::vec3 forward = glm::normalize(target - eye);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    glm::vec3 newUp = glm::cross(right, forward);

    glm::mat4 view = glm::mat4(1.0f);
    view[0][0] = right.x;
    view[1][0] = right.y;
    view[2][0] = right.z;
    view[0][1] = newUp.x;
    view[1][1] = newUp.y;
    view[2][1] = newUp.z;
    view[0][2] = -forward.x;
    view[1][2] = -forward.y;
    view[2][2] = -forward.z;
    view[3][0] = -glm::dot(right, eye);
    view[3][1] = -glm::dot(newUp, eye);
    view[3][2] = glm::dot(forward, eye);

    return view;
}

glm::mat4 perspective(float fov, float aspect, float near, float far) {
    float tanHalfFov = tan(fov / 2.0f);
    glm::mat4 projection = glm::mat4(0.0f);
    projection[0][0] = 1.0f / (aspect * tanHalfFov);
    projection[1][1] = 1.0f / tanHalfFov;
    projection[2][2] = -(far + near) / (far - near);
    projection[2][3] = -1.0f;
    projection[3][2] = -(2.0f * far * near) / (far - near);
    return projection;
}

void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    createShaderProgram();

    viewMatrix = lookAt(cameraPosition, cameraTarget, cameraUp);
    projectionMatrix = perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
}

void processInput(sf::Window& window) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        scale += 0.01f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
        scale -= 0.01f;
    }

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

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        yaw -= rotationSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        yaw += rotationSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        pitch += rotationSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        pitch -= rotationSpeed;
    }

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraTarget = cameraPosition + glm::normalize(direction);

    viewMatrix = lookAt(cameraPosition, cameraTarget, cameraUp);

    std::cout << "Current sphere scale: " << scale << std::endl;
    std::cout << "Camera position: (" << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << ")" << std::endl;
}

void resizeCallback(sf::Window& window, int width, int height) {
    glViewport(0, 0, width, height);
    projectionMatrix = perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
}

void generateSphere(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, float radius, int sectorCount, int stackCount) {
    float x, y, z, xy;
    float nx, ny, nz;
    float lengthInv = 1.0f / radius;

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

int main() {
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    sf::Window window(sf::VideoMode(800, 600), "Lab 2", sf::Style::Default, settings);
    window.setActive(true);

    glewInit();
    initOpenGL();

    std::vector<GLfloat> sphereVertices;
    std::vector<GLuint> sphereIndices;
    generateSphere(sphereVertices, sphereIndices, 1.0f, 36, 18);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(GLfloat), &sphereVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(GLuint), &sphereIndices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized) {
                resizeCallback(window, event.size.width, event.size.height);
            }
        }

        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 modelMatrix = scaleMatrix(scale, scale, scale);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        window.display();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    return 0;
}