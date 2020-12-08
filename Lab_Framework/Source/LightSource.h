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

class LightSource 
{
public:
	LightSource();
	//LightSource();
	~LightSource();
};