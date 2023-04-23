/**
*
* Much of the code was inspired form the following tutorials:
* https://www.youtube.com/watch?v=yNYwZMmgTJk
* https://www.youtube.com/watch?v=O9v6olrHPwI
* https://www.youtube.com/watch?v=qChQrNWU9Xw
*
*
*
**/

#include <iostream>
#include <math.h>
#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <numbers>
#include "GLInitializations.h"
#include "CyCodeBase/cyTriMesh.h"
#include "CyCodeBase/cyCore.h"
#include "CyCodeBase/cyVector.h"
#include "CyCodeBase/cyMatrix.h"
#include "CyCodeBase/cyGL.h"
#include "lodepng/lodepng.h"
#include "lodepng/lodepng.cpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "Mesh/Mesh.h"

void createOpenGLWindow(int width, int height);
void drawNewFrame();
void keyboardInterrupt(unsigned char key, int x, int y);
void mouseButtonTracker(int button, int state, int x, int y);
void mouseClickDrag(int x, int y);
void specialInput(int key, int x, int y);
void idleCallback();
void createScenePlane(GLuint& planeVao, int size);
Mesh** createSceneTerrain(GLuint& terrainVao, int mapSize);
void setRotationAndDistance(float& xRot, float& yRot, float& zRot);
float DEG2RAD(float degrees);
float RAD2DEG(float radians);
void drawPoint(float x, float y, float z);

bool leftMouse, GeoMeshToggle;
float movementSpeed;
int mouseX, mouseY;
int windowWidth, windowHeight;
GLuint terrainVao;
cy::GLSLProgram planeShaders;
cy::GLSLProgram wireMeshShaders;
Mesh terrain;
cy::Vec3f camPos;
cy::Vec3f cameraFront;


int main(int argc, char* argv[])
{

	/**
	*
	* Initializations
	*
	**/
	leftMouse = false;
	GeoMeshToggle = false;
	mouseX = 0; mouseY = 0;
	int mapSize = 400;    // this sets the side length of the terrain to be generated
	movementSpeed = 3.0f;
	camPos = cy::Vec3f(0.0f, 3.0f, 0.0f);

	// if there is not a png normal map then exit - simple check
	if (argc < 2) { exit(0); }

	// Initialize FreeGLUT
	glutInit(&argc, argv);
	glutInitContextVersion(4, 5);
	glutInitContextFlags(GLUT_DEBUG);

	// initalize a new window
	windowWidth = 1280;
	windowHeight = 720;
	createOpenGLWindow(windowWidth, windowHeight);

	//initialize glew
	GLenum res = glewInit();
	// Error code sourced from: https://youtu.be/6dtqg0r28Yc
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	CY_GL_REGISTER_DEBUG_CALLBACK;

	/**
	*
	* Register functiuons for GLUT
	*
	**/
	glutDisplayFunc(drawNewFrame);
	glutKeyboardFunc(keyboardInterrupt);
	glutIdleFunc(idleCallback);
	glutMouseFunc(mouseButtonTracker);
	glutMotionFunc(mouseClickDrag);
	glutSpecialFunc(specialInput);

	// OpenGL initializations
	GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f; // sourced from: https://youtu.be/6dtqg0r28Yc
	glClearColor(Red, Green, Blue, Alpha);


	/**
	*
	* Initialize Objects to be rendered
	*
	**/
	// createScenePlane(terrainVao, mapSize);   - deprecated for now
	std::cout << "Generating Terrain..." << std::endl;

	terrain = Mesh();
	terrain.generateVertices(mapSize, mapSize);
	createSceneTerrain(terrainVao, mapSize);
	// createScenePlane(terrainVao, mapSize);
	std::cout << "Done" << std::endl;

	// useTesselation = false;
	/**
	*
	* Compile shaders
	*
	**/
	// initialize CyGL
	planeShaders.BuildFiles("Shaders\\shader.vert", "Shaders\\shader.frag");
	wireMeshShaders.BuildFiles("Shaders\\Passthrough.vert", "Shaders\\SimpleTexture.frag", "Shaders\\wire.geom");

	// clear scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	std::cout << "Finished drawing first frame. Entering main loop\n";
	char t;
	//std::cin >> t;
	// Call main loop
	glutMainLoop();
	return 0;
}





