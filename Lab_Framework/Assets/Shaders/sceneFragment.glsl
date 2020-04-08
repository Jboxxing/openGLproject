#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TextCoords;
in vec4 FragPosLightSpace;
 
uniform vec3 lightPos;
uniform vec3 viewPos;

//uniform vec3 lightColor;
//uniform vec3 objectColor;

const float shading_ambient_strength    = 0.4;
const float shading_diffuse_strength    = 0.2;
const float shading_specular_strength   = 0.9;

uniform sampler2D shadow_map;
uniform sampler2D diffuse_texture;

float ShadowCalculation(vec4 _fragPosLightSpace) {

    vec3 projCoords = _fragPosLightSpace.xyz / _fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    float closest_depth = texture(shadow_map, projCoords.xy).r;

    float current_depth = projCoords.z;

    float bias = 0.005;
    return ((current_depth - bias) < closest_depth) ? 1.0 : 0.0;
}

void main()
{
    vec3 ambient = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);

	vec3 color = texture(diffuse_texture, TextCoords).rgb;
	vec3 normal = normalize(Normal);
	
	vec3 lightColor = vec3(1.0f);

	// Ambient
	ambient = shading_ambient_strength * color;

	// Diffuse
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	diffuse = shading_diffuse_strength * diff * lightColor;

	// Specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	specular = shading_specular_strength * spec * lightColor;

	// Shadow
	float shadow = ShadowCalculation(FragPosLightSpace);

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
	// * objectColor;
    
    FragColor = vec4(lighting, 1.0f);
}

//void main()
//{
//    // ambient
//    float ambientStrength = 0.2;
//    vec3 ambient = ambientStrength * lightColor;
//  	
//    // diffuse 
//    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(lightPos - FragPos);
//    float diff = max(dot(lightDir, norm), 0.0);
//    vec3 diffuse = diff * lightColor;
//            
//	// specular
//    float specularStrength = 1.0;
//    vec3 viewDir = normalize(viewPos - FragPos);
//	// vec3 reflectDir = reflect(-lightDir, norm); 
//
//	vec3 halfwayDir = normalize(lightDir + viewDir);
//    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);
//    vec3 specular = spec * specularStrength * lightColor;
//
//    vec3 result = (ambient + specular + diffuse) * objectColor;
//    FragColor = vec4(result, 1.0);
//} 