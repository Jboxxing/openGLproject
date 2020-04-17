#include "SceneObject.h"

#include "OBJloader.h"
#include "OBJloaderV2.h"

void SceneObject::initProceduralVBO()
{
	std::vector<glm::vec3> vertices;

	srand((unsigned)time(0));

	for (int i = -2; i < 3; i++) 
	{
		for (int j = -2; j < 3; j++) 
		{
			glm::vec3 current = glm::vec3(j * 2.0f, 0, i * 2.0f);
			vertices.push_back(current);
			std::cout << current.x << " " << current.y << " " << current.z << std::endl;
		}
	}

	std::vector<glm::vec3> normals;



	std::vector<glm::vec2> UVs;

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

	////Normals VBO setup
	//GLuint normals_VBO;
	//glGenBuffers(1, &normals_VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	//glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	//glEnableVertexAttribArray(1);

	////UVs VBO setup
	//GLuint uvs_VBO;
	//glGenBuffers(1, &uvs_VBO);
	//glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	//glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	//glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	this->vertexCount = vertices.size();
	this->VAO = VAO;
}

void SceneObject::initVBO(std::string path)
{
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
	this->vertexCount = vertices.size();
	this->VAO = VAO;
}

void SceneObject::initEBO(std::string path)
{
	std::vector<int> vertexIndices; //The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> UVs;

	//read the vertices from the cube.obj file
	//We won't be needing the normals or UVs for this program
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
	this->vertexCount = vertexIndices.size();
	this->VAO = VAO;
}

SceneObject::SceneObject()
{
	std::cout << "Default object initialized." << std::endl;
	this->objectName  = "NaN";
	this->position    = glm::vec3(0.0f);
	this->shader      = 0;
	this->texture     = 0;
	this->VAO         = 0;
	this->vertexCount = 0;
}

SceneObject::SceneObject(std::string _name, glm::vec3 _position, GLuint _shader, GLuint _texture, std::string _objectPath, unsigned int _bufferType)
	: objectName(_name), position(_position), shader(_shader), texture(_texture), bufferType(_bufferType)
{
	if (_bufferType == 0)
		this->initVBO(_objectPath);
	else if (_bufferType == 1)
		this->initEBO(_objectPath);
	else if (_bufferType == 2)
		this->initProceduralVBO();
	else
	{
		std::cout << "VAO not generated. Setting to 0." << std::endl;
		this->VAO = 0;
		this->vertexCount = 0;
	}
}

SceneObject::~SceneObject()
{
}

void SceneObject::Draw() 
{
	if (!this->bufferType) 
	{
		glUseProgram(this->getShader());
		glBindTexture(GL_TEXTURE_2D, this->getTexture());
		glDrawArrays(GL_TRIANGLES, 0, this->getVertices());
	}
	else if (this->bufferType == 2)
	{
		glUseProgram(this->getShader());
		glDrawArrays(GL_TRIANGLES, 0, this->getVertices());
	}
	else 
	{
		glUseProgram(this->getShader());
		glBindTexture(GL_TEXTURE_2D, this->getTexture());
		glDrawElements(GL_TRIANGLES, this->getVertices(), GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
}

glm::vec3 SceneObject::getPosition() const
{
	return this->position;
}

void SceneObject::setPosition(glm::vec3 _position)
{
	this->position = _position;
}

GLuint SceneObject::getVAO() const
{
	return this->VAO;
}

GLuint SceneObject::getShader() const
{
	return this->shader;
}

void SceneObject::setShader(GLuint _shader)
{
	this->shader = _shader;
}

GLuint SceneObject::getTexture() const
{
	return this->texture;
}

void SceneObject::setTexture(GLuint _texture)
{
	this->texture = _texture;
}

int SceneObject::getVertices() const
{
	return this->vertexCount;
}
