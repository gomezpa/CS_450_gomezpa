#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <GL/glew.h>					
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "MeshData.hpp"
#include "MeshGLData.hpp"
#include "GLSetup.hpp"
#include "Shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Utility.hpp"
using namespace std;

// Global float rotAngle
float rotAngle = 0.0f;

// Global glm::vec3 eye
glm::vec3 eye = glm::vec3(0,0,1);

// Global glm::vec3 lookAt
glm::vec3 lookAt = glm::vec3(0,0,0);

// Global glm::vec3 mousePos
glm::vec2 mousePos;

// Function for generating a transformation to rotate around an arbitrary point and axis
glm::mat4 makeLocalRotate(glm::vec3 offset, glm::vec3 axis, float angle) {
	float rad = glm::radians(angle);
	glm::mat4 negTranslation = glm::translate(glm::mat4(1.0f), -offset);
	glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), rad, axis);
	glm::mat4 offTranslation = glm::translate(glm::mat4(1.0f), offset);
	
	// Form a composite transformation
    glm::mat4 compTrans = offTranslation * rotate * negTranslation;
    return compTrans;
}

// Function for generating a transformation to rotate around the local Z axis
glm::mat4 makeRotateZ(glm::vec3 offset){
	float rad = glm::radians(rotAngle);

	glm::mat4 negTranslation = glm::translate(glm::mat4(1.0f), -offset);
	glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), rad, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 posTranslation = glm::translate(glm::mat4(1.0f), offset);
	glm::mat4 result = posTranslation * rotate * negTranslation;
	return result;
}

// Function to render a scene recursively
void renderScene(vector<MeshGL>&allMeshes, aiNode *node, glm::mat4 parentMat, GLint modelMatLoc, int level){
	aiMatrix4x4 currNode = node -> mTransformation;
	glm::mat4 nodeT;
	aiMatToGLM4(currNode, nodeT);
	glm::mat4 modelMat = parentMat * nodeT;
	glm::vec3 currPos = glm::vec3(modelMat[3]);
	glm::mat4 R = makeRotateZ(currPos);

	glm::mat4 tmpModel = R * modelMat;
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(tmpModel));

	for(unsigned int i = 0; i < node -> mNumMeshes; i++){
		int index = node->mMeshes[i];
		drawMesh(allMeshes.at(index));
	}

	for(unsigned int i = 0; i < node -> mNumChildren; i++){
		renderScene(allMeshes, node->mChildren[i], modelMat, modelMatLoc, level + 1);
	}

}

// Function to add mouse cursor movement
static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
	glm::vec2 relMouse = mousePos - glm::vec2(xpos, ypos);
	int fwidth, fheight;
	glfwGetFramebufferSize(window, &fwidth, &fheight);

	if (fwidth > 0 && fheight > 0) {
		float relMouseX = relMouse.x/(float)fwidth;
		float relMouseY = relMouse.y/(float)fheight;

		glm::vec4 lookAtV = glm::vec4(lookAt, 1.0);
		
		lookAtV = makeLocalRotate(eye, glm::vec3(0, 1, 0), 30.0f * relMouseX) * lookAtV;
		lookAtV = makeLocalRotate(eye, glm::cross(lookAt-eye, glm::vec3(0, 1, 0)), 30.0f * relMouseY) * lookAtV;

		lookAt = glm::vec3(lookAtV);
	}
	mousePos = glm::vec2(xpos, ypos);
}

