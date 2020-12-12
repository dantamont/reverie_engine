#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

// Defines:
# define MAX_LIGHTS 20

// Vertex info
in vec3 pos;
in vec4 vColor;
in vec2 uvCoord;
in vec3 surfaceNormal;

in vec3 toLightPositions[MAX_LIGHTS];
in vec3 toCameraVector;

// Uniform block for projection and view matrices
layout (std140) uniform CameraBuffer
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	float zNear;
	float zFar;
};

out vec4 fColor;

uniform int shaderMode;

// TODO: Create light struct
// https://learnopengl.com/Lighting/Light-casters
// https://learnopengl.com/Lighting/Multiple-lights
layout (std140) uniform LightBuffer
{
	int lightingModel;
	int lightCount;
	vec4 lightPositions[MAX_LIGHTS];
	vec4 lightDirections[MAX_LIGHTS];
	vec4 lightAmbientColors[MAX_LIGHTS];
	vec4 lightDiffuseColors[MAX_LIGHTS];
	vec4 lightSpecularColors[MAX_LIGHTS];
	vec4 lightAttributes[MAX_LIGHTS];
	vec4 lightAttributes1[MAX_LIGHTS]; // types, intensities
};

// Struct representing a material
//uniform bool useMaterial;
struct Material{
    bool useNormalMap; // If true, use unitNormal map for reflections, otherwise use geometry
	float shininess;
    vec3 specularity;
	float pad0; // vec3 are treated as vec4 in glsl, so need implicit paddiing for buffer
	mediump sampler2D diffuseTexture;
    bool useDiffuseTexture;
	mediump sampler2D normalMap;
};
uniform Material material;

// Effects
uniform vec3 rimColor;

// Calculate color for a single light ================================================
vec4 calcLightColor(int lightIndex, vec3 unitVectorToCamera)
{
    float intensity = lightAttributes1[lightIndex][1];

	// Get vectors to and from light
	int lightType = int(lightAttributes1[lightIndex][0]);
	vec3 toLightPosition = toLightPositions[lightIndex];
	vec3 lightDirection = lightDirections[lightIndex].xyz;
	if(lightType == 1){
		// Directional light
		toLightPosition = -lightDirection;
	}

	vec3 unitToLightVector = normalize(toLightPosition);
	vec3 fromLightVector = -unitToLightVector;
	float distance = length(toLightPosition);
		
	// Determine light attenuation
	vec4 attributes = lightAttributes[lightIndex];
	float attenuationFactor = 1.0f;
	
	if(lightType == 0){
		// Only attenuate point lights
		attenuationFactor = attributes.x + (attributes.y * distance) + (attributes.z * distance * distance);
	}
		
	// Diffuse, with normal
	vec3 diffuseColor = lightDiffuseColors[lightIndex].xyz;
	float normDotLight = dot(surfaceNormal, unitToLightVector);
	bool isSpotLight = lightType == 2;

	float spotTheta = 0.0f;
	float innerSpotCutoff = 0.0f;
	float outerSpotCutoff = 0.0f;
	if(isSpotLight){
		// TODO: Could use a flashlight texture to simulate real effect
		spotTheta = dot(unitToLightVector, -normalize(lightDirection));
		innerSpotCutoff = lightAttributes[lightIndex][0];
		outerSpotCutoff = lightAttributes[lightIndex][1];
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
		vec3 specularColor = lightSpecularColors[lightIndex].xyz;
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
	vec4 ambient = lightAmbientColors[lightIndex];
	ambient.w = 1.0;
	ambient = ambient * texture(material.diffuseTexture, uvCoord.st);

	return vec4(diffuse + specular + vec3(ambient), 1.0f);
} 

// Perform lighting calculation for all lights =====================================
vec4 getLightingColor(){
	// Perform full lighting calculation
	vec3 unitVectorToCamera = normalize(toCameraVector);
    vec4 outColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i=0; i < lightCount; ++i){
		outColor = outColor + calcLightColor(i, unitVectorToCamera);
	}	
	return outColor;
}

void main()
{
	// Everything from basic shader
	if(shaderMode == 2){
		// Purely textured shader mode
		vec4 textureColor = texture(material.diffuseTexture, uvCoord.st);
		fColor = textureColor;
	}
	else if(shaderMode == 3){
		// Lit shader mode (normals, no textures)
		fColor = getLightingColor();
	}
	else if(shaderMode == 4){
		// Textured shader mode with normals
		fColor = getLightingColor();
	}
	else{
		// Default to input color
		fColor = vColor;
	}
    
	// Effects 
	// Rim lighting
	bool useRim = true;
	if(useRim){
		vec3 viewNormal = normalize(mat3(viewMatrix) * surfaceNormal);
		vec3 viewPos = vec3(viewNormal * pos);
		vec3 eye = normalize(viewPos); // eye vector
		float rimContribution = 1.0 - max(dot(eye, viewNormal), 0.0);
		float rimMultiplier = 1.0;
		rimContribution = pow(rimContribution, rimMultiplier);
		float rimMin = 0.0;
		float rimMax = 1.0;
		vec3 rim = vec3(smoothstep(rimMin, rimMax, rimContribution));
		rim = rim * rimColor;

		fColor.rgb = fColor.rgb + rim;
	}

}