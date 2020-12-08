#include <cstdlib>
#include <ctime>
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
#include "SkyBox.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint loadCubeMap(vector<char*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
		if (data)
		{
			//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

GLuint loadTexture(const char* filename)
{
	// Step1 Create and bind textures
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	assert(textureId != 0);

	glBindTexture(GL_TEXTURE_2D, textureId);

	float borderColor[] = { 0.5f, 0.5f, 0.5f, 0.5f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

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

void vectorToString(glm::vec3 _vector) {
	float x = _vector.x;
	float y = _vector.y;
	float z = _vector.z;

	std::cout << "Vector Coordinates: " << x << " " << y << " " << z << std::endl;
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

GLuint terrainVBO = 0, terrainN_VBO = 0, terrainEBO = 0;
GLuint terrainVAO;
int terrainCount = 0;

//void genTerrain(GLuint _shaderProgram, int wRes = 128, int lRes = 128)
//{
//	const int size = wRes * lRes;
//	std::vector<glm::vec3> vertices(size), normals(size);
//	std::vector<glm::vec3> tris;
//
//	const float width = 10.0f, length = 10.0f;
//
//	int i = 0;
//	for (int z = 0; z < lRes; z++) for (int x = 0; x < wRes; x++)
//	{
//		glm::vec3 vert(x / (float)wRes, 0, z / (float)lRes);
//
//		vert.x *= width;
//		vert.z *= length;
//		vert.x -= width/2;
//		vert.z -= width/2;
//
//		vertices[i] = vert;
//		normals[i] = glm::vec3(0.0f, 1.0f, 0.0f);
//
//		if ((i + 1 % wRes) != 0 && z + 1 < lRes)
//		{
//			glm::vec3 tri(i, i + wRes, i + wRes + 1);
//			glm::vec3 tri2(i, i + wRes + 1, i + 1);
//
//			tris.push_back(tri);
//			tris.push_back(tri2);
//		}
//
//		i++;
//	}
//
//	terrainCount = tris.size();
//
//	glGenVertexArrays(1, &terrainVAO);
//	glBindVertexArray(terrainVAO);
//
//	glGenBuffers(1, &terrainVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//
//	glGenBuffers(1, &terrainN_VBO);
//	glBindBuffer(GL_ARRAY_BUFFER, terrainN_VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normals.size(), normals.data(), GL_STATIC_DRAW);
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(1);
//
//	glGenBuffers(1, &terrainEBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::vec3)*tris.size(), tris.data(), GL_STATIC_DRAW);
//	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(2);
//
//	glUseProgram(_shaderProgram);
//	glBindVertexArray(terrainVAO);
//	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
//}

glm::vec3 cameraPosition;
bool togglePlayerView = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		togglePlayerView = !togglePlayerView;
	}
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
	string joePath = "../Assets/Models/heracles.obj";
	string cylinderPath = "../Assets/Models/cylinder.obj";

	string shaderPathPrefix = "../Assets/Shaders/";

	char* brickTexture  = "../Assets/Textures/brick.jpg";
	char* cementTexture = "../Assets/Textures/cement.jpg";
	char* grassTexture = "../Assets/Textures/grass.jpg";
	char* treeBarkTexture = "../Assets/Textures/treeBark.png";
#endif

	///////////////////////////////////////////////////////////////////////
	////////
	////////				TEXTURES AND SHADER LOADING
	////////
	///////////////////////////////////////////////////////////////////////

	GLuint shaderScene       = loadSHADER(shaderPathPrefix + "sceneVertex.glsl", shaderPathPrefix + "sceneFragment.glsl");
	GLuint lightSourceScene  = loadSHADER(shaderPathPrefix + "lightObjectVertex.glsl", shaderPathPrefix + "lightObjectFragment.glsl");
	GLuint shaderShadow      = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl", shaderPathPrefix + "shadow_fragment.glsl");
	GLuint shaderSkybox      = loadSHADER(shaderPathPrefix + "skyboxVertex.glsl", shaderPathPrefix + "skyboxFragment.glsl");

	GLuint brickTextureID    = loadTexture(brickTexture);
	GLuint cementTextureID   = loadTexture(cementTexture);
	GLuint grassTextureID    = loadTexture(grassTexture);
	GLuint treeBarkTextureID = loadTexture(treeBarkTexture);
    
	const float DEPTH_MAP_TEXTURE_SIZE = 1024;

	///////////////////////////////////////////////////////////////////////
	////////
	////////				CAMERA INITIALIZATION
	////////
	///////////////////////////////////////////////////////////////////////

	cameraPosition = glm::vec3(0.0f, -2.0f, 20.0f);
	glm::vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

	glm::mat4 viewMatrix = glm::lookAt(cameraPosition,
										cameraPosition + cameraLookAt,
										cameraUp);

	glm::mat4 projectionMatrix = glm::perspective(70.0f,
		WIDTH / HEIGHT,
		0.01f, 300.0f);

	CameraControl mainCamera(shaderScene, viewMatrix, projectionMatrix);

	mainCamera.setCameraLookAt(cameraLookAt);
	mainCamera.setCameraPosition(cameraPosition);

	mainCamera.initCameraControls(window);

	///////////////////////////////////////////////////////////////////////
	////////
	////////				SHADOW AND LIGHT CONFIGURATIONS
	////////
	///////////////////////////////////////////////////////////////////////

    // Other OpenGL states to set once
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	// configure depth map FBO
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

	glm::vec3 lightColor(1.0f, 0.7f, 0.4f);

	///////////////////////////////////////////////////////////////////////
	////////
	////////			INITIAL WORLD POSITIONS GENERATION
	////////
	///////////////////////////////////////////////////////////////////////

	// Closest tree
	glm::vec3 closest_object(0.0f);
	glm::vec3 closest_object_scale(0.0f);

	std::vector<glm::vec3> objectPosVector;
	std::vector<glm::vec3> objectScaVector;

	std::vector<glm::vec3> floorPosVector;

	glm::mat4 playerPosition = glm::translate(glm::mat4(1.0f), mainCamera.getCameraPosition());
	glm::vec3 lastPos;
	glm::vec3 lastSca;

	// Skybox init
	SkyBox skyBox(0);

	srand((unsigned)time(0));

	for (unsigned int i = 0; i < 200; i++) {

		lastPos = glm::vec3(rand() % 200 + (-100), -5.0f, rand() % 100 + (-50));
		objectPosVector.push_back(lastPos);

		float scaleX = (rand() % 3 + 1) * 0.35f;
		float scaleY = rand() % 6 + 3;

		lastSca = glm::vec3(scaleX, scaleY, scaleX);

		objectScaVector.push_back(lastSca);

		if (glm::distance(cameraPosition, lastPos) < glm::distance(cameraPosition, closest_object))
			closest_object = lastPos;
	}

	for (int i = -2; i < 3; i++) {
		lastPos = glm::vec3(mainCamera.getCameraPosition().x, -5.0f, i * 50);
		floorPosVector.push_back(lastPos);
		std::cout << lastPos.y << std::endl;
	}

	///////////////////////////////////////////////////////////////////////
	////////
	////////				SET OF RENDEREABLE OBJECTS
	////////
	///////////////////////////////////////////////////////////////////////

	SceneObject treeTrunk_cube("CUBE", glm::vec3(0.0f), shaderScene, treeBarkTextureID, cubePath, 0);
	SceneObject cube("CUBE", glm::vec3(0.0f), shaderScene, grassTextureID, cubePath, 0);
	SceneObject sphere("SPHERE", glm::vec3(0.0f, -3.0f, -5.0f), shaderScene, brickTextureID, spherePath, 0);
	SceneObject light_sphere("LIGHT", glm::vec3(0.0f), lightSourceScene, 0, spherePath, 0);
	SceneObject floor_plane("FLOOR", glm::vec3(0.0f, -5.0f, 0.0f), shaderScene, grassTextureID, planePath, 0);
	SceneObject treeTrunk_cylinder("CYLINDER", glm::vec3(0.0f), shaderScene, treeBarkTextureID, cylinderPath, 0);
	//SceneObject player_cube("CUBE", mainCamera.getCameraPosition(), shaderScene, 0, cubePath, 0);

	///////////////////////////////////////////////////////////////////////
	////////
	////////				USED TO RECALCULATE FLOOR POSITIONS
	////////
	///////////////////////////////////////////////////////////////////////

	glm::vec3 furthestF = floorPosVector.at(0);
	float d_front = 150;
	int f_pos = 0;

	glm::vec3 furthestB = floorPosVector.at(4);
	float d_back = 150;
	int b_pos = 4;

	///////////////////////////////////////////////////////////////////////
	////////
	////////					VIEW TYPE TOGGLE
	////////
	///////////////////////////////////////////////////////////////////////

	bool viewType = false;

	skyBox.texture = loadCubeMap(skyBox.createModels());
	skyBox.init();
	SetUniform1Value(shaderSkybox, "skybox", 0);

    // Entering Main Loop
    while(!glfwWindowShouldClose(window))
    {
        // Each frame, reset color of each pixel to glClearColor
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Light Position stays above the player by 10 units as they move
		//glm::vec3 lightPos(mainCamera.getCameraPosition() + glm::vec3(0.0f, 10.0f, 0.0f));
		glm::vec3 lightPos(glm::vec3(mainCamera.getCameraPosition().x - 100.0f, 90.0f, mainCamera.getCameraPosition().z - 80));

		///////////////////////////////////////////////////////////////////////
		////////
		////////						LIGHTS AND SHADOW SETUP
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
			SetUniformMat4(shaderShadow, "model", glm::translate(glm::mat4(1.0f), floor_plane.getPosition()) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)));
			floor_plane.Draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderScene);

		SetUniformMat4(shaderScene, "lightSpaceMatrix", lightSpaceMatrix);
		SetUniformVec3(shaderScene, "lightPos", lightPos);
		SetUniformVec3(shaderScene, "viewPos", cameraPosition);
		SetUniformVec3(shaderScene, "lightColor", lightColor);
		
		///////////////////////////////////////////////////////////////////////
		////////
		////////				TREE GENERATION
		////////
		///////////////////////////////////////////////////////////////////////

		treeTrunk_cube.setShader(shaderScene);
		for (unsigned int i = 0; i < objectPosVector.size(); i++)
		{
			float x = rand() % 200 + (-100);
			float z = (rand() % 150) + 50;

			glm::vec3* currentObjPos = &objectPosVector.at(i);
			glm::vec3* currentObjSca = &objectScaVector.at(i);
			float currentRadius = currentObjSca->x * 4.0;

			if (glm::distance(*currentObjPos, mainCamera.getCameraPosition()) > 100)
			{
				*currentObjPos = glm::vec3(mainCamera.getCameraPosition().x + x, -3.0f, mainCamera.getCameraPosition().z - z);
			}
			else 
			{
				// Tree Trunk
				glBindVertexArray(treeTrunk_cylinder.getVAO());
				treeTrunk_cube.setTexture(treeBarkTextureID);
				SetUniformMat4(shaderScene, "model", glm::translate(glm::mat4(1.0f), *currentObjPos)
					* glm::scale(glm::mat4(1.0f), glm::vec3((*currentObjSca).x, (*currentObjSca).y, (*currentObjSca).z)));
				treeTrunk_cylinder.Draw();

				// Leaves
				glBindVertexArray(sphere.getVAO());
				sphere.setTexture(grassTextureID);
				SetUniformMat4(shaderScene, "model", glm::translate(glm::mat4(1.0f), *currentObjPos)
					* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, (*currentObjSca).y * 4.0f, 0.0f))
					* glm::scale(glm::mat4(1.0f), glm::vec3((*currentObjSca).y * 1.2f)));
				sphere.Draw();
			}

			if ((glm::distance(*currentObjPos, mainCamera.getPlayerBodyPosition()) < glm::distance(closest_object, mainCamera.getPlayerBodyPosition())))
			{
				closest_object = *currentObjPos;
				closest_object_scale = *currentObjSca;
			}
		}

		///////////////////////////////////////////////////////////////////////
		////////
		////////				PLAYER CONTROLLS / CAMERA
		////////
		///////////////////////////////////////////////////////////////////////

		mainCamera.setShaderProjection(shaderScene, projectionMatrix);

		glm::mat4 playerTransform = glm::mat4(1.0f);
		glm::mat4 playerFront = glm::mat4(1.0f);
		glm::vec3 _currentPlayerPos = sphere.getPosition();

		// True for first person - False for third person
		if (togglePlayerView)
		{
			mainCamera.toggleView = togglePlayerView;
			// Debugging
			//vectorToString(closest_object);
			//vectorToString(mainCamera.getPlayerBodyPosition());
		}
		else
		{
			mainCamera.toggleView = togglePlayerView;
		}

		mainCamera.playerController(window, shaderScene, _currentPlayerPos, playerTransform, closest_object, closest_object_scale);

		if (!togglePlayerView)
		{
			glBindVertexArray(sphere.getVAO());
			sphere.setTexture(brickTextureID);
			SetUniformMat4(shaderScene, "model", playerTransform);
			sphere.Draw();

			glBindVertexArray(sphere.getVAO());
			sphere.setTexture(cementTextureID);
			playerFront = playerTransform
				* glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.5f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
			SetUniformMat4(shaderScene, "model", playerFront);
			sphere.Draw();
		}
		
		///////////////////////////////////////////////////////////////////////
		////////
		////////						FLOOR GENERATION
		////////
		///////////////////////////////////////////////////////////////////////

		for (int i = 0; i < floorPosVector.size(); i++)
		{
			floor_plane.setShader(shaderScene);

			glBindVertexArray(floor_plane.getVAO());
			SetUniformMat4(shaderScene, "model", glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), floorPosVector.at(i)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)));
			floor_plane.Draw();

			glBindVertexArray(floor_plane.getVAO());
			SetUniformMat4(shaderScene, "model", glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), floorPosVector.at(i) + glm::vec3(-50.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)));
			floor_plane.Draw();

			glBindVertexArray(floor_plane.getVAO());
			SetUniformMat4(shaderScene, "model", glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), floorPosVector.at(i) + glm::vec3(-100.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)));
			floor_plane.Draw();

			glBindVertexArray(floor_plane.getVAO());
			SetUniformMat4(shaderScene, "model", glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), floorPosVector.at(i) + glm::vec3(50.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)));
			floor_plane.Draw();

			glBindVertexArray(floor_plane.getVAO());
			SetUniformMat4(shaderScene, "model", glm::mat4(1.0f) * glm::translate(glm::mat4(1.0f), floorPosVector.at(i) + glm::vec3(100.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)));
			floor_plane.Draw();
		}

		if (glm::distance(mainCamera.getCameraPosition(), furthestB) > d_back)
		{
			floorPosVector.at(b_pos).z = floorPosVector.at(f_pos).z - 50;
			f_pos = b_pos;
			b_pos--;
			if (b_pos < 0)
				b_pos = 4;
			furthestB = floorPosVector.at(b_pos);
		}

		///////////////////////////////////////////////////////////////////////
		////////
		////////				LIGHT SOURCE SHADER / SCENE
		////////
		///////////////////////////////////////////////////////////////////////
		
		mainCamera.setShaderView(lightSourceScene, mainCamera.getViewMatrix());
		mainCamera.setShaderProjection(lightSourceScene, projectionMatrix);

		glUseProgram(lightSourceScene);

		glBindVertexArray(light_sphere.getVAO());
		SetUniformMat4(lightSourceScene, "model", glm::translate(glm::mat4(1.0f), lightPos) 
			* glm::scale(glm::mat4(1.0f), glm::vec3(3.25f)));
		SetUniformVec3(lightSourceScene, "lightColor", lightColor);
		light_sphere.Draw();

		///////////////////////////////////////////////////////////////////////
		////////
		////////						SKYBOX
		////////
		///////////////////////////////////////////////////////////////////////

		glm::mat4 skyView = glm::mat4(glm::mat3(mainCamera.getViewMatrix()));
		mainCamera.setShaderView(shaderSkybox, skyView);
		mainCamera.setShaderProjection(shaderSkybox, projectionMatrix);
		skyBox.draw(shaderSkybox);

        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();

		// Initial keyCallback setup
		glfwSetKeyCallback(window, key_callback);
        
        // Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			glfwSetKeyCallback(window, key_callback);
	}

    glfwTerminate();
    
	return 0;
}