/******************************************************************************
*
*
*                               GL CALLBACK FUNCTIONS
*
*
********************************************************************************/


/**
*
* GLUT Display Callback Function - Do not directly call.
*
**/
void drawNewFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// render plane under argument object (also used for testing as a plane to render depth map to)

	glBindVertexArray(terrainVao);
	int numVerts = terrain.getVertices().size();

	if (GeoMeshToggle)
	{
		// draw triangulation plane
		wireMeshShaders.Bind();
		glDrawArrays(GL_TRIANGLES, 0, numVerts);
	}

	// draw plane normally
	planeShaders.Bind();
	glDrawArrays(GL_TRIANGLES, 0, numVerts);

	// drawPoint(2, 0, 2);

	glutSwapBuffers();
	return;
}


/**
*
* GLUT Keyboard Interrupt Callback Function - Do not directly call.
*
* param:
* key - ascii value of key pressed
* x - x coordinate of mouse pointer at time of press
* y - y coordinate of mouse pointer at time of press
*
**/
void keyboardInterrupt(unsigned char key, int x, int y)
{
	// used for calculating right and left
	cy::Vec3f up(0.0f, 1.0f, 0.0f);

	switch (key) {
	case 27:    // escape key
		std::cout << "User pressed escape key.\n";
		glutLeaveMainLoop();
		break;
	case 32:    // spacebar
		camPos.y += movementSpeed;
		break;
	case 87:    // W
	case 119:
		std::cout << "W\n";
		camPos += (cameraFront * movementSpeed);
		break;
	case 65:    // A
	case 97:
		std::cout << "A\n";
		camPos += cy::Normalize(up.Cross(cameraFront)) * movementSpeed;
		break;
	case 83:    // S
	case 115:
		std::cout << "S\n";
		camPos -= (cameraFront * movementSpeed);
		break;
	case 68:    // D
	case 100:
		std::cout << "D\n";
		// use negative up to get vector pointing right of camera
		camPos += cy::Normalize((-up).Cross(cameraFront)) * movementSpeed;
		break;
	case 71:	// G
	case 103:
		std::cout << "User Toggled Wire Mesh\n";
		GeoMeshToggle = !GeoMeshToggle;
	}
	// check shift button which moves player down
	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
	{
		camPos.y -= movementSpeed;
	}
	return;
}


/**
*
* GLUT Keyboard Interrupt Callback Function - Do not directly call.
*
* param:
* key - ascii value of key pressed
* x - x coordinate of mouse pointer at time of press
* y - y coordinate of mouse pointer at time of press
*
**/
void mouseButtonTracker(int button, int state, int x, int y)
{

	switch (button) {
	case 0:
		leftMouse = !state;
		break;
	case 2:
		// this indicates right mouse button was pressed
		break;
	}
	return;
}


/**
*
* GLUT Keyboard Interrupt Callback Function - Do not directly call.
*
* param:
* key - ascii value of key pressed
* x - x coordinate of mouse pointer at time of press
* y - y coordinate of mouse pointer at time of press
*
**/
void mouseClickDrag(int x, int y)
{
	mouseX = x;
	mouseY = y;
	return;
}

/// <summary>
/// method sourced from: https://community.khronos.org/t/what-are-the-codes-for-arrow-keys-to-use-in-glut-keyboard-callback-function/26457
/// </summary>
void specialInput(int key, int x, int y)
{
	int maxTesselation = 32;

	switch (key)
	{
	case GLUT_KEY_LEFT:
		// by casting to unsigned char first we clamp tessLevel between 0 and 255
		// left arrow key was pressed
		break;
	case GLUT_KEY_RIGHT:
		// right arrow key was pressed
		break;
	}
}


