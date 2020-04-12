
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

#include "shaderloader.h" // Load shaders

#include "SceneObject.h"
#include "CameraControl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

//void setProjectionMatrix(int shaderProgram, glm::mat4 projectionMatrix)
//{
//    glUseProgram(shaderProgram);
//    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection");
//    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
//}
//
//void setViewMatrix(int shaderProgram, glm::mat4 viewMatrix)
//{
//    glUseProgram(shaderProgram);
//    GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "view");
//    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
//}

void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix)
{
	glUseProgram(shaderProgram);
	GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
}

GLuint setUpModelInstanceEBO(string path, int& vertexCount) {
	vector<int> vertexIndices; //The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> UVs;

	//read the vertices from the cube.obj file
	//We won't be needing the normals or UVs for this program
	//loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);
	//loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);
	
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

void SetUniformMat4(GLuint shader_id, const char* uniform_name, glm::mat4 uniform_value)
{
	glUseProgram(shader_id);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void SetUniformVec3(GLuint shader_id, const char* uniform_name, glm::vec3 uniform_value)
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

	const float WIDTH = 1024.0f, HEIGHT = 768.0f;

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Comp371 - A3", NULL, NULL);
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
	string planePath = "../Assets/Models/plane.obj";

	string shaderPathPrefix = "../Assets/Shaders/";

	char* brickTexture  = "../Assets/Textures/brick.jpg";
	char* cementTexture = "../Assets/Textures/cement.jpg";
	char* grassTexture = "../Assets/Textures/grass.jpg";
#endif

	GLuint shaderScene      = loadSHADER(shaderPathPrefix + "sceneVertex.glsl", shaderPathPrefix + "sceneFragment.glsl");
	GLuint lightSourceScene = loadSHADER(shaderPathPrefix + "lightObjectVertex.glsl", shaderPathPrefix + "lightObjectFragment.glsl");
	GLuint shaderShadow     = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl", shaderPathPrefix + "shadow_fragment.glsl");

	GLuint brickTextureID  = loadTexture(brickTexture);
	GLuint cementTextureID = loadTexture(cementTexture);
	GLuint grassTextureID  = loadTexture(grassTexture);

    // Camera parameters for view transform
	glm::vec3 cameraPosition(0.0f, 1.0f, 20.0f);
	glm::vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    
    // Other camera parameters
    float cameraSpeed = 5.0f;
    float cameraFastSpeed = 2 * cameraSpeed;
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;

	const float DEPTH_MAP_TEXTURE_SIZE = 1024;
    
	glm::mat4 projectionMatrix = glm::perspective(70.0f,            // field of view in degrees
                                             WIDTH / HEIGHT,  // aspect ratio
                                             0.01f, 300.0f);   // near and far (near > 0)
    
	glm::mat4 viewMatrix = glm::lookAt(cameraPosition,           // eye
                             cameraPosition + cameraLookAt, // center
                             cameraUp);                     // up

	CameraControl mainCamera(shaderScene, viewMatrix, projectionMatrix);
	mainCamera.setCameraLookAt(cameraLookAt);
	mainCamera.setCameraPosition(cameraPosition);

    // Other OpenGL states to set once
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

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
	glm::vec3 lightColor(0.2f, 0.6f, 0.2f);

	///////////////////////////////////////////////////////////////////////
	////////
	////////					 C O N T R O L S
	////////
	///////////////////////////////////////////////////////////////////////

	glm::mat4 modelWorldMatrix(1.0f);

	mainCamera.initCameraControls(window);

	///////////////////////////////////////////////////////////////////////
	////////
	////////						W O R L D
	////////
	///////////////////////////////////////////////////////////////////////

	std::vector<glm::mat4> posVector;

	for (unsigned int i = 0; i < 1000; i++) {
		posVector.push_back(glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), glm::vec3(rand() % 100 + (-50), -7.0f, -rand() % 10 + (-5))) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
	};

	SceneObject cube("CUBE", glm::vec3(0.0f), shaderScene, grassTextureID, cubePath, 0);
	SceneObject sphere("SPHERE", glm::vec3(0.0f), shaderScene, brickTextureID, spherePath, 1);
	SceneObject light_cube("LIGHT", glm::vec3(0.0f), lightSourceScene, cementTextureID, cubePath, 0);
	SceneObject floor_plane("FLOOR", glm::vec3(0.0f), shaderScene, grassTextureID, planePath, 0);

    // Entering Main Loop
    while(!glfwWindowShouldClose(window))
    {
        // Each frame, reset color of each pixel to glClearColor
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mainCamera.playerController(window, shaderScene, viewMatrix, cameraPosition, cameraLookAt);
		mainCamera.setShaderProjection(shaderScene, projectionMatrix);
		
		///////////////////////////////////////////////////////////////////////
		////////
		////////						MAIN WORLD SHADER
		////////
		///////////////////////////////////////////////////////////////////////

		float lightNearPlane = 1.0f;
		float lightFarPlane = 7.5f;

		glm::vec3 lightFocus(0.0f);

		glm::mat4 lightProjectionMatrix = // glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, lightNearPlane, lightFarPlane);
			glm::perspective(45.0f, (float)DEPTH_MAP_TEXTURE_SIZE / (float)DEPTH_MAP_TEXTURE_SIZE, lightNearPlane, lightFarPlane);
		glm::mat4 lightViewMatrix = glm::lookAt(lightPos, lightFocus, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

		glUseProgram(shaderShadow);

		SetUniformMat4(shaderShadow, "lightSpaceMatrix", lightSpaceMatrix);
		//cube.setShader(shaderShadow);
		floor_plane.setShader(shaderShadow);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(floor_plane.getVAO());
			SetUniformMat4(shaderShadow, "model", glm::mat4(1.0f) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)));
			floor_plane.Draw();

			//for (unsigned int i = 0; i < posVector.size(); i++) {
			//	SetUniformMat4(shaderShadow, "model", posVector.at(i) * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f)));
			//	cube.Draw();
			//}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderScene);

		SetUniformMat4(shaderScene, "lightSpaceMatrix", lightSpaceMatrix);
		SetUniformVec3(shaderScene, "lightPos", lightPos);
		SetUniformVec3(shaderScene, "viewPos", cameraPosition);
		SetUniformVec3(shaderScene, "lightColor", lightColor);

		cube.setShader(shaderScene);
		for (unsigned int i = 0; i < posVector.size(); i++) 
		{
			if (i < posVector.size() / 3)
			{
				glBindVertexArray(cube.getVAO());
				cube.setTexture(brickTextureID);
				SetUniformMat4(shaderScene, "model", posVector.at(i) * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f)));
				cube.Draw();
			}
			else if (i > posVector.size() / 3 && i < (2 * posVector.size()) / 3)
			{
				glBindVertexArray(cube.getVAO());
				cube.setTexture(cementTextureID);
				SetUniformMat4(shaderScene, "model", posVector.at(i) * glm::scale(glm::mat4(1.0f), glm::vec3(0.23f)));
				cube.Draw();
			}
			else
			{
				glBindVertexArray(cube.getVAO());
				cube.setTexture(grassTextureID);
				SetUniformMat4(shaderScene, "model", posVector.at(i) * glm::scale(glm::mat4(1.0f), glm::vec3(0.28f)));
				cube.Draw();
			}
		}

		glBindVertexArray(sphere.getVAO());
		SetUniformMat4(shaderScene, "model", glm::mat4(1.0f) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
		sphere.Draw();

		glBindVertexArray(light_cube.getVAO());
		SetUniformMat4(shaderScene, "model", glm::mat4(1.0f) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
		light_cube.Draw();

		floor_plane.setShader(shaderScene);
		glBindVertexArray(floor_plane.getVAO());
		SetUniformMat4(shaderScene, "model", glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(100.0f)));
		floor_plane.Draw();

		///////////////////////////////////////////////////////////////////////
		////////
		////////						LIGHT SOURCE SHADER
		////////
		///////////////////////////////////////////////////////////////////////

		glUseProgram(lightSourceScene);

		SetUniformMat4(lightSourceScene, "model", glm::translate(glm::mat4(1.0f), lightPos) * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f)));
		
		mainCamera.setShaderView(lightSourceScene, mainCamera.getViewMatrix());
		mainCamera.setShaderProjection(lightSourceScene, projectionMatrix);

        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		{
			lightPos   += glm::vec3(1.0f, 0.0f, 0.0f);
			//lightFocus += glm::vec3(1.0f, 0.0f, 0.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		{
			lightPos   += glm::vec3(-1.0f, 0.0f, 0.0f);
			//lightFocus += glm::vec3(-1.0f, 0.0f, 0.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		{
			lightPos   += glm::vec3(0.0f, 1.0f, 0.0f);
			//lightFocus += glm::vec3(1.0f, 0.0f, 0.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		{
			lightPos   += glm::vec3(0.0f, -1.0f, 0.0f);
			//lightFocus += glm::vec3(-1.0f, 0.0f, 0.0f);
		}
    }

    glfwTerminate();
    
	return 0;
}

