#include <array>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <vector>

using namespace std;

// Kor pontjai es szinei
std::vector<glm::vec3> circlePoints;
std::vector<glm::vec3> circleColors;
std::vector<glm::vec3> linePoints;
std::vector<glm::vec3> lineColors;

// Ablak merete
const int window_width = 600;
const int window_height = 600;
const float circle_radius = 50.0f / 300.0f; // 50 pixel sugaru kor normalt koordinatakban

GLuint VBO[4];  // Most 4 VBO-t hasznalunk
GLuint VAO[2];  // Most 2 VAO-t hasznalunk
GLuint renderingProgram;
GLuint lineShaderProgram;
float lineY = 0.0f;

// Kor mozgasanak valtozoi
float circleX = 0.0f;
float circleY = 0.0f;
float direction = 1.0f;
float speed = 0.01f;
bool isMoving = false;

// Alapertelmezett vizszintes mozgas sebessege
float defaultSpeed = 0.01f;

// Kepernyo szeleire pattanas logika valtozatlan marad
const float angle = 25.0f;  // Az iranyvektor szoge (25fok)
const float directionLength = 0.02f;  // 10 pixel normalt koordinatakban

// Iranyvektor kiszamitasa
float directionX = directionLength * cos(glm::radians(angle));  // X komponens
float directionY = directionLength * sin(glm::radians(angle));  // Y komponens



// Check if the circle intersects the horizontal line
bool checkIntersection(float cx, float cy, float radius, float lineY, float lineX1, float lineX2) {
    // Check if the circle's Y position is within range of the line's Y position ? radius
    if (cy - radius > lineY || cy + radius < lineY) {
        return false;  // The circle is too far away from the line
    }

    // Check if the circle's X position is within the line's X-range
    if (cx + radius >= lineX1 && cx - radius <= lineX2) {
        return true; // The circle intersects the line within X range
    }

    return false; // No intersection
}

void checkShaderCompilation(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cerr << "Hiba a shader forditasanal: " << infoLog << endl;
    }
}

void checkProgramLinking(GLuint program) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cerr << "Hiba a shader program linkelesenel: " << infoLog << endl;
    }
}


std::string readShaderFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer{};
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode = readShaderFile(vertexPath);
    std::string fragmentCode = readShaderFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty()) {
        return 0; // Return an invalid program if file reading failed
    }

    const char* vShaderSource = vertexCode.c_str();
    const char* fShaderSource = fragmentCode.c_str();

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderSource, NULL);
    glCompileShader(vShader);
    checkShaderCompilation(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderSource, NULL);
    glCompileShader(fShader);
    checkShaderCompilation(fShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);
    checkProgramLinking(program);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return program;
}

GLuint createLineShaderProgram() {
    return createShaderProgram("lineVertexShader.glsl", "lineFragmentShader.glsl");
}

GLuint createCircleShaderProgram() {
    return createShaderProgram("circleVertexShader.glsl", "circleFragmentShader.glsl");
}




void generateCirclePoints(glm::vec2 O, GLfloat r, GLint num_segment) {
    GLfloat x, y;
    GLfloat alpha = 0.0f;
    GLfloat full_circle = 2.0f * M_PI;

    circlePoints.push_back(glm::vec3(O.x, O.y, 0.0f));
    circleColors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

    for (int i = 0; i <= num_segment; i++) {
        x = O.x + r * cos(alpha);
        y = O.y + r * sin(alpha);

        circlePoints.push_back(glm::vec3(x, y, 0.0f));
        circleColors.push_back(glm::vec3(0.0f, 0.7f, 0.3f));

        alpha += full_circle / num_segment;
    }
}

