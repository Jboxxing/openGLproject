//
// COMP 371 Lab 6 - Models and EBOs
//
// Created by Zachary Lapointe on 17/07/2019.
//

#include <iostream>
#include <algorithm>
#include <vector>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp> // value_ptr

#include "OBJloader.h"  //For loading .obj files
#include "OBJloaderV2.h"  //For loading .obj files using a polygon list format

#include "shaderloader.h" // Load shaders

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;
using namespace std;

GLuint loadTexture(const char* filename)
{
	// Step1 Create and bind textures
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	assert(textureId != 0);

	glBindTexture(GL_TEXTURE_2D, textureId);

	// Step2 Set filter parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Step3 Load Textures with dimension data
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
		return 0;
	}

	// Step4 Upload the texture to the PU
	GLenum format = 0;
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if (nrChannels == 4)
		format = GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
		0, format, GL_UNSIGNED_BYTE, data);

	// Step5 Free resources
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureId;
}

void setProjectionMatrix(int shaderProgram, mat4 projectionMatrix)
{
    glUseProgram(shaderProgram);
    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void setViewMatrix(int shaderProgram, mat4 viewMatrix)
{
    glUseProgram(shaderProgram);
    GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}

void setWorldMatrix(int shaderProgram, mat4 worldMatrix)
{
	glUseProgram(shaderProgram);
	GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
}

GLuint setupModelVBO(string path, int& vertexCount) {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> UVs;
	
	//read the vertex data from the model's OBJ file
	loadOBJ(path.c_str(), vertices, normals, UVs);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); //Becomes active VAO
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).

	//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	//UVs VBO setup
	GLuint uvs_VBO;
	glGenBuffers(1, &uvs_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs, as we are using multiple VAOs)
	vertexCount = vertices.size();
	return VAO;
}

GLuint setUpModelInstanceEBO(string path, int& vertexCount) {
	vector<int> vertexIndices; //The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> UVs;

	//read the vertices from the cube.obj file
	//We won't be needing the normals or UVs for this program
	//loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);
	loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);
	
	// generate a list of 100 quad locations/translation-vectors
	// ---------------------------------------------------------
	glm::vec2 translations[100];
	int index = 0;
	float offset = 0.1f;
	for (int y = -10; y < 10; y += 2)
	{
		for (int x = -10; x < 10; x += 2)
		{
			glm::vec2 translation;
			translation.x = (float)x / 10.0f + offset;
			translation.y = (float)y / 10.0f + offset;
			translations[index++] = translation;
		}
	}

	// store instance data in an array buffer
	// --------------------------------------
	unsigned int instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	GLuint instances_VBO;
	glGenBuffers(1, &instances_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, instances_VBO);
	// UVs for now
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
	glEnableVertexAttribArray(2);

	//EBO setup
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(int), &vertexIndices.front(), GL_STATIC_DRAW);

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	vertexCount = vertexIndices.size();
	return VAO;
}

//Sets up a model using an Element Buffer Object to refer to vertex data
GLuint setupModelEBO(string path, int& vertexCount)
{
	vector<int> vertexIndices; //The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> UVs;

	//read the vertices from the cube.obj file
	//We won't be needing the normals or UVs for this program
	//loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);
	loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); //Becomes active VAO
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).

	//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	//UVs VBO setup
	GLuint uvs_VBO;
	glGenBuffers(1, &uvs_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(2);

	//EBO setup
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(int), &vertexIndices.front(), GL_STATIC_DRAW);

	glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	vertexCount = vertexIndices.size();
	return VAO;
}

// shader variable setters
void SetUniformMat4(GLuint shader_id, const char* uniform_name, mat4 uniform_value)
{
	glUseProgram(shader_id);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void SetUniformVec3(GLuint shader_id, const char* uniform_name, vec3 uniform_value)
{
	glUseProgram(shader_id);
	glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1, value_ptr(uniform_value));
}

template <class T>
void SetUniform1Value(GLuint shader_id, const char* uniform_name, T uniform_value)
{
	glUseProgram(shader_id);
	glUniform1i(glGetUniformLocation(shader_id, uniform_name), uniform_value);
	glUseProgram(0);
}

