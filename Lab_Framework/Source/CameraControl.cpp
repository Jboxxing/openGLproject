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
	this->playerBody       = SceneObject();
}

CameraControl::CameraControl(GLuint _shaderProgram, glm::mat4 _viewMatrix, glm::mat4 _projectionMatrix) :
	shaderProgram(_shaderProgram), viewMatrix(_viewMatrix), projectionMatrix(_projectionMatrix)
{
	this->setShaderView(_shaderProgram, _viewMatrix);
	this->setShaderProjection(_shaderProgram, _projectionMatrix);
	this->playerBody = SceneObject("PLAYER", this->cameraPositon, _shaderProgram, 0, "../Assets/Models/cube.obj", 0);
}

CameraControl::CameraControl(GLuint _shaderProgram, glm::mat4 _projectionMatrix, glm::vec3 _cameraPositon, glm::vec3 _cameraLookAt) :
	shaderProgram(_shaderProgram), projectionMatrix(_projectionMatrix), cameraPositon(_cameraPositon), cameraLookAt(_cameraLookAt)
{
	this->setViewMatrix(glm::lookAt(_cameraPositon, _cameraLookAt, this->cameraUp));
	this->setProjectionMatrix(_projectionMatrix);
	this->playerBody = SceneObject("PLAYER", this->cameraPositon, _shaderProgram, 0, "../Assets/Models/cube.obj", 0);
}

CameraControl::~CameraControl()
{
}

glm::vec3 CameraControl::getCameraPosition() const
{
	return this->cameraPositon;
}

glm::vec3 CameraControl::getCameraLookAt() const
{
	return this->cameraLookAt;
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

SceneObject CameraControl::getSceneObject() const
{
	return this->playerBody;
}

void CameraControl::setSceneObject(SceneObject _player)
{
	this->playerBody = _player;
}

void CameraControl::firstPersonController(GLFWwindow* _window, GLuint _shader, glm::vec3 &_playerPos)
{   
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

	// Clamp Y position
	 
	this->cameraPositon.y = -3.0f;

	this->setViewMatrix(glm::lookAt(this->cameraPositon, this->cameraPositon + this->cameraLookAt, this->cameraUp));
	this->setShaderView(_shader, this->getViewMatrix());

	//_playertransform = glm::translate(glm::mat4(1.0f), this->cameraPositon + glm::vec3(0.0f, 0.0f, -10.0f));
	//std::cout << _playerPos.z << std::endl;
}

void CameraControl::thirdPersonController(GLFWwindow* _window, GLuint _shader, glm::vec3 &_playerPos, glm::mat4 &_playerTransform)
{
	glm::vec3 fixed(0.0f, -1.0f, -5.0f);

	glfwGetCursorPos(_window, &this->mousePosX, &this->mousePosY);

	float dt = glfwGetTime() - this->lastFrameTime;

	bool fastCam = glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
		float currentCameraSpeed = (fastCam) ? this->cameraFastSpeed : this->cameraSpeed;

	double dx = this->mousePosX - this->lastMousePosX;
	double dy = this->mousePosY - this->lastMousePosY;

	this->lastFrameTime += dt;
	this->lastMousePosX = this->mousePosX;
	this->lastMousePosY = this->mousePosY;

	this->cameraHorizontalAngle -= dx * this->cameraAngularSpeed * dt;
	this->cameraVerticalAngle -= dy * this->cameraAngularSpeed * dt;

	this->cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, this->cameraVerticalAngle));

	float theta = glm::radians(this->cameraHorizontalAngle);
	
	//this->setCameraLookAt(glm::vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta)));
	glm::vec3 cameraSideVector = glm::cross(glm::vec3(cosf(theta), 0.0f, sinf(theta)), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::normalize(cameraSideVector);
	

	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		_playerPos += dt * currentCameraSpeed * cameraSideVector;
			//* glm::vec3(0.0f, 0.0f, -1.0f);
	}

	if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		_playerPos += dt * currentCameraSpeed * cameraSideVector;
	}

	if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		_playerPos +=  dt * currentCameraSpeed * cameraSideVector;
	}

	if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		_playerPos += dt * currentCameraSpeed * cameraSideVector;
	}

	_playerPos.y += -4.0f;

	/// Clamp Y position ///
	glm::vec3 cameraPos = _playerPos + glm::vec3(0.0f, 1.0f, 8.0f);

	glm::mat4 rotation          = glm::rotate(glm::mat4(1.0f), -theta, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 translation       = glm::translate(glm::mat4(1.0f), _playerPos);

	_playerTransform = translation * rotation;

	this->setCameraPosition(cameraPos);
	this->setViewMatrix(glm::lookAt(cameraPos, _playerPos, glm::vec3(0.0f, 1.0f, 0.0f)));
	this->setShaderView(_shader, this->getViewMatrix());
}

