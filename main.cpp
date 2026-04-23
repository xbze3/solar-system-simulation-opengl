#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <string>

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightDir;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 ambient = 0.25 * objectColor;
    vec3 diffuse = diff * objectColor;
    FragColor = vec4(ambient + diffuse, 1.0);
}
)";

// For Saturns ring and also displaying planetary orbits
void createRingMesh(
    float innerRadius,
    float outerRadius,
    int segments,
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices
) {
    const float PI = 3.14159265359f;

    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float x = cos(angle);
        float z = sin(angle);

        // outer vertex
        vertices.push_back(x * outerRadius);
        vertices.push_back(0.0f);
        vertices.push_back(z * outerRadius);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);

        // inner vertex
        vertices.push_back(x * innerRadius);
        vertices.push_back(0.0f);
        vertices.push_back(z * innerRadius);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
    }

    for (int i = 0; i < segments; i++) {
        int outer1 = i * 2;
        int inner1 = i * 2 + 1;
        int outer2 = (i + 1) * 2;
        int inner2 = (i + 1) * 2 + 1;

        indices.push_back(outer1);
        indices.push_back(inner1);
        indices.push_back(outer2);

        indices.push_back(outer2);
        indices.push_back(inner1);
        indices.push_back(inner2);
    }
}


// Shader compilation helper
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Shader compilation failed:\n" << infoLog << std::endl;
    }

    return shader;
}

// Check program link status helper
void checkProgramLinkStatus(unsigned int program) {
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cout << "Program linking failed:\n" << infoLog << std::endl;
    }
}

// Callback function to allow the viewport to resize dynamically with the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Create verticies for lines around the sun showing the paths of the orbits
void createOrbitCircle(float radius, int segments, std::vector<float>& vertices) {
    const float PI = 3.14159265359f;

    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);

        // dummy normal (not used for lines, but shader expects it)
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
    }
}

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 30.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// mouse control
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;

// store camera captured state
bool cursorDisabled = true;
bool tabPressedLastFrame = false;

