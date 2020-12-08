#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>

#include <iostream>
#include <vector>

using namespace std;
using namespace glm;

struct Vertex
{
	vec3 position;
};

class SkyBox {
private:
	unsigned int mVAO;
	unsigned int mVBO;
	unsigned int numOfVertices;

	//unsigned int loadCubemap(std::vector<std::string> faces);

public:
	vec3 position = vec3(0.0f);
	vec3 rotation = vec3(0.0f);
	vec3 scaling = vec3(1.0f);
	vec3 color = vec3(0.0f);
	vec3 hitbox = vec3(0.0f);
	GLuint texture;
	SkyBox(GLuint texture);
	~SkyBox();
	void init();
	void draw(GLuint _shader);
	std::vector<char*> createModels();
};