// GLFW key callback function
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (action == GLFW_PRESS || action == GLFW_REPEAT){
		switch(key){
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
			case GLFW_KEY_J:
				rotAngle += 1.0;
				break;
			case GLFW_KEY_K:
				rotAngle -= 1.0;
				break;
			case GLFW_KEY_W:
				lookAt += (lookAt - eye) * glm::vec3(0.1, 0.1, 0.1);
				eye += (lookAt - eye) * glm::vec3(0.1, 0.1, 0.1);
				break; 
			case GLFW_KEY_S:
				lookAt -= (lookAt - eye) * glm::vec3(0.1, 0.1, 0.1);
				eye -= (lookAt - eye) * glm::vec3(0.1, 0.1, 0.1);
				break;
			case GLFW_KEY_D:
				lookAt += glm::cross((lookAt - eye), glm::vec3(0,1,0)) * glm::vec3(0.1, 0.1, 0.1);
				eye += glm::cross((lookAt - eye), glm::vec3(0,1,0)) * glm::vec3(0.1, 0.1, 0.1);
			case GLFW_KEY_A:
				lookAt -= glm::cross((lookAt - eye), glm::vec3(0,1,0)) * glm::vec3(0.1, 0.1, 0.1);
				eye -= glm::cross((lookAt - eye), glm::vec3(0,1,0)) * glm::vec3(0.1, 0.1, 0.1);
			default:
				break;
		}
	}
}

// Create very simple mesh: a quad (4 vertices, 6 indices, 2 triangles)
void createSimpleQuad(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create four corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;

	// Set positions of vertices
	// Note: glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	upperRight.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);

	// Add to mesh's list of vertices
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(upperRight);	
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);
	
	// Add indices for two triangles
	m.indices.push_back(0);
	m.indices.push_back(3);
	m.indices.push_back(1);

	m.indices.push_back(0);
	m.indices.push_back(2);
	m.indices.push_back(3);
}

void createSimplePentagon(Mesh &m) {
	// Clear out vertices and elements
	m.vertices.clear();
	m.indices.clear();

	// Create five corners
	Vertex upperLeft, upperRight;
	Vertex lowerLeft, lowerRight;
	Vertex centerLeft;

	// Set positions of vertices
	// Note glm::vec3(x, y, z)
	upperLeft.position = glm::vec3(-0.5, 0.5, 0.0);
	upperRight.position = glm::vec3(0.5, 0.5, 0.0);
	centerLeft.position = glm::vec3(-0.9, 0.0, 0.0);
	lowerLeft.position = glm::vec3(-0.5, -0.5, 0.0);
	lowerRight.position = glm::vec3(0.5, -0.5, 0.0);

	// Set vertex colors (red, green, blue, white)
	// Note: glm::vec4(red, green, blue, alpha)
	upperLeft.color = glm::vec4(0.0, 1.0, 0.0, 1.0);
	upperRight.color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	centerLeft.color = glm::vec4(0.2, 0.5, 0.8, 0.5);
	lowerLeft.color = glm::vec4(0.0, 0.0, 1.0, 1.0);
	lowerRight.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	
	// Add to mesh's list of vertices
	m.vertices.push_back(upperRight);
	m.vertices.push_back(upperLeft);
	m.vertices.push_back(centerLeft);
	m.vertices.push_back(lowerLeft);
	m.vertices.push_back(lowerRight);

	// Add indices for three triangles

	// Draw square shape
	m.indices.push_back(1);
	m.indices.push_back(3);
	m.indices.push_back(4);

	m.indices.push_back(1);
	m.indices.push_back(0);
	m.indices.push_back(4);

	// Center-left triangle
	m.indices.push_back(1);
	m.indices.push_back(2);
	m.indices.push_back(3);
	
}

void extractMeshData(aiMesh *mesh, Mesh &m) {
	m.vertices.clear();
	m.indices.clear();
	Vertex AIMeshVertex;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		AIMeshVertex.position.x = mesh->mVertices[i].x;
		AIMeshVertex.position.y = mesh->mVertices[i].y;
		AIMeshVertex.position.z = mesh->mVertices[i].z;

		AIMeshVertex.color = glm::vec4(0.8, 0.3, 0.2, 1.0);

		m.vertices.push_back(AIMeshVertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			m.indices.push_back(face.mIndices[j]);
		}
	}
}

