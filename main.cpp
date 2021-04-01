#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightDirMatrix;
glm::mat4 lightRotation;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

glm::vec4 pointSource;
GLuint pointSourceLoc;
glm::vec3 pointColor;
GLuint pointColorLoc;

glm::vec4 wolfSource;
GLuint wolfSourceLoc;
glm::vec3 wolfColor;
GLuint wolfColorLoc;


gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.02f;

GLboolean pressedKeys[1024];

gps::Model3D ground;
gps::Model3D structures;
gps::Model3D campsite;

gps::Model3D bow;
gps::Model3D arrow;

gps::Model3D trees;
gps::Model3D scalableTrees;

gps::Model3D boat;
gps::Model3D bucketWater;

gps::Model3D cat;
gps::Model3D horse;
gps::Model3D wolf;

gps::Model3D ducks;
gps::Model3D rotatedDuck;
gps::Model3D staticDucks;

gps::Model3D plane;

gps::Model3D lantern;
gps::Model3D screenQuad;

gps::Model3D lightCube;

GLfloat angle = 0;

gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader lightCubeShader;
gps::Shader screenQuadShader;
gps::Shader shaderStart;
gps::Shader skyBoxShader;

gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;

GLuint shadowMapFBO;
GLuint depthMapTexture;

const GLuint SHADOW_WIDTH = 1024;
const GLuint SHADOW_HEIGHT = 1024;

float pitch = 0;
float yaw = -90;

bool firstMouse = true;
double xdiff = 0;
double ydiff = 0;
double xposs = 0;
double yposs = 0;
double lastX = 400;
double lastY = 300;

bool depth = true;
bool showDepthMap = false;

float moveBoat = 0.0f;
int rotateBoat = 0;

bool toScaleTrees = false;
float scaleTrees = 0.0f;

bool shootArrow = false;
bool shot = false;
bool arrowDistance = 0.0f;

bool toMoveDucks = false;
float moveDucks = 0.0f;

bool toMovePlane = false;
bool movePlane = 0.0f;

bool beginPresentation = false;
int goFront = 1;
int goRight = 0;
int goLeft = 0;
int goCenter = 0;
int rotateCam = 0;
int count = 0;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);

    WindowDimensions dimensions;
    dimensions.width = width;
    dimensions.height = height;

    myWindow.setWindowDimensions(dimensions);
    shaderStart.useShaderProgram();
    
    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(shaderStart.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 359.0f) {
        pitch = 359.0f;
    }

    if (pitch < -359.0f) {
        pitch = -359.0f;
    }

    myCamera.rotate(pitch, yaw);
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_UP]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_DOWN]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_J]) {
        angle += 1.0f;
        if (angle > 360.0f)
            angle -= 360.0f;
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));

    }

    if (pressedKeys[GLFW_KEY_K]) {
        angle -= 1.0f;
        if (angle < 0.0f)
            angle += 360.0f;
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));

    }

    if (pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        myCamera.setStartingPosition();
        beginPresentation = true;
    }

    if (pressedKeys[GLFW_KEY_R]) {
        showDepthMap = true;
    }

    if (pressedKeys[GLFW_KEY_C]) {
        toScaleTrees = true;
        scaleTrees += 0.01f;
    }

    if (pressedKeys[GLFW_KEY_V]) {
        toScaleTrees = true;
        scaleTrees -= 0.01f;
    }

    if (pressedKeys[GLFW_KEY_B]) {
        shootArrow = true;
    }

    if (pressedKeys[GLFW_KEY_N]) {
        shootArrow = false;
        arrowDistance = 0.0f;
    }

    if (pressedKeys[GLFW_KEY_T]) {
        toMoveDucks = true;
    }



}