/**
*
* GLUT Idle Callback Function - Do not directly call.
**/
void idleCallback()
{
	// static initializations
	static float yRot = 0.0f;
	static float xRot = 0.0f;
	static float zRot = 0.0f;

	setRotationAndDistance(xRot, yRot, zRot);

	//cy::Matrix3f rotMatrix = cy::Matrix3f::RotationXYZ(yRot, xRot, zRot);
	float halfWidth = terrain.getMeshWidth() / 2;
	cy::Matrix4f centerMeshOnWorld = cy::Matrix4f::Translation(cy::Vec3f(-halfWidth, 0.0f, -halfWidth));
	// define the scale of the plane to fit the size of the current scene objects
	cy::Matrix4f planeScale = cy::Matrix4f::Scale(cy::Vec3f(1.0f, 1.0f, 1.0f));
	cy::Matrix4f planeModel = planeScale * centerMeshOnWorld;
	
	// define where to look at
	// source: https://learnopengl.com/Getting-started/Camera
	cy::Vec3f direction;
	direction.x = cos(xRot) * cos(yRot);
	direction.y = sin(yRot);
	direction.z = sin(xRot) * cos(yRot);
	cameraFront = Normalize(direction);

	cy::Matrix4f view = cy::Matrix4f::View(camPos, camPos + cameraFront, cy::Vec3f(0.0f, 1.0f, 0.0f));
	cy::Matrix4f projMatrix = cy::Matrix4f::Perspective(DEG2RAD(90), float(windowWidth) / float(windowHeight), 0.1f, 1000.0f);

	// translation matrix inteded to be used to prevent z-fighting between the actual plane and it's wire mesh
	cy::Matrix4f VerticalTrans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.1f, 0.0f));

	cy::Vec3f lightPos = cy::Vec3f(0.0f, 100.0f, 20.0f);

	planeShaders["viewPos"] = camPos;
	planeShaders["model"] = planeModel;
	planeShaders["view"] = view;
	planeShaders["projection"] = projMatrix;
	planeShaders["lightPos"] = lightPos;

	wireMeshShaders["viewPos"] = camPos;
	wireMeshShaders["model"] = VerticalTrans * planeModel;
	wireMeshShaders["view"] = view;
	wireMeshShaders["projection"] = projMatrix;
	wireMeshShaders["lightPos"] = lightPos;

	// Tell GLUT to redraw
	glutPostRedisplay();
}






/******************************************************************************
*
*
*                               HELPER FUNCTIONS
*
*
********************************************************************************/


/**
*
* Given an integer width and height of the window this method
* will create a new OpenGL window
*
*/
void createOpenGLWindow(int width, int height)
{
	// Create a window
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 50);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("CS 5610 Project 8 - Tessellation\tTrevor Koenig");
	glEnable(GL_DEPTH_TEST);

	const char* versionGL = (const char*)glGetString(GL_VERSION);
	std::cout << "Current OpenGL version: " << versionGL << "\n";
}


/// <summary>
/// Readies the linked Vao to be rendered as a plane in the scene
/// </summary>
void createScenePlane(GLuint& planeVao, int size)
{
	GLuint planeVbo;
	GLuint planeNBuffer;
	GLuint planeEBuffer;
	GLuint planeTxc;

	// define plane positioning
	float planeVert[] = {
		1, 0, 0,
		0, 0, 1,
		0, 0, 0,
		1, 0, 0,
		1, 0, 1,
		0, 0, 1,
		2, 0, 0,
		1, 0, 1,
		1, 0, 0,
		2, 0, 0,
		2, 0, 1,
		1, 0, 1,
		1, 0, 1,
		0, 0, 2,
		0, 0, 1,
		1, 0, 1,
		1, 0, 2,
		0, 0, 2,
		2, 0, 1,
		1, 0, 2,
		1, 0, 1,
		2, 0, 1,
		2, 0, 2,
		1, 0, 2,
	};

	// define plane normals
	float planeNorms[] = {
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
	};

	// define plane faces
	int planeFaces[] = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11,
		12, 13, 14,
		15, 16, 17,
		18, 19, 20,
		21, 22, 23,
	};

	// define texture coordinates
	float planeTxcArray[] = {
		1.0, 1.0,
		0.0, 1.0,
		1.0, 0.0,
		0.0, 1.0,
		0.0, 0.0,
		1.0, 0.0
	};

	// create plane plane VAO and vbo
	glGenVertexArrays(1, &planeVao);
	glBindVertexArray(planeVao);
	glGenBuffers(1, &planeVbo);
	glBindBuffer(GL_ARRAY_BUFFER, planeVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVert), planeVert, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// create plane normal buffer
	glGenBuffers(1, &planeNBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, planeNBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeNorms), planeNorms, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);

	// create plane element buffer
	glGenBuffers(1, &planeEBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeFaces), planeFaces, GL_STATIC_DRAW);

	// create texture coordinates buffer
	glGenBuffers(1, &planeTxc);
	glBindBuffer(GL_ARRAY_BUFFER, planeTxc);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeTxcArray), planeTxcArray, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(2);
}


