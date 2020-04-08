#version 330 core          

// uniform vec3 view_position;           

layout (location = 0) in vec3 aPos;     
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

out	vec3 FragPos;
out	vec3 Normal;
out	vec2 TextCoords;
out	vec4 FragPosLightSpace;

out vec3 vertexColor;
// out vec2 vertexUV;

uniform mat4 worldMatrix      = mat4(1.0);
uniform mat4 viewMatrix       = mat4(1.0);
uniform mat4 projectionMatrix = mat4(1.0);

uniform mat4 light_space_matrix;

void main()                             
{	
	FragPos = vec3(worldMatrix * vec4(aPos, 1.0));
	Normal = transpose(inverse(mat3(worldMatrix))) * aNormal;
	// TextCoords = aUV;
	FragPosLightSpace = light_space_matrix * vec4(FragPos, 1.0);

	vertexColor = aNormal;
	mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;
	gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	// vertexUV = aUV;
}

