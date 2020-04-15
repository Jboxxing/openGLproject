#include "CameraControl.h"

CameraControl::CameraControl() 
{
	this->shaderProgram    = 0;
	this->fov              = 0;
	this->aspectRatio      = 0;
	this->nearPlane        = 0;
	this->farPlane         = 0;
	this->cameraPositon    = glm::vec3(0.0f);
	this->cameraLookAt     = glm::vec3(0.0f);
	this->viewMatrix       = glm::mat4(1.0f);
	this->projectionMatrix = glm::mat4(1.0f);
}

CameraControl::CameraControl(GLuint _shaderProgram, glm::mat4 _viewMatrix, glm::mat4 _projectionMatrix) :
	shaderProgram(_shaderProgram), viewMatrix(_viewMatrix), projectionMatrix(_projectionMatrix)
{
	this->setShaderView(_shaderProgram, _viewMatrix);
	this->setShaderProjection(_shaderProgram, _projectionMatrix);
}

CameraControl::CameraControl(GLuint _shaderProgram, glm::mat4 _projectionMatrix, glm::vec3 _cameraPositon, glm::vec3 _cameraLookAt) :
	shaderProgram(_shaderProgram), projectionMatrix(_projectionMatrix), cameraPositon(_cameraPositon), cameraLookAt(_cameraLookAt)
{
	this->setViewMatrix(glm::lookAt(_cameraPositon, _cameraLookAt, this->cameraUp));
	this->setProjectionMatrix(_projectionMatrix);
}

CameraControl::~CameraControl()
{
}

glm::vec3 CameraControl::getCameraPosition() const
{
	return this->cameraPositon;
}

void CameraControl::setViewMatrix(glm::mat4 _viewMat)
{
	this->viewMatrix = _viewMat;
}

glm::mat4 CameraControl::getViewMatrix() const
{
	return this->viewMatrix;
}

void CameraControl::setProjectionMatrix(glm::mat4 _projMat)
{
	this->projectionMatrix = _projMat;
}

void CameraControl::setShaderView(GLuint _shaderProgram, glm::mat4 _viewMatrix)
{
	glUseProgram(_shaderProgram);
	GLuint viewMatrixLocation = glGetUniformLocation(_shaderProgram, "view");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &_viewMatrix[0][0]);
}

void CameraControl::setShaderProjection(GLuint _shaderProgram, glm::mat4 _projectionMatrix)
{
	glUseProgram(_shaderProgram);
	GLuint projectionMatrixLocation = glGetUniformLocation(_shaderProgram, "projection");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &_projectionMatrix[0][0]);
}

void CameraControl::setCameraPosition(glm::vec3 _position)
{
	this->cameraPositon = _position;
}

void CameraControl::setCameraLookAt(glm::vec3 _lookAt)
{
	this->cameraLookAt = _lookAt;
}

void CameraControl::initCameraControls(GLFWwindow* _window)
{
	this->lastMouseLeftState = GLFW_RELEASE;
	this->cameraVerticalAngle = 0.0f;
	this->cameraHorizontalAngle = 90.0f;
	this->cameraSpeed = 8.0f;
	this->cameraFastSpeed = this->cameraSpeed * 2;
	this->lastFrameTime = glfwGetTime();

	glfwGetCursorPos(_window, &this->lastMousePosX, &this->lastMousePosY);
}

void CameraControl::playerController(GLFWwindow* _window, GLuint _shader, glm::mat4 _viewMatrix, glm::vec3 _pos, glm::vec3 _look)
{   
	this->setViewMatrix(glm::lookAt(this->cameraPositon, this->cameraPositon + this->cameraLookAt, this->cameraUp));
	this->setShaderView(_shader, this->getViewMatrix());

	glfwGetCursorPos(_window, &this->mousePosX, &this->mousePosY);

	float dt = glfwGetTime() - this->lastFrameTime;

	bool fastCam = glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
		float currentCameraSpeed = (fastCam) ? this->cameraFastSpeed : this->cameraSpeed;

	double dx = this->mousePosX - this->lastMousePosX;
	double dy = this->mousePosY - this->lastMousePosY;

	this->lastFrameTime += dt;
	this->lastMousePosX = this->mousePosX;
	this->lastMousePosY  = this->mousePosY;

	this->cameraHorizontalAngle -= dx * this->cameraAngularSpeed * dt;
	this->cameraVerticalAngle -= dy * this->cameraAngularSpeed * dt;

	this->cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, this->cameraVerticalAngle));

	float theta = glm::radians(this->cameraHorizontalAngle);
	float phi = glm::radians(this->cameraVerticalAngle);

	this->setCameraLookAt(glm::vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta)));
	glm::vec3 cameraSideVector = glm::cross(this->cameraLookAt, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::normalize(cameraSideVector);

	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		this->cameraPositon += this->cameraLookAt * dt * currentCameraSpeed;
	}

	if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		this->cameraPositon -= this->cameraLookAt * dt * currentCameraSpeed;
	}

	if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		this->cameraPositon += cameraSideVector * dt * currentCameraSpeed;
	}

	if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		this->cameraPositon -= cameraSideVector * dt * currentCameraSpeed;
	}


}