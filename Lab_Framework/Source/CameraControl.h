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

class CameraControl 
{
private:
	GLuint shaderProgram;
	
	float fov;
	float aspectRatio;
	float nearPlane;
	float farPlane;

	glm::vec3 cameraPositon;
	glm::vec3 cameraLookAt;
	const glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

public:

	const float cameraAngularSpeed = 40.0f;

	float cameraVerticalAngle;
	float cameraHorizontalAngle = 90.0f;
	int   lastMouseLeftState;

	float cameraSpeed;
	float cameraFastSpeed;

	float dt;
	float lastFrameTime;

	double mousePosX, mousePosY;
	double lastMousePosX, lastMousePosY;

	CameraControl();
	CameraControl(GLuint _shaderProgram, glm::mat4 _viewMatrix, glm::mat4 _projectionMatrix);
	CameraControl(GLuint _shaderProgram, glm::mat4 _projectionMatrix, glm::vec3 _cameraPositon, glm::vec3 _cameraLookAt);
	~CameraControl();

	void initCameraControls(GLFWwindow* _window);

	void setCameraPosition(glm::vec3 _position);
	void setCameraLookAt(glm::vec3 _lookAt);

	void setViewMatrix(glm::mat4 _viewMat);
	glm::mat4 getViewMatrix() const;
	void setProjectionMatrix(glm::mat4 _projMat);

	void setShaderView(GLuint _shaderProgram, glm::mat4 _viewMatrix);
	void setShaderProjection(GLuint _shaderProgram, glm::mat4 _projectionMatrix);

	void playerController(GLFWwindow * _window, GLuint _shader, glm::mat4 _viewMatrix, glm::vec3 _pos, glm::vec3 _look);
};