int main(int argc, char*argv[])
{
    // Initialize GLFW and OpenGL version
    glfwInit();
    
#if defined(PLATFORM_OSX)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // On windows, we set OpenGL version to 2.1, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Comp371 - A3", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);

    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
  
    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
	//Setup models
#if defined(PLATFORM_OSX)
	string cubePath = "Models/cube.obj";
	string heraclesPath = "Models/heracles.obj";

	string shaderPathPrefix = "Assets/Shaders/";

	GLuint brickTextureID = loadTexture("Assets/Textures/brick.jpg");
	GLuint cementTextureID = loadTexture("Assets/Textures/cement.jpg");
#else
	string cubePath = "../Assets/Models/cube.obj";
	string spherePath = "../Assets/Models/uvSphere.obj";

	string shaderPathPrefix = "../Assets/Shaders/";

	char* brickTexture  = "../Assets/Textures/brick.jpg";
	char* cementTexture = "../Assets/Textures/cement.jpg";
#endif

	GLuint shaderScene = loadSHADER(shaderPathPrefix + "sceneVertex.glsl", shaderPathPrefix + "sceneFragment.glsl");
	GLuint lightSourceScene = loadSHADER(shaderPathPrefix + "lightObjectVertex.glsl", shaderPathPrefix + "lightObjectFragment.glsl");

	GLuint shaderShadow = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl", shaderPathPrefix + "shadow_fragment.glsl");

	GLuint brickTextureID = loadTexture(brickTexture);
	GLuint cementTextureID = loadTexture(cementTexture);
	
	int cubeVertices;
	GLuint cubeVAO = setupModelVBO(cubePath, cubeVertices);	
	
	int sphereVertices;
	GLuint sphereVAO = setupModelEBO(spherePath, sphereVertices);

	//int cubeVertices;
	//GLuint cubeInstanceVAO = setUpModelInstanceEBO(cubePath, cubeVertices);

	// Draw colored geometry
	glUseProgram(shaderShadow);
	glUseProgram(shaderScene);
	glUseProgram(lightSourceScene);
	
	//int herVertices;
	//GLuint herVAO = setupModelEBO(heraclesPath, herVertices);

	//int activeVAOVertices = sphereVertices;
	//GLuint activeVAO = sphereVAO;
	int activeVAOVertices = cubeVertices;
	GLuint activeVAO = cubeVAO;
	//int activeVAOVertices = cubeVertices;
	//GLuint activeVAO = cubeInstanceVAO;

    // Camera parameters for view transform
    vec3 cameraPosition(0.0f, 1.0f, 10.0f);
    vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);
    
    // Other camera parameters
    float cameraSpeed = 3.0f;
    float cameraFastSpeed = 2 * cameraSpeed;
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;

	const float DEPTH_MAP_TEXTURE_SIZE = 1024;

    // Spinning cube at camera position
    float spinningAngle = 0.0f;
    
    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix = glm::perspective(70.0f,            // field of view in degrees
                                             1024.0f / 768.0f,  // aspect ratio
                                             0.01f, 100.0f);   // near and far (near > 0)
    
    // Set initial view matrix
    mat4 viewMatrix = lookAt(cameraPosition,  // eye
                             cameraPosition + cameraLookAt,  // center
                             cameraUp ); // up
    
    // Set View and Projection matrices on both shaders
    setViewMatrix(shaderScene, viewMatrix);

    setProjectionMatrix(shaderScene, projectionMatrix);

    // For frame time
    float lastFrameTime = glfwGetTime();
    int lastMouseLeftState = GLFW_RELEASE;
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

    // Other OpenGL states to set once
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	std::vector<glm::mat4> posVector;

	for (unsigned int i = 0; i < 1000; i++) {
		posVector.push_back(glm::mat4(1.0f) * glm::translate(mat4(1.0f), vec3(rand() % 100, -7.0f, -rand() % 10)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f)));
	};

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
	// --------------------
	glUseProgram(shaderScene);
	SetUniform1Value(shaderScene, "diffuse_texture", 0);
	SetUniform1Value(shaderScene, "shadow_map", 1);

	// lighting
	glm::vec3 lightPos(30.0f, 10.0f, -5.0f);

    // Entering Main Loop
    while(!glfwWindowShouldClose(window))
    {
        // Frame time calculation
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;

        // Each frame, reset color of each pixel to glClearColor
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 modelWorldMatrix(1.0f);
		mat4 viewMatrix(1.0f);

		viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);

		setViewMatrix(shaderScene, viewMatrix);
		setProjectionMatrix(shaderScene, projectionMatrix);

		///////////////////////////////////////////////////////////////////////
		////////
		////////						MAIN WORLD SHADER
		////////
		///////////////////////////////////////////////////////////////////////

		//SetUniformMat4(shaderScene, "model", modelWorldMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -9.0f, 0.0f)));
		//glBindVertexArray(activeVAO);
		//glDrawElements(GL_TRIANGLES, activeVAOVertices, GL_UNSIGNED_INT, 0);

		float lightNearPlane = 1.0f;
		float lightFarPlane = 8.0f;

		glm::vec3 lightFocus(0.2f);

		mat4 lightProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, lightNearPlane, lightFarPlane);
			// glm::perspective(20.0f, (float)DEPTH_MAP_TEXTURE_SIZE / (float)DEPTH_MAP_TEXTURE_SIZE, lightNearPlane, lightFarPlane);
		mat4 lightViewMatrix = glm::lookAt(lightPos, lightFocus, glm::vec3(0.0f, 1.0f, 0.0f));
		mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

		glUseProgram(shaderShadow);

		SetUniformMat4(shaderShadow, "lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			//glActiveTexture(GL_TEXTURE0);
			
			for (unsigned int i = 0; i < posVector.size(); i++) {
				glBindVertexArray(activeVAO);
				glBindTexture(GL_TEXTURE_2D, brickTextureID);
				SetUniformMat4(shaderShadow, "model", posVector.at(i));
				//glDrawElements(GL_TRIANGLES, activeVAOVertices, GL_UNSIGNED_INT, 0);
				glDrawArrays(GL_TRIANGLES, 0, activeVAOVertices);
			}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, 1024, 768);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderScene);

		SetUniformMat4(shaderScene, "projection", projectionMatrix);
		SetUniformMat4(shaderScene, "view", viewMatrix);

		SetUniformMat4(shaderScene, "lightSpaceMatrix", lightSpaceMatrix);
		SetUniformVec3(shaderScene, "lightPos", lightPos);
		SetUniformVec3(shaderScene, "viewPos", cameraPosition);

		//SetUniformVec3(shaderScene, "objectColor", glm::vec3(0.1f, 0.7f, 0.1f));
		
		for (unsigned int i = 0; i < posVector.size(); i++) {
			glBindVertexArray(activeVAO);
			glBindTexture(GL_TEXTURE_2D, brickTextureID);
			SetUniformMat4(shaderScene, "model", posVector.at(i) * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f)));
			//glDrawElements(GL_TRIANGLES, activeVAOVertices, GL_UNSIGNED_INT, 0);
			glDrawArrays(GL_TRIANGLES, 0, activeVAOVertices);
		}

		glBindVertexArray(0);

		///////////////////////////////////////////////////////////////////////
		////////
		////////						LIGHT SOURCE SHADER
		////////
		///////////////////////////////////////////////////////////////////////

		glUseProgram(lightSourceScene);

		SetUniformMat4(lightSourceScene, "model", glm::translate(glm::mat4(1.0f), lightPos) * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f)));
		SetUniformMat4(lightSourceScene, "projection", projectionMatrix);
		SetUniformMat4(lightSourceScene, "view", viewMatrix);

		glBindVertexArray(activeVAO);
		//glDrawElements(GL_TRIANGLES, activeVAOVertices, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, activeVAOVertices);

		glBindVertexArray(0);

        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
        
        // This was solution for Lab02 - Moving camera exercise
        // We'll change this to be a first or third person camera
        bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;
        
        
        // - Calculate mouse motion dx and dy
        // - Update camera horizontal and vertical angle
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);
        
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;
        
        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;

        // Convert to spherical coordinates
        const float cameraAngularSpeed = 60.0f;
        cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
        cameraVerticalAngle   -= dy * cameraAngularSpeed * dt;
        
        // Clamp vertical angle to [-85, 85] degrees
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));
        
        float theta = radians(cameraHorizontalAngle);
        float phi = radians(cameraVerticalAngle);
        
        cameraLookAt = vec3(cosf(phi)*cosf(theta), sinf(phi), -cosf(phi)*sinf(theta));
        vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
        
		glm::normalize(cameraSideVector);
        
        // Use camera lookat and side vectors to update positions with ASDW
        if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS)
        {
            cameraPosition += cameraLookAt * dt * currentCameraSpeed;
        }
        
        if (glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS)
        {
            cameraPosition -= cameraLookAt * dt * currentCameraSpeed;
        }
        
        if (glfwGetKey(window, GLFW_KEY_D ) == GLFW_PRESS)
        {
            cameraPosition += cameraSideVector * dt * currentCameraSpeed;
        }
        
        if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS)
        {
            cameraPosition -= cameraSideVector * dt * currentCameraSpeed;
        }  
    }

    glfwTerminate();
    
	return 0;
}