// Moon values
float moonSize = 0.25f;
float moonOrbitRadius = 1.5f;
float moonRotationSpeed = 1.0f;
float moonRevolutionSpeed = 10.0f;
glm::vec3 moonColor = glm::vec3(0.8f, 0.8f, 0.85f);

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Create function to keep all input code organized
void processInput(GLFWwindow* window) {
    float cameraSpeed = 8.0f * deltaTime;

    glm::vec3 forward = glm::normalize(cameraFront);
    glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * forward;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * forward;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= right * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += right * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressedLastFrame) {
        cursorDisabled = !cursorDisabled;

        if (cursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    tabPressedLastFrame = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
}

int main()
{
    // Initialize GLFW
    glfwInit();

    // GLFW Setup
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window object
    GLFWwindow* window = glfwCreateWindow(800, 600, "CSE3200 - Group 1 - Solar System Simulator", NULL, NULL);

    // Check to make sure window object was successfully created
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Viewport
    glViewport(0, 0, 800, 600);

    // Compile and link shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramLinkStatus(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create a UV sphere (positions + normals)
    std::vector<float> interleaved; // pos(x,y,z) + normal(x,y,z)
    std::vector<unsigned int> indices;

    const float radius = 1.0f; // base unit sphere radius
    const unsigned int sectorCount = 64;
    const unsigned int stackCount = 64;
    const float PI = 3.14159265359f;

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * PI / stackCount;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * PI / sectorCount;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            interleaved.push_back(x);
            interleaved.push_back(y);
            interleaved.push_back(z);

            glm::vec3 n = glm::normalize(glm::vec3(x, y, z));
            interleaved.push_back(n.x);
            interleaved.push_back(n.y);
            interleaved.push_back(n.z);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
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

    // Create VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, interleaved.size() * sizeof(float), interleaved.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Matrices
    glm::mat4 model = glm::mat4(1.0f);

    glUseProgram(shaderProgram);
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    unsigned int lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");

    // Celestial body properties (Sun)
    struct Body {
        std::string name;
        float scaledSize;    // simulation units
        float orbitRadius;   // simulation units
        float rotationSpeed; // degrees per second
        float revolutionSpeed;// degrees per second
        glm::vec3 color; // added color so we can tell between the spheres for right now
    };

    // Enable for scary mode
    //std::vector<Body> bodies = {
    //    {"Sun",     5.3631f,   0.0f,      0.2f,  0.0f,  glm::vec3(1.0f, 0.85f, 0.2f)},
    //    {"Mercury", 1.0153f,   8.0000f,   0.4f,  1.6f,  glm::vec3(0.7f, 0.7f, 0.7f)},
    //    {"Venus",   1.0380f,  14.9499f,   0.15f, 1.2f,  glm::vec3(0.9f, 0.7f, 0.3f)},
    //    {"Earth",   1.0400f,  20.6701f,   0.5f,  1.0f,  glm::vec3(0.2f, 0.4f, 1.0f)},
    //    {"Mars",    1.0213f,  31.4888f,   0.45f, 0.8f,  glm::vec3(0.8f, 0.3f, 0.2f)},
    //    {"Jupiter", 1.4484f, 107.5786f,   0.9f,  0.4f,  glm::vec3(0.8f, 0.6f, 0.4f)},
    //    {"Saturn",  1.3780f, 198.0656f,   0.8f,  0.3f,  glm::vec3(0.9f, 0.8f, 0.5f)},
    //    {"Uranus",  1.1603f, 396.8912f,   0.6f,  0.2f,  glm::vec3(0.5f, 0.8f, 0.9f)},
    //    {"Neptune", 1.1553f, 621.0846f,   0.55f, 0.15f, glm::vec3(0.3f, 0.5f, 0.9f)}
    //};

    std::vector<Body> bodies = {
        {"Sun",     4.5f,   0.0f,  0.2f,  6.0f,  glm::vec3(1.0f, 0.85f, 0.2f)},
        {"Mercury", 0.45f,  16.0f,  0.4f,  7.6f,  glm::vec3(0.7f, 0.7f, 0.7f)},
        {"Venus",   0.85f, 19.0f,  0.15f, 7.2f,  glm::vec3(0.9f, 0.7f, 0.3f)},
        {"Earth",   0.90f, 22.5f,  0.5f,  7.0f,  glm::vec3(0.2f, 0.4f, 1.0f)},
        {"Mars",    0.65f, 26.0f,  0.45f, 6.8f,  glm::vec3(0.8f, 0.3f, 0.2f)},
        {"Jupiter", 2.20f, 31.5f,  0.9f,  6.4f,  glm::vec3(0.8f, 0.6f, 0.4f)},
        {"Saturn",  1.90f, 40.0f,  0.8f,  6.3f,  glm::vec3(0.9f, 0.8f, 0.5f)},
        {"Uranus",  1.30f, 46.5f,  0.6f,  6.2f,  glm::vec3(0.5f, 0.8f, 0.9f)},
        {"Neptune", 1.20f, 53.0f,  0.55f, 6.15f, glm::vec3(0.3f, 0.5f, 0.9f)}
    };

    // Setup Saturns ring buffers
    std::vector<float> ringVertices;
    std::vector<unsigned int> ringIndices;
    createRingMesh(2.4f, 3.6f, 100, ringVertices, ringIndices);

    unsigned int ringVAO, ringVBO, ringEBO;
    glGenVertexArrays(1, &ringVAO);
    glGenBuffers(1, &ringVBO);
    glGenBuffers(1, &ringEBO);

    glBindVertexArray(ringVAO);

    glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    glBufferData(GL_ARRAY_BUFFER, ringVertices.size() * sizeof(float), ringVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ringEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ringIndices.size() * sizeof(unsigned int), ringIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw rings around the sun showing the orbits of the planets
    std::vector<float> orbitVertices;
    createOrbitCircle(1.0f, 100, orbitVertices); // unit circle

    unsigned int orbitVAO, orbitVBO;
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);

    glBindVertexArray(orbitVAO);

    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(float), orbitVertices.data(), GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal (dummy)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Callbacks and input
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        processInput(window);

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Rebuild projection matrix using current window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;

        glm::mat4 projection = glm::perspective(
            glm::radians(70.0f),
            (float)width / (float)height,
            0.1f,
            100.0f
        );

        glUseProgram(shaderProgram);

        // Update model for the sun: scale by `sun.scaledSize` and rotate over time
        // I updated this to be a loop that loops through the planet definitions in the vector above 

        float time = (float)glfwGetTime();

        // Value to store earths position so that the moon can orbit it as the origin
        glm::vec3 earthPosition(0.0f);

        for (const Body& body : bodies) {
            glm::mat4 model = glm::mat4(1.0f);

            // Revolution around the origin
            float revAngle = glm::radians(body.revolutionSpeed * time);
            glm::vec3 position = glm::vec3(
                cos(revAngle) * body.orbitRadius,
                0.0f,
                sin(revAngle) * body.orbitRadius
            );
            model = glm::translate(model, position);

            // Self rotation
            float rotAngle = glm::radians(body.rotationSpeed * time);
            model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));

            // Scale
            model = glm::scale(model, glm::vec3(body.scaledSize));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3f(colorLoc, body.color.r, body.color.g, body.color.b);

            glm::vec3 lightDir = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));
            glUniform3f(lightDirLoc, lightDir.x, lightDir.y, lightDir.z);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

            // Draw lines around sun showing the orbits of the plantes
            if (body.orbitRadius > 0.0f) {
                glm::mat4 orbitModel = glm::mat4(1.0f);

                // scale unit circle to planet orbit radius
                orbitModel = glm::scale(orbitModel, glm::vec3(body.orbitRadius));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(orbitModel));
                glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

                // dim color for orbit line
                glUniform3f(colorLoc, 0.5f, 0.5f, 0.5f);

                glBindVertexArray(orbitVAO);
                glDrawArrays(GL_LINE_LOOP, 0, 100);
            }

            if (body.name == "Earth") {
                earthPosition = position;
            }

            if (body.name == "Saturn") {
                glm::mat4 ringModel = glm::mat4(1.0f);

                float revAngle = glm::radians(body.revolutionSpeed * time);
                glm::vec3 position = glm::vec3(
                    cos(revAngle) * body.orbitRadius,
                    0.0f,
                    sin(revAngle) * body.orbitRadius
                );
                ringModel = glm::translate(ringModel, position);

                // slight tilt
                ringModel = glm::rotate(ringModel, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ringModel));
                glUniform3f(colorLoc, 0.8f, 0.75f, 0.6f);

                glBindVertexArray(ringVAO);
                glDrawElements(GL_TRIANGLES, (GLsizei)ringIndices.size(), GL_UNSIGNED_INT, 0);
            }

        }

        // Draw moon

        glm::mat4 moonModel = glm::mat4(1.0f);

        float moonAngle = glm::radians(moonRevolutionSpeed * time);
        glm::vec3 moonOffset = glm::vec3(
            cos(moonAngle) * moonOrbitRadius,
            0.0f,
            sin(moonAngle) * moonOrbitRadius
        );

        moonModel = glm::translate(moonModel, earthPosition + moonOffset);

        float moonRotAngle = glm::radians(moonRotationSpeed * time);
        moonModel = glm::rotate(moonModel, moonRotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        moonModel = glm::scale(moonModel, glm::vec3(moonSize));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(moonModel));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(colorLoc, moonColor.r, moonColor.g, moonColor.b);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

        glfwPollEvents();

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}