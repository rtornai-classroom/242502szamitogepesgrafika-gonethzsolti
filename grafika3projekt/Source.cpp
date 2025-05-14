#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


using namespace std;
GLuint sphereVAO, sphereVBO, sphereEBO;
GLuint lightSphereProgram;
int sphereIndexCount;




GLfloat kocka1[8 * 3] = {
	  0.5,  0.5, -0.5,
	 -0.5,  0.5, -0.5,
	  0.5, -0.5, -0.5,
	 -0.5, -0.5, -0.5,
	  0.5,  0.5,  0.5,
	 -0.5,  0.5,  0.5,
	 -0.5, -0.5,  0.5,
	  0.5, -0.5,  0.5
};

GLfloat kocka2[8 * 3] = {
	  0.5, 0.5, 1.5,
	  -0.5, 0.5, 1.5,
	  0.5, -0.5, 1.5,
	  -0.5, -0.5, 1.5,
	  0.5, 0.5, 2.5,
	  -0.5, 0.5, 2.5,
	  -0.5, -0.5, 2.5,
	  0.5, -0.5, 2.5
};
GLfloat kocka3[8 * 3] = {
	  0.5, 0.5, -2.5,
	  -0.5, 0.5, -2.5,
	  0.5, -0.5, -2.5,
	  -0.5, -0.5, -2.5,
	  0.5, 0.5, -1.5,
	  -0.5, 0.5, -1.5,
	  -0.5, -0.5, -1.5,
	  0.5, -0.5, -1.5
};


GLfloat colorData[8 * 3] = {
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
	1, 1, 1,
};


GLuint vertexDrawIndices[14] = {
   3,2,6,7,4,2,0,3,1,6,5,4,1,0
};

#define		numVBOs			6 
#define		numVAOs			3 
#define		numEBOs			1 

GLuint		VBO[numVBOs];
GLuint		VAO[numVAOs];
GLuint		EBO[numEBOs];

int			window_width = 600;
int			window_height = 600;
char		window_title[] = "3. beadando";
GLboolean	keyboard[512] = { GL_FALSE };
GLFWwindow* window = nullptr;
GLuint		renderingProgram;
GLuint		renderingProgram2;
GLuint		renderingProgram3;
GLuint		transformLoc;

GLuint		modelLoc;
GLuint		viewLoc;
GLuint		projectionLoc;



glm::mat4		model = glm::mat4(1.0f),
model2 = glm::mat4(1.0f),
model3 = glm::mat4(1.0f),
view,
//A vetítés legyen perspective			   55 értékkel.
projection = glm::perspective(glm::radians(55.0f),
	(float)window_width / (float)window_height, 0.1f, 100.0f);

GLdouble		deltaTime, lastTime = glfwGetTime();
GLfloat			cameraSpeed;

float r = 9.0f; // ahol (8 ≤ r ≤ 10).

glm::vec3
//A kamera kezdőpozíciója legyen (r; 0; 0), ahol (8 ≤ r ≤ 10).
cameraPos = glm::vec3(r, 0.0f, 0.0f),

// direction for camera
cameraDirection = glm::vec3(0.0f, 0.0f, 1.0f),

//Az UP vektorunk számára vegyük fel a (0; 0; 1) értéket.
cameraUpVector = glm::vec3(0.0f, 0.0f, 1.0f),

//A kamera mindig az origóba nézzen.
cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

//(0-360)
float forgatas_foka = 0;
bool lightingEnabled = false;
float lightAngle = 0.0f;
GLuint lightColorLoc, lightPosLoc, lightingEnabledLoc;
glm::vec3 lightPos;

std::vector<GLfloat> sphereVertices;
std::vector<GLuint> sphereIndices;