void generateLinePoints() {
    float lineLength = 2.0f / 3.0f;  // Harmad oldalhosszusagu vonal
    float yPos = 0.0f;  // Kozepen elhelyezve

    // A vonal ket vegpontja
    linePoints.push_back(glm::vec3(-lineLength / 2.0f, yPos, 0.0f));  // Bal vegpont
    linePoints.push_back(glm::vec3(lineLength / 2.0f, yPos, 0.0f));   // Jobb vegpont

    // Vonal szinenek beallitasa (kek)
    lineColors.push_back(glm::vec3(0.0f, 0.0f, 1.0f));  // Elso pont szine
    lineColors.push_back(glm::vec3(0.0f, 0.0f, 1.0f));  // Masodik pont szine
}

// 'S' billentyu lenyomasa
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP) {
            lineY += 0.05f;  // Felfele mozgas
        }
        if (key == GLFW_KEY_DOWN) {
            lineY -= 0.05f;  // Lefele mozgas
        }
        if (key == GLFW_KEY_S) {
            isMoving = true;  // Az 'S' lenyomasaval inditjuk el az iranyvektorral valo mozgast
        }
    }
}





void init(GLFWwindow* window) {
    renderingProgram = createCircleShaderProgram();  // A korhzz
    lineShaderProgram = createLineShaderProgram();  // A vonalhoz

    // A tobbi inicializalas valtozatlan
    generateCirclePoints(glm::vec2(0.0f, 0.0f), circle_radius, 64);
    generateLinePoints();

    glGenBuffers(4, VBO);  // 4 VBO-t hasznalsz
    glGenVertexArrays(2, VAO);  // 2 VAO-t hasznalsz

    // kor inicializalasa
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, circlePoints.size() * sizeof(glm::vec3), circlePoints.data(), GL_STATIC_DRAW);

    glBindVertexArray(VAO[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, circleColors.size() * sizeof(glm::vec3), circleColors.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    // Vizszintes vonal inicializalasa
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, linePoints.size() * sizeof(glm::vec3), linePoints.data(), GL_STATIC_DRAW);

    glBindVertexArray(VAO[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, lineColors.size() * sizeof(glm::vec3), lineColors.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glClearColor(0.72f, 0.61f, 0.3f, 1.0f);  // Hatter szin beallitasa
}



void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);

    if (isMoving) {
        circleX += directionX;
        circleY += directionY;
        if (circleX + circle_radius >= 1.0f || circleX - circle_radius <= -1.0f) directionX *= -1.0f;
        if (circleY + circle_radius >= 1.0f || circleY - circle_radius <= -1.0f) directionY *= -1.0f;
    }
    else {
        circleX += defaultSpeed;
        if (circleX + circle_radius >= 1.0f || circleX - circle_radius <= -1.0f) defaultSpeed *= -1.0f;
    }

    glUseProgram(renderingProgram);
    GLuint circleCenterLoc = glGetUniformLocation(renderingProgram, "circleCenter");
    glUniform2f(circleCenterLoc, circleX, circleY);

    // Check if the circle intersects with the line (based on X-range and Y-distance)
    bool isIntersecting = checkIntersection(circleX, circleY, circle_radius, lineY, linePoints[0].x, linePoints[1].x);

    GLuint colorSwapLoc = glGetUniformLocation(renderingProgram, "colorSwap");
    glUniform1i(colorSwapLoc, isIntersecting); // If it intersects, color will change

    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circlePoints.size());
    glBindVertexArray(0);

    // Draw the line as usual
    glUseProgram(lineShaderProgram);
    GLuint lineYLoc = glGetUniformLocation(lineShaderProgram, "lineY");
    if (lineYLoc != -1) glUniform1f(lineYLoc, lineY);

    glLineWidth(3.0f);

    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}








int main(void) {
    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Goneth Zsolt 1.beadando", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) exit(EXIT_FAILURE);
    glfwSwapInterval(1);

    // Billentyuzet kezelo regisztralasa
    glfwSetKeyCallback(window, key_callback);

    init(window);

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteProgram(renderingProgram);
    glDeleteProgram(lineShaderProgram);


    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