void presentScene() {

    if (beginPresentation == true) {
        glm::vec3 pos = myCamera.getCameraPosition();

        if (goFront == 1) {
            if (pos.z > 1.3) {
                myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
            }
            else {
                goFront = 0;
                goRight = 1;
            }
        }
        
        if (goRight == 1) {
            if (pos.x < 3.2) {
                myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
            }
            else {
                goRight = 0;
                goLeft = 1;
            }
        }

        if (goLeft == 1) {
            if (pos.x > -3.3) {
                myCamera.move(gps::MOVE_LEFT, cameraSpeed);
            }
            else {
                goLeft = 0;
                goCenter = 1;
            }

        }

        if (goCenter == 1) {
            if (pos.x < 0.0f) {
                myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
            }
            else {
                goCenter = 0;
                beginPresentation = false;
            }
        }

    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs() {
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initModels() {

    //environment
    ground.LoadModel("models/ground/ground4.obj");
    structures.LoadModel("models/buildings/structures.obj");
    campsite.LoadModel("models/buildings/campsite.obj");
    trees.LoadModel("models/trees/staticTrees.obj");
    cat.LoadModel("models/cat/cats.obj");
    horse.LoadModel("models/horse/horse.obj");
    bow.LoadModel("models/bow and arrow/bow.obj");
    staticDucks.LoadModel("models/duck/staticDucks.obj");
    rotatedDuck.LoadModel("models/duck/rotateDuck.obj");

    //moving
    boat.LoadModel("models/boat/boat.obj");
    ducks.LoadModel("models/duck/movingDucks.obj");
    arrow.LoadModel("models/bow and arrow/arrow.obj");
    scalableTrees.LoadModel("models/trees/scalableTrees.obj");
    bucketWater.LoadModel("models/ground/bucketWater.obj");
    plane.LoadModel("models/plane/plane.obj");
    
    //lighting 
    lantern.LoadModel("models/lantern/lantern.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
    wolf.LoadModel("models/cat/wolf.obj");

   
   
}

void initSkyBox() {
    faces.push_back("skybox/bluecloud_rt.tga");
    faces.push_back("skybox/bluecloud_lf.tga");
    faces.push_back("skybox/bluecloud_up.tga");
    faces.push_back("skybox/bluecloud_dn.tga");
    faces.push_back("skybox/bluecloud_bk.tga");
    faces.push_back("skybox/bluecloud_ft.tga");
}

void initShaders() {
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
    lightCubeShader.loadShader("shaders/lightCubeShader.vert", "shaders/lightCubeShader.frag");
    screenQuadShader.loadShader("shaders/screenQuadShader.vert", "shaders/screenQuadShader.frag");
    shaderStart.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    skyBoxShader.loadShader("shaders/skyBoxShader.vert", "shaders/skyBoxShader.frag");
}

void initUniforms() {
	shaderStart.useShaderProgram();

    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(shaderStart.shaderProgram, "model");

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(shaderStart.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(shaderStart.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(shaderStart.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	lightDir = glm::vec3(0.0f, 1.0f, 3.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(shaderStart.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); 
	lightColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    pointColor = glm::vec3(0.0f, 0.0f, 1.0f);
    pointColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "pointLightColor");
    glUniform3fv(pointColorLoc, 1, glm::value_ptr(pointColor));
    pointSourceLoc = glGetUniformLocation(shaderStart.shaderProgram, "pointLightSource");

    wolfColor = glm::vec3(1.0f, 0.0f, 1.0f);
    wolfColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "wolfLightColor");
    glUniform3fv(wolfColorLoc, 1, glm::value_ptr(wolfColor));
    wolfSourceLoc = glGetUniformLocation(shaderStart.shaderProgram, "wolfLightSource");

    lightCubeShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    
    mySkyBox.Load(faces);
    skyBoxShader.useShaderProgram();


}

void computePointLight() {
    pointSource = glm::translate(glm::mat4(1.0f), glm::vec3(2.8f, 0.5f, 0.6f)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform3fv(pointSourceLoc, 1, glm::value_ptr(view * pointSource));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void computeWolfLight() {
    wolfSource = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, -5.0f)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform3fv(wolfSourceLoc, 1, glm::value_ptr(view * wolfSource));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

glm::mat4 computeLightSpaceTrMatrix() {
    const GLfloat near_plane = 0.1f;
    const GLfloat far_plane = 50.0f;

    glm::mat4 lightProjection = glm::ortho(-7.0f, 7.0f, -7.0f, 7.0f, near_plane, far_plane);
    glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
    glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::mat4 lightView = glm::lookAt(lightDirTr, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return lightProjection * lightView;
}

void renderSkyBox() {
    skyBoxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    mySkyBox.Draw(skyBoxShader, view, projection);
}


void renderLightCube() {
    lightCubeShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, 1.0f * lightDir);
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(lightCubeShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    lightCube.Draw(lightCubeShader);

}

void renderGround(gps::Shader shader) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    ground.Draw(shader);
}

void renderTrees(gps::Shader shader) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    trees.Draw(shader);
}

void renderScalableTrees(gps::Shader shader) {

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.6f, 0.0f));
    if (toScaleTrees == true) {
        model = glm::scale(model, glm::vec3(1.0f + scaleTrees, 1.0f + scaleTrees, 1.0f + scaleTrees));
    }

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    scalableTrees.Draw(shader);
}


void renderStructures(gps::Shader shader) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    structures.Draw(shader);
}

void renderCampsite(gps::Shader shader) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    campsite.Draw(shader);
}

void renderCat(gps::Shader shader) {
    model = glm::rotate(glm::mat4(1.0f), glm::radians(-10.0f), glm::vec3(-0.2f, 1.2f, 1.0f));
    model = glm::translate(model, glm::vec3(-1.3f, -0.3f, 0.0f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    cat.Draw(shader);
}

void renderHorse(gps::Shader shader) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    horse.Draw(shader);
}

void renderBow(gps::Shader shader) {
    model = glm::rotate(glm::mat4(1.0f), glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.8f, -0.6f, 1.5f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    bow.Draw(shader);
}

void renderPlane(gps::Shader shader) {

    if (toMovePlane == true) {
        if (movePlane >= 10.0) {
            movePlane = 0.0f;
        }
        else {
            movePlane = movePlane + 0.05;
        }
    }

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-moveBoat, -0.5f, 2.9f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    plane.Draw(shader);
}

void renderLantern(gps::Shader shader) {
    model = glm::rotate(glm::mat4(1.0f), glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.8f, -0.6f, -1.5f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    lantern.Draw(shader);
}

void renderWolf(gps::Shader shader) {
    model = glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), glm::vec3(1.0f, 0.0f, 1.0f));
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    wolf.Draw(shader);
}

void renderArrow(gps::Shader shader) {
    
    if (shootArrow == true) {
       arrowDistance += 0.05f;
    }
    float move = 1.5f - arrowDistance;
    
    model = glm::rotate(glm::mat4(1.0f), glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.8f, -0.6f, 1.5f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    arrow.Draw(shader);
}

void renderBoat(gps::Shader shader) {
    switch (rotateBoat) {
    case 0:
        if (moveBoat >= 2.0)
            rotateBoat = 1;
        else
            moveBoat += 0.005f;
        break;
    case 1:
        if (moveBoat <= 0.0f)
            rotateBoat = 0;
        else
            moveBoat -= 0.005f;
        break;
    }

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-moveBoat, -0.45f, 2.9f));
    
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    boat.Draw(shader);
}

void renderDucks(gps::Shader shader) {
    
    if (toMoveDucks == true) {
        moveDucks -= 0.001f;
        if (moveDucks <= 1.5f) {
            toMoveDucks = false;
        }
    }
    model = glm::translate(glm::mat4(1.0f), glm::vec3(moveDucks, -0.5f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    ducks.Draw(shader);
}

void renderRotatedDuck(gps::Shader shader) {
   
    model = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.05f, -1.55f, -0.9f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    rotatedDuck.Draw(shader);
}

void renderStaticDucks(gps::Shader shader){
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (depth == false) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    staticDucks.Draw(shader);
}

void renderScene() {

    processMovement();
    presentScene();

    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    depth = true;

    renderGround(depthMapShader);
    renderStructures(depthMapShader);
    renderCampsite(depthMapShader);

    renderCat(depthMapShader);
    renderHorse(depthMapShader);

    renderStaticDucks(depthMapShader);
    renderRotatedDuck(depthMapShader);

    renderLantern(depthMapShader);
    renderWolf(depthMapShader);

    renderScalableTrees(depthMapShader);
    renderDucks(depthMapShader);
    renderBoat(depthMapShader);
    
    renderBow(depthMapShader);
    renderArrow(depthMapShader);
    renderPlane(depthMapShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    if (showDepthMap) {
        glViewport(0, 0, (float) myWindow.getWindowDimensions().width, (float) myWindow.getWindowDimensions().height);
        glClear(GL_COLOR_BUFFER_BIT);
        screenQuadShader.useShaderProgram();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderStart.useShaderProgram();
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "shadowMap"), 3);
        glUniformMatrix4fv(glGetUniformLocation(shaderStart.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

        computePointLight();
        computeWolfLight();

        depth = false;

        renderGround(shaderStart);
        renderStructures(shaderStart);
        renderCampsite(shaderStart);

        renderCat(shaderStart);
        renderHorse(shaderStart);

        renderStaticDucks(shaderStart);
        renderRotatedDuck(shaderStart);

        renderLantern(shaderStart);
        renderWolf(shaderStart);

        renderBoat(shaderStart);
        renderDucks(shaderStart);
        renderScalableTrees(shaderStart);

        renderBow(shaderStart);
        renderArrow(shaderStart);
        renderPlane(shaderStart);
        
        renderLightCube();
    }
    
    renderSkyBox();
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initFBOs();
    initSkyBox();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
