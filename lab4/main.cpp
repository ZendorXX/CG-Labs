#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

const GLfloat cubeVertices[] = {
    // Front face
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    // Back face
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    // Right face
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
    // Left face
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    // Top face
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    // Bottom face
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
};

const GLfloat cubeNormals[] = {
    // Front face
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    // Back face
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    // Right face
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    // Left face
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    // Top face
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    // Bottom face
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
};

const GLuint cubeIndices[] = {
    0, 1, 2, 2, 3, 0,   // Front
    4, 5, 6, 6, 7, 4,   // Back
    8, 9, 10, 10, 11, 8, // Right
    12, 13, 14, 14, 15, 12, // Left
    16, 17, 18, 18, 19, 16, // Top
    20, 21, 22, 22, 23, 20  // Bottom
};

GLuint VAO, VBO, NBO, EBO;
float scale = 1.0f;
const float minScale = 0.05f;
const float maxScale = 6.0f;

glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

GLuint flatShaderProgram, gouraudShaderProgram;
GLuint currentShaderProgram;

glm::vec3 cameraPosition = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.01f;

float yaw = -90.0f; 
float pitch = 0.0f; 
float rotationSpeed = 0.01f; 

bool flatShading = true; // Флаг для переключения между плоским и гладким затенением

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

void createFlatShaderProgram() {
    std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;

        out vec3 FragPos;
        out vec3 Normal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    std::string fragmentShaderSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;

        out vec4 FragColor;

        uniform vec3 lightPos;
        uniform vec3 lightPos2;
        uniform vec3 viewPos;
        uniform vec3 lightColor;

        void main() {
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            vec3 lightDir2 = normalize(lightPos2 - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            float diff2 = max(dot(norm, lightDir2), 0.0);
            vec3 diffuse = diff * lightColor * 1.25;
            vec3 diffuse2 = diff2 * lightColor * 1.25;

            vec3 result = diffuse + diffuse2;
            FragColor = vec4(result, 1.0f);
        }
    )";

    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    flatShaderProgram = glCreateProgram();
    glAttachShader(flatShaderProgram, vertexShader);
    glAttachShader(flatShaderProgram, fragmentShader);
    glLinkProgram(flatShaderProgram);

    GLint success;
    glGetProgramiv(flatShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(flatShaderProgram, 512, nullptr, infoLog);
        std::cerr << "Flat shader program linking error: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void createGouraudShaderProgram() {
    std::string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;

        out vec3 FragColor;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform vec3 lightPos;
        uniform vec3 lightPos2;
        uniform vec3 viewPos;
        uniform vec3 lightColor;

        void main() {
            vec3 FragPos = vec3(model * vec4(aPos, 1.0));
            vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
            vec3 norm = normalize(Normal);

            // Освещение от первого источника света
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;

            // Освещение от второго источника света
            vec3 lightDir2 = normalize(lightPos2 - FragPos);
            float diff2 = max(dot(norm, lightDir2), 0.0);
            vec3 diffuse2 = diff2 * lightColor;

            // Суммарное освещение
            FragColor = diffuse + diffuse2;

            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    std::string fragmentShaderSource = R"(
        #version 330 core
        in vec3 FragColor;

        out vec4 FragColorOut;

        void main() {
            FragColorOut = vec4(FragColor, 1.0f);
        }
    )";

    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    gouraudShaderProgram = glCreateProgram();
    glAttachShader(gouraudShaderProgram, vertexShader);
    glAttachShader(gouraudShaderProgram, fragmentShader);
    glLinkProgram(gouraudShaderProgram);

    GLint success;
    glGetProgramiv(gouraudShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(gouraudShaderProgram, 512, nullptr, infoLog);
        std::cerr << "Gouraud shader program linking error: " << infoLog << std::endl;
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
    return glm::lookAt(eye, target, up);
}

glm::mat4 perspective(float fov, float aspect, float near, float far) {
    return glm::perspective(fov, aspect, near, far);
}

void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    createFlatShaderProgram();
    createGouraudShaderProgram();

    currentShaderProgram = flatShaderProgram;

    viewMatrix = lookAt(cameraPosition, cameraTarget, cameraUp);
    projectionMatrix = perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
}

void drawCube() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool mKeyPressed = false;

void processInput(sf::Window& window) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        scale += 0.01f;
        scale = std::min(scale, maxScale);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
        scale -= 0.01f;
        scale = std::max(scale, minScale);
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

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::M) && !mKeyPressed) {
        flatShading = !flatShading;
        currentShaderProgram = flatShading ? flatShaderProgram : gouraudShaderProgram;
        std::cout << "Shading mode: " << (flatShading ? "Flat" : "Gouraud") << std::endl;
        mKeyPressed = true;
    }
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
        mKeyPressed = false;
    }
}

void resizeCallback(sf::Window& window, int width, int height) {
    glViewport(0, 0, width, height);
    projectionMatrix = perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
}

int main() {
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    sf::Window window(sf::VideoMode(800, 600), "Lab 4", sf::Style::Default, settings);
    window.setActive(true);

    glewInit();
    initOpenGL();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    glm::vec3 lightPos = glm::vec3(1.5f, 2.0f, 3.0f);
    glm::vec3 lightPos2 = glm::vec3(-1.5f, 2.0f, -3.0f);

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

        glUseProgram(currentShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(currentShaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniform3fv(glGetUniformLocation(currentShaderProgram, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(currentShaderProgram, "lightPos2"), 1, &lightPos2[0]);
        glUniform3fv(glGetUniformLocation(currentShaderProgram, "viewPos"), 1, &cameraPosition[0]);
        glUniform3f(glGetUniformLocation(currentShaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        drawCube();

        window.display();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);
    glDeleteBuffers(1, &EBO);

    return 0;
}