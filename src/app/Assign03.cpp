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
using namespace std;
// Trivial changes

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

		AIMeshVertex.color = glm::vec4(0.5, 0.8, 0.2, 1.0);

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

	// Check OpenGL version
	checkOpenGLVersion();

	// Set up debugging (if requested)
	if(DEBUG_MODE) checkAndSetupOpenGLDebugging(); 

	// Set the background color to a shade of blue
	glClearColor(0.5f, 0.2f, 0.3f, 1.0f);	

	// Check for sufficient arguments
	if (argc <= 1) {
		exit(EXIT_FAILURE);
	}

	// Create and load shader
	GLuint programID = 0;
	try {		
		// Load vertex shader code and fragment shader code
		string vertexCode = readFileToString("./shaders/Assign03/Basic.vs");
		string fragCode = readFileToString("./shaders/Assign03/Basic.fs");

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
	// createSimpleQuad(m);
	//createSimplePentagon(m);

	// Create OpenGL mesh (VAO) from data
	//MeshGL mgl;
	//createMeshGL(m, mgl);

	Assimp::Importer importer;
	
	const aiScene *scene = importer.ReadFile(argv[1],
											 aiProcess_Triangulate |
											 aiProcess_FlipUVs |
											 aiProcess_GenNormals |
											 aiProcess_JoinIdenticalVertices);

	// Check if model was loaded correctly
	if (scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == NULL) {
		cout << importer.GetErrorString() << endl;
		exit(EXIT_FAILURE);
	}

	vector <MeshGL> myVector;

	for (int i = 0; i < scene->mNumMeshes; i++) {
		Mesh m;
		MeshGL mgl;
		extractMeshData(scene->mMeshes[i], m);
		createMeshGL(m, mgl);
		myVector.push_back(mgl);
	}
	
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

		// Draw object
		//drawMesh(mgl);
		for (int i = 0; i < myVector.size(); i++) {
			drawMesh(myVector[i]);
		}	

		// Swap buffers and poll for window events		
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Sleep for 15 ms
		this_thread::sleep_for(chrono::milliseconds(15));
	}

	// Clean up mesh
	//cleanupMesh(mgl);
	for (int i = 0; i < myVector.size(); i++) {
		cleanupMesh(myVector[i]);
	}

	// Clean up shader programs
	glUseProgram(0);
	glDeleteProgram(programID);
		
	// Destroy window and stop GLFW
	cleanupGLFW(window);

	return 0;
}