/// <summary>
/// 
/// </summary>
/// <param name="terrainVao">a Vertex Array Object which will be filled with the vertex info of the terrain</param>
/// <param name="mapSize">the width of the desired map to be rendered</param>
/// <returns>returns a 2d array of "Mesh" classes which represent the terrain</returns>
Mesh** createSceneTerrain(GLuint& terrainVao, int mapSize)
{
	GLuint planeVbo;
	GLuint planeNBuffer;
	GLuint planeEBuffer;
	GLuint planeTxc;

	// define texture coordinates
	float planeTxcArray[] = {
		1.0, 1.0,
		0.0, 1.0,
		1.0, 0.0,
		0.0, 1.0,
		0.0, 0.0,
		1.0, 0.0
	};

	// INJECTED CODE
	std::vector<cy::Vec3f> terrainVert = terrain.getVertices();
	std::vector<cy::Vec3f> terrainFaces = terrain.getFaces();
	std::vector<cy::Vec3f> terrainNorms = terrain.getNorms();
	float size = terrainVert.size();
	for (int i = 0; i < size; i++)
	{
		terrainNorms.emplace_back(cy::Vec3f(0.0f, 1.0f, 0.0f));
	}

	// create plane plane VAO and vbo
	glGenVertexArrays(1, &terrainVao);
	glBindVertexArray(terrainVao);
	glGenBuffers(1, &planeVbo);
	glBindBuffer(GL_ARRAY_BUFFER, planeVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * terrainVert.size(), &terrainVert[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// create plane normal buffer
	glGenBuffers(1, &planeNBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, planeNBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * terrainNorms.size(), &terrainNorms[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1);

	// create plane element buffer
	glGenBuffers(1, &planeEBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(&terrainFaces[0]) * terrainFaces.size(), &terrainFaces[0], GL_STATIC_DRAW);

	// create texture coordinates buffer
	glGenBuffers(1, &planeTxc);
	glBindBuffer(GL_ARRAY_BUFFER, planeTxc);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeTxcArray), planeTxcArray, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(2);

	return NULL;
}


/**
*
* Set the x, y, and z rotation values given based on mouse location.
*
**/
void setRotationAndDistance(float& xRot, float& yRot, float& zRot)
{
	static int prevMouseX = mouseX;
	static int prevMouseY = mouseY;

	// check to see if there has been a significant jump
	if (abs(mouseX - prevMouseX) > 30 || abs(mouseY - prevMouseY) > 30)
	{
		prevMouseX = mouseX;
		prevMouseY = mouseY;
	}

	// true means camera is right side up
	bool upDir = true;

	if (leftMouse)
	{
		// std::cout << "leftMouse drag.\nChange in Y: " << (prevMouseY - mouseY) << "\n";
		float yDelt = float(mouseY - prevMouseY) / (0.2 * windowHeight);
		float xDelt = float(mouseX - prevMouseX) / (0.2 * windowWidth);
		// dont ask me why these have to be flipped, but if you know let me know
		xRot += xDelt;
		yRot -= yDelt;
		// zRot -= float((mouseX - prevMouseX) + (mouseY - prevMouseY)) / 4.0f;
	}

	prevMouseY = mouseY;
	prevMouseX = mouseX;

	// clean up rotation values
	// std::cout << "xRot: " << RAD2DEG(xRot) << "\tyRot: " << RAD2DEG(yRot) << "\tzRot: " << RAD2DEG(zRot) << std::endl;
	xRot = fmod(xRot, DEG2RAD(360.0f));
	yRot = cy::Clamp(yRot, DEG2RAD(-89.99f), DEG2RAD(89.99f));
	zRot = fmod(zRot, DEG2RAD(360.0f));
}


/**
*
* Converts Degrees to Radians
*
**/
float DEG2RAD(float degrees)
{
	return degrees * (M_PI / 180.0f);
}


/**
*
* Converts Radians to Degrees
*
**/
float RAD2DEG(float radians)
{
	return (radians * 180.0f) / M_PI;
}

void drawPoint(float x, float y, float z)
{
	GLuint vaoID;
	GLuint vboID;
	float rainDrop[3] = { x, y, z };
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);
	glGenBuffers(1, &vboID);
	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float), rainDrop);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindVertexArray(0);
}