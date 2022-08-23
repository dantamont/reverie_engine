#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

// Defines:

// Vertex info

in FragData{
	vec3 pos;
	vec4 worldPosition;
	vec4 vColor;
	vec2 uvCoord;
	vec3 surfaceNormal; // applies world matrix
	vec3 toCameraVector;
};

out vec4 fColor;

uniform int shaderMode;
// uniform mat4 worldMatrix;

// Uniform block for light settings
layout (std140) uniform LightSettingsBuffer
{
	int lightingModel;
	int lightCount;
};

struct Light {
	vec4 position;
	vec4 direction;
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
	vec4 attributes;
	vec4 intensity;
	ivec4 typeIndexAndFlags;
	// uint flags;
};

// struct VisibleIndex {
	// int index;
// };

// Shader storage buffer objects
layout(std430, binding = 0) readonly buffer LightBuffer {
	Light data[];
} lightBuffer;

// layout(std430, binding = 1) readonly buffer VisibleLightIndicesBuffer {
	// VisibleIndex data[];
// } visibleLightIndicesBuffer;


// Struct representing a material
//uniform bool useMaterial;
struct Material{
    bool useNormalMap; // If true, use unitNormal map for reflections, otherwise use geometry
	float shininess;
    vec3 specularity;
	float pad0; // vec3 are treated as vec4 in glsl, so need implicit padding for buffer
	mediump sampler2D diffuseTexture;
    bool useDiffuseTexture;
	mediump sampler2D normalMap;
};
uniform Material material;

// Calculate color for a single light ================================================
vec4 calcLightcolor(int lightIndex, vec3 unitVectorToCamera)
{
	Light light = lightBuffer.data[lightIndex];
    float intensity = light.intensity[0];

	// Get vectors to and from light
	int lightType = light.typeIndexAndFlags[0];
	vec3 toLightPosition = light.position.xyz - worldPosition.xyz;
	vec3 lightDirection = light.direction.xyz;
	if(lightType == 1){
		// Directional light
		toLightPosition = -lightDirection;
	}

	vec3 unitToLightVector = normalize(toLightPosition);
	vec3 fromLightVector = -unitToLightVector;
	float distance = length(toLightPosition);
		
	// Determine light attenuation
	vec4 attributes = light.attributes;
	float attenuationFactor = 1.0f;
	
	if(lightType == 0){
		// Only attenuate point lights
		attenuationFactor = attributes.x + (attributes.y * distance) + (attributes.z * distance * distance);
	}
		
	// Diffuse, with normal
	vec3 diffuseColor = light.diffuseColor.xyz;
	float normDotLight = dot(surfaceNormal, unitToLightVector);
	bool isSpotLight = lightType == 2;

	float spotTheta = 0.0f;
	float innerSpotCutoff = 0.0f;
	float outerSpotCutoff = 0.0f;
	if(isSpotLight){
		// TODO: Could use a flashlight texture to simulate real effect
		spotTheta = dot(unitToLightVector, -normalize(lightDirection));
		innerSpotCutoff = attributes[0];
		outerSpotCutoff = attributes[1];
		float epsilon = innerSpotCutoff - outerSpotCutoff;
		intensity *= smoothstep(0.0, 1.0, (spotTheta - outerSpotCutoff) / epsilon);
	}

	vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
	vec3 specular = vec3(0.0f, 0.0f, 0.0f);
	if(spotTheta > outerSpotCutoff || !isSpotLight){

		float brightness = max(normDotLight, 0.0);
	    diffuse = (intensity * brightness * diffuseColor)/attenuationFactor;
	
		// If using material diffuse texture
		if(material.useDiffuseTexture && shaderMode != 3){
			diffuse = diffuse * texture(material.diffuseTexture, uvCoord.st).xyz;
		}

		// Specular
	//	if(useMaterial){
		// Modify diffuse to use texture color

		// Calculate specular
		// TODO: Implement specular map
		float specularFactor;
		vec3 specularColor = light.specularColor.xyz;
		if(lightingModel == 0){
			// Phong lighting model
			vec3 reflectedLightDirection = reflect(fromLightVector, surfaceNormal);
			// unitVectorToCamera = viewPos - fragPos
			// fromLightVector = lightPos - fragPos
			specularFactor = dot(reflectedLightDirection, unitVectorToCamera);
			specularFactor = max(specularFactor, 0.0);
		}
		else{
			// Blinn-Phong lighting model
			// UNTESTED, see https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
			vec3 halfwayDir = normalize(fromLightVector + unitVectorToCamera); 
			specularFactor = dot(surfaceNormal, fromLightVector);
			specularFactor = max(specularFactor, 0.0);
		}
		float dampedFactor = pow(specularFactor, material.shininess); // Makes low values lower, doesn't affect higher values as much
		vec3 finalSpecular = dampedFactor * material.specularity * specularColor;
		specular = specular + (intensity * finalSpecular/attenuationFactor);
	//	}
	}

	// Calculate ambient color
	vec4 ambient = vec4(light.ambientColor.xyz, 1.0);
	if(material.useDiffuseTexture && shaderMode != 3){
		ambient = ambient * texture(material.diffuseTexture, uvCoord.st);
	}

	return vec4(diffuse + specular + vec3(ambient), 1.0f);
} 

// Perform lighting calculation for all lights =====================================
vec4 getLightingColor(){
	// Perform full lighting calculation
	vec3 unitVectorToCamera = normalize(toCameraVector);
    vec4 outColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i=0; i < lightCount; ++i){
		outColor = outColor + calcLightcolor(i, unitVectorToCamera);
	}	
	return outColor;
}

// Main ============================================================================
void main()
{
	vec4 textureColor = texture(material.diffuseTexture, uvCoord.st);
	if(shaderMode == 2){
		// Discard fragments with low opacity
		if(textureColor.a <= 0.2){
			discard;
		}
		
		// Purely textured shader mode
		fColor = textureColor;
	}
	else if(shaderMode == 3){
		// Lit shader mode (normals, no textures)
		fColor = getLightingColor();
	}
	else if(shaderMode == 4){
		// Discard fragments with low opacity
		if(textureColor.a <= 0.2){
			discard;
		}
		
		// Textured shader mode with normals
		fColor = getLightingColor();
	}
	else{
		// Default to input color
		fColor = vColor;
	}
    
}