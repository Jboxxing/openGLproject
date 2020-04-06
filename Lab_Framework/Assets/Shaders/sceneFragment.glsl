#version 330 core
// More or less useful
const float PI = 3.1415926535897932384626433832795;

in vec3 FragPos;
in vec3 Normal;
in vec2 TextCoords;
in vec4 FragPosLightSpace;

uniform vec3 objColor;

uniform vec3 shading_ambient_strength    = vec3(1.0f);  
	// vec3(0.329412f, 0.223529f, 0.027451f);	 
uniform vec3 shading_diffuse_strength    = vec3(1.0f);  
	// vec3(0.780392f, 0.568627f, 0.113725f);		 
uniform vec3 shading_specular_strength   = vec3(1.0f);   
	// vec3(0.992157f, 0.941176f, 0.807843f);

uniform float alpha = 1.0f;

uniform vec3 light_color;
uniform vec3 light_direction;

uniform vec3 light_position;
uniform vec3 view_position;

uniform sampler2D shadow_map;
uniform sampler2D textureSampler;

in vec3 vertexColor;
in vec2 vertexUV;

out vec4 FragColor;

float shadow_scalar(vec4 fragPosLightSpace) {
	// this function returns 1.0 when the surface receives light, 
	// and 0.0 when it is in a shadow perform perspective divide

	vec3 projected_coordinates = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// transform to [0,1] range

	projected_coordinates = projected_coordinates * 0.5 + 0.5;

	// get closest depth value from light's perspective (using [0,1] range fragment_position_light_space as coords)
    
	float closest_depth = texture(shadow_map, projected_coordinates.xy).r;
    
	// get depth of current fragment from light's perspective
    
	float current_depth = projected_coordinates.z;
    
	// check whether current frag pos is in shadow
	vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light_position - FragPos);
	float bias = max(0.05 * (1.0 - dot(normal, light_direction)), 0.005);
	float shadow = 0.0;

	vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadow_map, projected_coordinates.xy + vec2(x, y) * texelSize).r; 
			shadow += current_depth - bias > pcfDepth  ? 1.0 : 0.0;        
		}    
	}

	shadow /= 9.0;

	if(projected_coordinates.z > 1.0)
		shadow = 0.0;
		
	return shadow;
	//return ((current_depth - bias) < closest_depth) ? 1.0 : 0.0;
}

void main()
{
	vec3 ambient = vec3(0.0f);
	vec3 diffuse = vec3(0.0f);
	vec3 specular = vec3(0.0f);
	vec3 color = objColor;

	// Texture mapping
	// vec4 textureColor = texture(textureSampler, TextCoords);
	//vec4 textureColor = mix(texture(textureSampler, fs_in.TextCoords),  texture(shadow_map, fs_in.TextCoords), alpha);
	vec3 normal = normalize(Normal);

	// Light Color
	// vec3 lightColor = vec3(0.4);
	
	// Ambient
	ambient = 0.5 * shading_ambient_strength;
		// * textureColor.rgb;

	// Diffuse
	vec3 lightDir = normalize(light_position - FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	diffuse = diff * shading_diffuse_strength * light_color;
	
	// Specular
	vec3 viewDir = normalize(view_position - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	specular = spec * shading_specular_strength * light_color;

	// Shadow
	float shadow = shadow_scalar(FragPosLightSpace);

	// Checks if there is a texture
	// if (textureColor.x == 0 && textureColor.y == 0 && textureColor.z == 0) { textureColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); }
	// if (objColor.x == 0 && objColor.y == 0 && objColor.z == 0) { color = vec3(1.0f, 1.0f, 1.0f); }
	//  
	vec3 result = (ambient + (1.0 - shadow) * (specular + diffuse));
	vec4 preTexture = vec4(result, 1.0f);

	// OPACITY CHECK
	// FragColor = vec4(vec3(textureColor), alpha) * preTexture;
	// FragColor = vec4(vec3(textureColor * preTexture), alpha);
	FragColor = vec4(vec3(color), alpha) * preTexture;
}