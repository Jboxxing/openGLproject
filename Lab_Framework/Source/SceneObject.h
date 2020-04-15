#pragma once

#include <iostream>
#include <algorithm>
#include <vector>

#define GLEW_STATIC 1
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

class SceneObject
{
private:
	std::string objectName;

	// Modifiable
	glm::vec3 position;
	GLuint shader;
	GLuint texture;

	// Remains constant
	GLuint VAO;
	int vertexCount;
	unsigned int bufferType;

	void initVBO(std::string path);
	void initEBO(std::string path);

public:
	SceneObject();
	SceneObject(std::string _name, glm::vec3 _position, GLuint _shader, GLuint _texture, std::string _objectPath, unsigned int _bufferType);
	~SceneObject();

	void Draw();

	glm::vec3 getPosition() const;
	void setPosition(glm::vec3 _position);

	GLuint getVAO() const;

	GLuint getShader() const;
	void setShader(GLuint _shader);

	GLuint getTexture() const;
	void setTexture(GLuint _texture);

	int getVertices() const;
};