// Main 
int main(int argc, char **argv) {
	
	// Are we in debugging mode?
	bool DEBUG_MODE = true;

	// GLFW setup
	// Switch to 4.1 if necessary for macOS
	GLFWwindow* window = setupGLFW(4, 3, 800, 800, DEBUG_MODE);

	// GLEW setup
	setupGLEW(window);

	//Set the key callback function
	glfwSetKeyCallback(window, keyCallback);

	// Check OpenGL version
	checkOpenGLVersion();

	// Set up debugging (if requested)
	if(DEBUG_MODE) checkAndSetupOpenGLDebugging();

	// Set the background color to a shade of blue
	glClearColor(0.5f, 0.2f, 0.3f, 1.0f); 

	// Create and load shader
	GLuint programID = 0;
	try {		
		// Load vertex shader code and fragment shader code
		string vertexCode = readFileToString("./shaders/Assign05/Basic.vs");
		string fragCode = readFileToString("./shaders/Assign05/Basic.fs");

		// Print out shader code, just to check
		if(DEBUG_MODE) printShaderCode(vertexCode, fragCode);

		// Create shader program from code
		programID = initShaderProgramFromSource(vertexCode, fragCode);
	}
	catch (exception e) {		
		// Close program
		cleanupGLFW(window);
		exit(EXIT_FAILURE);
	}

	// Create simple quad
	//Mesh m;
	//createSimplePentagon(m);

	// Create OpenGL mesh (VAO) from data
	//MeshGL mgl;
	//createMeshGL(m, mgl);
	
	Assimp::Importer importer;

	//Check for arguments
	string modelPath = "./sampleModels/sphere.obj";
	if(argc >= 2){
		modelPath = argv[1];
	}

	const aiScene* scene = importer.ReadFile(modelPath, 
											aiProcess_Triangulate | 
											aiProcess_FlipUVs | 
											aiProcess_GenNormals | 
											aiProcess_JoinIdenticalVertices);

	// Check if model was loaded correctly
	if(scene == NULL || scene -> mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene -> mRootNode == NULL){
		cout << importer.GetErrorString() << endl;
		exit(EXIT_FAILURE);
	}
	
	vector <MeshGL> myVector;
	
	for(unsigned int i = 0; i < scene -> mNumMeshes; i++){
		Mesh m;
		MeshGL mgl;
		extractMeshData(scene -> mMeshes[i], m);
		createMeshGL(m, mgl);
		myVector.push_back(mgl);
	}

	GLint modelMatLoc = glGetUniformLocation(programID, "modelMat");

	double mx, my;
	glfwGetCursorPos(window, &mx, &my);
	mousePos = glm::vec2(mx, my);
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	GLint ViewMatrixLocation = glGetUniformLocation(programID, "viewMat");
	GLint ProjectionMatrixLocation = glGetUniformLocation(programID, "projMat");

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {
		// Set viewport size
		int fwidth, fheight;
		glfwGetFramebufferSize(window, &fwidth, &fheight);
		glViewport(0, 0, fwidth, fheight);

		// Clear the framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use shader program
		glUseProgram(programID);

		glm::mat4 viewMat = glm::lookAt(eye, lookAt, glm::vec3(0, 1, 0));
		glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE,  glm::value_ptr(viewMat));

		float aspectRatio;
		if (fwidth <= 0 || fheight <= 0) {
			aspectRatio = 1.0f;
		} else {
			aspectRatio = (float) fwidth / (float) fheight;
		}

		glm::mat4 projMat = glm::perspective((float) glm::radians(90.0f), aspectRatio, 0.01f, 50.0f);
		glUniformMatrix4fv(ProjectionMatrixLocation, 1, GL_FALSE,  glm::value_ptr(projMat));

		// Draw object
		//drawMesh(mgl);
		//for(int i = 0; i < Object.size(); i++){
		//	drawMesh(Object[i]);
		//}	
		renderScene(myVector, scene->mRootNode, glm::mat4(1.0), modelMatLoc, 0);

		// Swap buffers and poll for window events		
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Sleep for 15 ms
		this_thread::sleep_for(chrono::milliseconds(15));
	}

	// Clean up mesh
	//cleanupMesh(mgl);
	for(int i = 0; i < myVector.size(); i++){
		cleanupMesh(myVector[i]);	
	}

	// Clean up shader programs
	glUseProgram(0);
	glDeleteProgram(programID);
		
	// Destroy window and stop GLFW
	cleanupGLFW(window);

	return 0;
}

