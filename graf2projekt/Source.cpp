#include <array>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

using namespace std;

GLFWwindow* window = nullptr;
int			window_width = 600;
int			window_height = 600;
char		window_title[] = "Goneth Zsolt 2.beadando";

std::vector<glm::vec3> pointToDraw;

std::vector<glm::vec3> myControlPoints = {
	glm::vec3(-0.5f, -0.4f, 0.0f),
	glm::vec3(-0.5f,  0.5f, 0.0f),
	glm::vec3(0.5f, -0.4f, 0.0f),
	glm::vec3(0.5f,  0.5f, 0.0f),
};

float lineSize = 300.0f;
int windowSize = 600;

glm::vec3 controlPointsColor = glm::vec3(0.7f, 6.0f, 0.0f);
glm::vec3 controlPoligonColor = glm::vec3(0.4f, 0.3f, 0.8f);
glm::vec3 bezierCurveColor = glm::vec3(0.5f, 0.1f, 0.0f);

#define		numVBOs 2
#define		numVAOs 2

GLuint		VBO[numVBOs];
GLuint		VAO[numVAOs];
GLuint		colorLocation;
GLuint		isPointLocation;
GLuint		renderingProgram;
GLint		dragged = -1;
bool		added = false;
bool		removed = false;

bool checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}

void printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}

void printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}

string readShaderSource(const char* filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";

	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

GLuint createShaderProgram()
{
	GLint vertCompiled;
	GLint fragCompiled;
	GLint linked;

	string vertShaderStr = readShaderSource("vertexShader.glsl");
	string fragShaderStr = readShaderSource("fragmentShader.glsl");

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vertShaderSrc = vertShaderStr.c_str();
	const char* fragShaderSrc = fragShaderStr.c_str();

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	glCompileShader(vShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
	if (vertCompiled != 1) {
		cout << "vertex compilation failed" << endl;
		printShaderLog(vShader);
	}

	glCompileShader(fShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1) {
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);

	glLinkProgram(vfProgram);
	checkOpenGLError();
	glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
	if (linked != 1) {
		cout << "linking failed" << endl;
		printProgramLog(vfProgram);
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	return vfProgram;
}

void cleanUpScene() {
	glDeleteVertexArrays(numVAOs, VAO);
	glDeleteBuffers(numVBOs, VBO);
	glDeleteProgram(renderingProgram);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

int NCR(int n, int r) {
	if (r == 0) return 1;
	if (r > n / 2) return NCR(n, n - r);
	long res = 1;
	for (int k = 1; k <= r; ++k) {
		res *= n - k + 1;
		res /= k;
	}

	return res;
}


GLfloat blending(GLint n, GLint i, GLfloat t) {
	return NCR(n, i) * pow(t, i) * pow(1.0f - t, n - i);
}

void drawBezierCurve(std::vector<glm::vec3> controlPoints) {
	glm::vec3 nextPoint;
	GLfloat t = 0.0f;
	GLfloat B;
	GLfloat increment = 1.0f / lineSize;
	pointToDraw.erase(pointToDraw.begin(), pointToDraw.end());
	while (t <= 1.0f) {
		nextPoint = glm::vec3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < controlPoints.size(); i++) {
			B = blending(controlPoints.size() - 1, i, t);
			nextPoint.x += B * controlPoints.at(i).x;
			nextPoint.y += B * controlPoints.at(i).y;
			nextPoint.z += B * controlPoints.at(i).z;
		}
		pointToDraw.push_back(nextPoint);
		t += increment;
	}
}


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE)) cleanUpScene();
}

void attachBuffers(void) {
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, myControlPoints.size() * sizeof(glm::vec3), myControlPoints.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	drawBezierCurve(myControlPoints);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, pointToDraw.size() * sizeof(glm::vec3), pointToDraw.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
	if (dragged >= 0) {
		GLfloat	xNorm = xPos / ((GLfloat)window_width / 2.0f) - 1.0f;
		GLfloat	yNorm = ((GLfloat)window_height - yPos) / ((GLfloat)window_height / 2.0f) - 1.0f;
		myControlPoints.at(dragged).x = xNorm;
		myControlPoints.at(dragged).y = yNorm;
		attachBuffers();
	}
}