void generateSphere(float radius, int sectorCount, int stackCount)
{
	sphereVertices.clear();
	sphereIndices.clear();

	float x, y, z, xy;
	float sectorStep = 2 * M_PI / sectorCount;
	float stackStep = M_PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = M_PI / 2 - i * stackStep;
		xy = radius * cosf(stackAngle);
		z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;
			x = xy * cosf(sectorAngle);
			y = xy * sinf(sectorAngle);
			sphereVertices.push_back(x);
			sphereVertices.push_back(y);
			sphereVertices.push_back(z);
		}
	}

	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);
		k2 = k1 + sectorCount + 1;

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				sphereIndices.push_back(k1);
				sphereIndices.push_back(k2);
				sphereIndices.push_back(k1 + 1);
			}

			if (i != (stackCount - 1))
			{
				sphereIndices.push_back(k1 + 1);
				sphereIndices.push_back(k2);
				sphereIndices.push_back(k2 + 1);
			}
		}
	}
}



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

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
	string vertShaderStr = readShaderSource(vertexPath);
	string fragShaderStr = readShaderSource(fragmentPath);

	const char* vertShaderSrc = vertShaderStr.c_str();
	const char* fragShaderSrc = fragShaderStr.c_str();

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	glCompileShader(vShader);
	glCompileShader(fShader);

	GLint compiled;
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) printShaderLog(vShader);

	glGetShaderiv(fShader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) printShaderLog(fShader);

	GLuint program = glCreateProgram();
	glAttachShader(program, vShader);
	glAttachShader(program, fShader);
	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) printProgramLog(program);

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	return program;
}

GLuint createShaderProgram() {

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

void computeCameraMatrix() {
	view = glm::lookAt(cameraPos, cameraTarget, cameraUpVector);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void initLightSphere()
{
	generateSphere(1.0f, 16, 16);
	sphereIndexCount = sphereIndices.size();

	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glGenBuffers(1, &sphereEBO);

	glBindVertexArray(sphereVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	lightSphereProgram = createShaderProgram("lightSphereVertexShader.glsl", "lightSphereFragmentShader.glsl");
}

void init(GLFWwindow* window) {

	glEnable(GL_DEPTH_TEST);
	glGenVertexArrays(numVAOs, VAO);
	glGenBuffers(numVBOs, VBO);
	glGenBuffers(numEBOs, EBO);


	//Első kocka
	renderingProgram = createShaderProgram();

	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kocka1), kocka1, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexDrawIndices), vertexDrawIndices, GL_STATIC_DRAW); //* sizeof(GLuint)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(renderingProgram);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	modelLoc = glGetUniformLocation(renderingProgram, "model");
	viewLoc = glGetUniformLocation(renderingProgram, "view");
	projectionLoc = glGetUniformLocation(renderingProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//Első kocka vége



	//Második kocka
	renderingProgram2 = createShaderProgram();
	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(kocka2), kocka2, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexDrawIndices), vertexDrawIndices, GL_STATIC_DRAW); //* sizeof(GLuint)

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(renderingProgram);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	modelLoc = glGetUniformLocation(renderingProgram2, "model");
	viewLoc = glGetUniformLocation(renderingProgram2, "view");
	projectionLoc = glGetUniformLocation(renderingProgram2, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//Második kocka vége



	//Harmadik kocka
	renderingProgram3 = createShaderProgram();

	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(kocka3), kocka3, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexDrawIndices), vertexDrawIndices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(renderingProgram);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	modelLoc = glGetUniformLocation(renderingProgram2, "model");
	viewLoc = glGetUniformLocation(renderingProgram2, "view");
	projectionLoc = glGetUniformLocation(renderingProgram2, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//Harmadik kocka vége


	glClearColor(0.0, 0.0, 0.0, 1.0);



}