GLfloat pontDistance(glm::vec3 P1, glm::vec3 P2) {
	GLfloat		dx = P1.x - P2.x;
	GLfloat		dy = P1.y - P2.y;

	return dx * dx + dy * dy;
}

GLint getActivePoint(vector<glm::vec3> p, GLfloat sensitivity, GLfloat x, GLfloat y) {
	GLfloat		s = sensitivity * sensitivity;
	GLint		size = p.size();
	GLfloat		xNorm = x / ((GLfloat)window_width / 2.0) - 1.0f;
	GLfloat		yNorm = y / ((GLfloat)window_height / 2.0) - 1.0f;
	glm::vec3	mousePos = glm::vec3(xNorm, yNorm, 0.0f);

	for (GLint i = 0; i < size; i++)
		if (pontDistance(p[i], mousePos) < s)
			return i;
	return -1;
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	// Bal egérgomb lenyomására
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double	x, y;
		glfwGetCursorPos(window, &x, &y);
		dragged = getActivePoint(myControlPoints, 0.1f, x, window_height - y);
		if (dragged < 0 && !added) {
			GLfloat	xNorm = x / ((GLfloat)window_width / 2.0f) - 1.0f;
			GLfloat	yNorm = ((GLfloat)window_height - y) / ((GLfloat)window_height / 2.0f) - 1.0f;
			myControlPoints.push_back(glm::vec3(xNorm, yNorm, 0.0f));
			attachBuffers();
			added = true;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		dragged = -1;
		added = false;
	}
	if (!removed && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		double	x, y;
		glfwGetCursorPos(window, &x, &y);
		GLint PointID = -1;
		PointID = getActivePoint(myControlPoints, 0.1f, x, window_height - y);
		if (PointID >= 0) {
			myControlPoints.erase(myControlPoints.begin() + PointID);
			removed = true;
			attachBuffers();
		}
	}
	// Jobb egérgomb felengedésére
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) removed = false;
}


void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	window_width = width;
	window_height = height;
	glViewport(0, 0, width, height);
}

void init(GLFWwindow* window) {
	renderingProgram = createShaderProgram();
	drawBezierCurve(myControlPoints);
	glGenBuffers(numVBOs, VBO);
	glGenVertexArrays(numVAOs, VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, pointToDraw.size() * sizeof(glm::vec3), pointToDraw.data(), GL_STATIC_DRAW);
	glBindVertexArray(VAO[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, myControlPoints.size() * sizeof(glm::vec3), myControlPoints.data(), GL_STATIC_DRAW);
	glBindVertexArray(VAO[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	colorLocation = glGetUniformLocation(renderingProgram, "color");
	isPointLocation = glGetUniformLocation(renderingProgram, "isPoint");


	glUseProgram(renderingProgram);

	glClearColor(0.9, 0.7, 0.2, 0.1);
}


void display(GLFWwindow* window, double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_POINT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(12.0f);
	glLineWidth(4.0f);

	// Draw control polygon (lines)
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBindVertexArray(VAO[1]);

	glProgramUniform1i(renderingProgram, isPointLocation, 0);  // Not point rendering
	glProgramUniform3f(renderingProgram, colorLocation, controlPoligonColor.r, controlPoligonColor.g, controlPoligonColor.b);
	glDrawArrays(GL_LINE_STRIP, 0, myControlPoints.size());

	// Draw control points
	glProgramUniform1i(renderingProgram, isPointLocation, 1);  // Now rendering points
	glProgramUniform3f(renderingProgram, colorLocation, controlPointsColor.r, controlPointsColor.g, controlPointsColor.b);
	glDrawArrays(GL_POINTS, 0, myControlPoints.size());

	glBindVertexArray(0);

	// Draw Bézier curve
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBindVertexArray(VAO[0]);

	glProgramUniform1i(renderingProgram, isPointLocation, 0);  // Line strip again
	glProgramUniform3f(renderingProgram, colorLocation, bezierCurveColor.r, bezierCurveColor.g, bezierCurveColor.b);
	glDrawArrays(GL_LINE_STRIP, 0, pointToDraw.size());

	glBindVertexArray(0);

}


int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(windowSize, windowSize, window_title, NULL, NULL);

	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);
	glfwSetWindowSizeLimits(window, 600, 600, 600, 600);

	init(window);

	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanUpScene();
}