void display_kocka(int number_of_vao, GLuint program, glm::mat4 modelMatrix)
{
	glUseProgram(program);
	GLuint modelLoc = glGetUniformLocation(program, "model");
	GLuint viewLoc = glGetUniformLocation(program, "view");
	GLuint projectionLoc = glGetUniformLocation(program, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(VAO[number_of_vao]);
	glDrawElements(GL_TRIANGLE_STRIP, 14, GL_UNSIGNED_INT, vertexDrawIndices);
	glBindVertexArray(0);
}

void initLightingUniforms(GLuint program) {
	lightColorLoc = glGetUniformLocation(program, "lightColor");
	lightPosLoc = glGetUniformLocation(program, "lightPos");
	lightingEnabledLoc = glGetUniformLocation(program, "lightingEnabled");
}

void updateLightingUniforms(GLuint program, float r, float currentTime) {
	float radius = 2.0f * r;
	lightPos = glm::vec3(radius * cos(lightAngle), radius * sin(lightAngle), 0.0f); // <-- ide
	glm::vec3 lightColor(1.0f, 0.6f, 0.3f);
	glUseProgram(program);
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
	glUniform1i(lightingEnabledLoc, lightingEnabled ? 1 : 0);
}


void display(GLFWwindow* window, double currentTime) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	cameraSpeed = 2.5f * (GLfloat)deltaTime;

	//A kamerát a z-tengely mentén fel-le tudjuk mozgatni a fel-le nyíl billentyűkkel.
	if ((keyboard[GLFW_KEY_UP])) {
		//cameraTarget -= cameraSpeed * cameraDirection;
		//cameraPos -= cameraSpeed * cameraDirection;

		forgatas_foka += (cameraSpeed * 0.5f);
		cameraPos.z = r * sin(forgatas_foka);
		cameraPos.x = r * cos(forgatas_foka);
	}

	//A kamerát a z-tengely mentén fel-le tudjuk mozgatni a fel-le nyíl billentyűkkel.
	if ((keyboard[GLFW_KEY_DOWN])) {
		//cameraTarget += cameraSpeed * cameraDirection;
		//cameraPos += cameraSpeed * cameraDirection;

		forgatas_foka -= (cameraSpeed * 0.5f);
		cameraPos.z = r * sin(forgatas_foka);
		cameraPos.x = r * cos(forgatas_foka);

	}

	//A kamerát lehessen mozgatni a z-tengely körül, az r sugarú körön a jobb-bal nyilakkal.
	if ((keyboard[GLFW_KEY_LEFT])) {
		forgatas_foka += (cameraSpeed * 0.5f);
		cameraPos.y = r * sin(forgatas_foka);
		cameraPos.x = r * cos(forgatas_foka);
	}

	//A kamerát lehessen mozgatni a z-tengely körül, az r sugarú körön a jobb-bal nyilakkal.
	if ((keyboard[GLFW_KEY_RIGHT])) {
		forgatas_foka -= (cameraSpeed * 0.5f);
		cameraPos.y = r * sin(forgatas_foka);
		cameraPos.x = r * cos(forgatas_foka);
	}

	computeCameraMatrix();

	//1.kocka
	display_kocka(0, renderingProgram, model);
	display_kocka(1, renderingProgram2, model2);
	display_kocka(2, renderingProgram3, model3);
	//3.kocka vége

	glBindVertexArray(0);
	updateLightingUniforms(renderingProgram, r, currentTime);
	updateLightingUniforms(renderingProgram2, r, currentTime);
	updateLightingUniforms(renderingProgram3, r, currentTime);

	glUseProgram(lightSphereProgram);

	glm::mat4 sphereModel = glm::translate(glm::mat4(1.0f), lightPos);
	sphereModel = glm::scale(sphereModel, glm::vec3(0.25f)); // átmérő = 0.5

	glUniformMatrix4fv(glGetUniformLocation(lightSphereProgram, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
	glUniformMatrix4fv(glGetUniformLocation(lightSphereProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(lightSphereProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// A szín legyen megegyező a világítással (pl. warmLight)
	glm::vec3 warmLightColor = glm::vec3(1.0f, 0.55f, 0.1f);
	glUniform3fv(glGetUniformLocation(lightSphereProgram, "color"), 1, glm::value_ptr(warmLightColor));

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}



void cleanUpScene() {

	glDeleteVertexArrays(numVAOs, VAO);
	glDeleteBuffers(numVBOs, VBO);
	glDeleteBuffers(numEBOs, EBO);

	glDeleteProgram(renderingProgram);

	glfwDestroyWindow(window);

	glfwTerminate();

	exit(EXIT_SUCCESS);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	window_width = width;
	window_height = height;

	glViewport(0, 0, width, height);

	projection = glm::perspective(glm::radians(55.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);

	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);
	//		cleanUpScene();

	if (action == GLFW_PRESS) {
		keyboard[key] = GL_TRUE;
		if (key == GLFW_KEY_L)
			lightingEnabled = !lightingEnabled;
	}
	else if (action == GLFW_RELEASE) {
		keyboard[key] = GL_FALSE;
	}
}




int main(void) {
	glEnable(GL_DEPTH_TEST);
	if (!glfwInit()) exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	if (glewInit() != GLEW_OK) exit(EXIT_FAILURE);
	glfwSwapInterval(1);

	init(window);
	initLightSphere();

	initLightingUniforms(renderingProgram);
	initLightingUniforms(renderingProgram2);
	initLightingUniforms(renderingProgram3);

	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();
		lightAngle += 1.0f * (currentTime - lastTime);
		display(window, currentTime);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanUpScene();
	return EXIT_SUCCESS;
}