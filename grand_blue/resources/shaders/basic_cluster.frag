#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

// Defines:

// Vertex info
in vec4 fragPos;
in vec4 worldPosition;
in vec4 vColor;
in vec2 uvCoord;
in vec3 surfaceNormal;
in vec3 toCameraVector;
// in vec4 posLightSpace;

out vec4 fColor;

uniform int shaderMode;
uniform vec2 viewportPercentSize; // Viewport size as a percentage of screen size

// Set to texture unit binding 0 (supported since OpenGL 4.2)
layout(binding=0) uniform samplerCubeArray pointShadowMap;
layout(binding=1) uniform sampler2DArray dirShadowMap;
layout(binding=2) uniform sampler2DArray spotShadowMap;
layout(binding=3) uniform sampler2D ssaoTexture;

layout (std140) uniform CameraBuffer
{
	mat4 viewMatrix;
	mat4 invViewMatrix;
	mat4 projectionMatrix;
	mat4 invProjectionMatrix;
	float zNear;
	float zFar;
	uvec2 viewportDimensions; // In pixels
	vec4 screenPercentage; // Screen percentage of viewport, padded for alignment with empty z, w
};

// Uniform block for light settings
layout (std140) uniform LightSettingsBuffer
{
	int lightingModel;
	int totalLightCount; // total number of lights in the scene
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
};

struct LightGrid{
    uint offset;
    uint count;
};

// Shader storage buffer objects
layout(std430, binding = 0) readonly buffer LightBuffer {
	Light data[];
} lightBuffer;

layout (std430, binding = 2) buffer ScreenToViewBuffer{
    mat4 inverseProjection;
    uvec4 tileGridSizes;
	uvec4 tilePixelSizes;
    uvec2 screenDimensions; // This is framebuffer size, not actual widget size
    float scale;
    float bias;
};

layout (std430, binding = 4) buffer LightIndexBuffer{
    uint globalLightIndexList[];
};
layout (std430, binding = 5) buffer LightGridBuffer{
    LightGrid lightGrid[];
};

layout (std430, binding = 7) buffer ActiveClusterBuffer{
	uint activeClusters[];
	// uint uniqueActiveClusters[];
};

// Remember, alignment is that of largest data member for std430
struct ShadowInfo{
	// 0 - Index of each light's shadow map, index is negative if not associated with a shadow map
	// 1 - minimum shadow bias (for getting rid of artifacts)
	// 2 - maximum shadow bias (for getting rid of artifacts)
	// 3 - The far plane of the sphere camera (for point light)
	vec4 attributes;
	vec4 _pad; 
	vec4 _pad0;
	vec4 _pad1;
	
	// Light-space matrix associated with each shadow map
	// For point lights, stores additional attributes:
	// [0][0] Near clip plane
	mat4 lightSpaceMatrix;
};

// Shadow casting
layout(std430, binding = 8) readonly buffer ShadowBuffer {
	ShadowInfo shadows[];
} shadowBuffer;

// TODO: Use UBO
// Struct representing a material
//uniform bool useMaterial;
struct Material{
	float shininess;
    vec3 specularity;
	float pad0; // vec3 are treated as vec4 in glsl, so need implicit padding for buffer
	mediump sampler2D diffuseTexture;
	mediump sampler2D normalMap;
	mediump sampler2D specularTexture;
};
uniform Material material;



vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

// Function Prototypes ============================================
float linearDepth(float depthSample, float zNear, float zFar);
float depthSample(float linearDepth, float zNear, float zFar);
vec4 frag2ndc(vec4 fpos, vec4 viewport);
vec4 frag2clip(vec4 frag, vec4 viewport);
vec4 clipToView(vec4 clip);
vec2 screen2UV(vec4 screen);
vec4 screen2Clip(vec4 screen);
vec4 screen2View(vec4 screen);

// Calculate the shadow factor =======================================================
// Returns 1.0 when the fragment is in shadow, and 0.0 if it is not
// TODO: See variance shadow maps:
// http://www.punkuser.net/vsm/
float shadowCalculation(uint lightIndex, float normDotLight, sampler2DArray shadowMap){
	// Get shadow info
	// See: https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
	ShadowInfo shadow = shadowBuffer.shadows[lightIndex];
	int mapIndex = int(shadow.attributes[0]);
	vec4 fragPosLightSpace = shadow.lightSpaceMatrix * worldPosition;

	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // Mapping to clip space by dividing z by w component
	
	// Map from NDC [-1, 1] to range to [0,1] 
	projCoords = projCoords * 0.5 + 0.5;
	vec2 sampleCoords = projCoords.xy;
	
	// Get closest depth from the light's POV
	float sampleDepth = texture(shadowMap, vec3(sampleCoords, mapIndex)).r;   

	// Get the current depth at this fragment
	float currentDepth = projCoords.z;
	
	// Check whether or not the current frag position is in shadow
	
	// Use a bias to avoid moire pattern so fragments are not incorrectly considered below the surface
	// Max bias of 0.05, min of 0.005
	// Surfaces almost perpendicular to the light (like a floor) get a small bias, while sides of things more parallel get a larger bias
	// Increment bias until acne removed
	float bias = shadow.attributes[1]; // 0.005 is a good default
	float maxBias = shadow.attributes[2]; // 0.05 is a good default
	float shadowBias = bias*tan(acos(normDotLight)); 
	shadowBias = clamp(shadowBias, 0.0, maxBias);
	
	// Implement PCF (percentage-closer filtering)
	float shadowFactor =0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy; 
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, mapIndex)).r; 
			shadowFactor += float(currentDepth - shadowBias > pcfDepth);
		}    
	}
	shadowFactor /= 9.0;
	
	// Don't want a dark region when depth greater than 1.0, result of GL_CLAMP_TO_BORDER for shadow map
	if(projCoords.z > 1.0){
        shadowFactor = 0.0;
	}
	
	return shadowFactor;
}

// Shadow calculation for point lights
// Samples differently than directional and spot lights, so is simpler,
// and doesn't necessitate a light matrix anymore
float pointShadowCalculation(uint lightIndex, float normDotLight, samplerCubeArray shadowMap)
{
	// Get shadow info
	// See: https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
	ShadowInfo shadow = shadowBuffer.shadows[lightIndex];
	Light light = lightBuffer.data[lightIndex];
	int mapIndex = int(shadow.attributes[0]);
	// vec4 fragPosLightSpace = shadow.lightSpaceMatrix * worldPosition;

    // Get vector between fragment position and light position
    vec4 fragToLight = worldPosition - light.position;
    // use the light to fragment vector to sample from the depth map    
    // float sampledDepth = texture(shadowMap, vec4(fragToLight.xyz, mapIndex)).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
	// float nearPlane = shadow.lightSpaceMatrix[0][0];
	float farPlane = shadow.attributes[3];
	// float closestDepth = sampledDepth * farPlane;
    // float closestDepth = linearDepth(sampledDepth, nearPlane, farPlane);
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
	float bias = shadow.attributes[1]; // 0.005 is a good default
	float maxBias = shadow.attributes[2]; // 0.05 is a good default
	// float shadowBias = bias*tan(acos(normDotLight)); 
	// float shadowBias = max(maxBias * (1.0 - normDotLight), bias);
	// shadowBias = clamp(shadowBias, 0.0, maxBias);
	float shadowBias = bias;
    // float shadowFactor = float(currentDepth -  shadowBias > closestDepth);

	float shadowFactor = 0.0;
	int numSamples  = 20;
	float viewDistance = length(toCameraVector);
	// float diskRadius = 0.05; 
	float diskRadius = (1.0 + 10.0*(viewDistance / farPlane)) / 20.0;  // make shadows softer when farther away
	for(int i = 0; i < numSamples; ++i)
	{
		float closestDepth = texture(shadowMap, vec4(fragToLight.xyz + sampleOffsetDirections[i] * diskRadius, mapIndex)).r;
		closestDepth *= farPlane;   // undo mapping [0;1]
		shadowFactor += float(currentDepth - shadowBias > closestDepth);
	}
	shadowFactor /= float(numSamples);  


	// Don't show shadows if shadow map was saturated
	// if(sampledDepth >= 1.0){
        // shadowFactor = 0.0;
	// }
	
    return shadowFactor;
}  

// Calculate color for a single light ================================================
vec4 calcLightcolor(uint lightIndex, vec3 unitVectorToCamera)
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
		diffuse = diffuse * texture(material.diffuseTexture, uvCoord.st).xyz;

		// Specular
		// Modify diffuse to use texture color

		// Calculate specular
		// TODO: Implement specular map
		float specularFactor;
		vec3 specularColor = light.specularColor.xyz;
		if(lightingModel == 0){
			// Phong lighting model
			vec3 reflectedLightDirection = reflect(fromLightVector, surfaceNormal);
			specularFactor = dot(reflectedLightDirection, unitVectorToCamera);
			specularFactor = max(specularFactor, 0.0);
		}
		else{
			// Blinn-Phong lighting model
			// UNTESTED, see https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
			vec3 halfwayDir = normalize(unitToLightVector + unitVectorToCamera); 
			specularFactor = dot(surfaceNormal, halfwayDir);
			specularFactor = max(specularFactor, 0.0);
		}
		float dampedFactor = pow(specularFactor, material.shininess); // Makes low values lower, doesn't affect higher values as much
		vec3 fragSpecularity;
		if(textureSize(material.specularTexture, 0).x > 1){
			// If using specular map
			fragSpecularity = texture(material.specularTexture, uvCoord.st).xyz;
		}else{
			fragSpecularity = material.specularity;
		}
		vec3 finalSpecular = dampedFactor * fragSpecularity * specularColor;
		specular = intensity * finalSpecular/attenuationFactor;
	}

	// Calculate ambient color
	vec3 ambient = light.ambientColor.xyz;
	vec2 screenCoords = ((fragPos.xy / fragPos.w) + 1.0)/2.0; 	// Convert from clip-space to UV coordinates ([-1, 1] to [0, 1])
	float ssaoFactor = texture2D(ssaoTexture, screenCoords).r;
	ambient = ambient * texture(material.diffuseTexture, uvCoord.st).xyz; // Texture is white if no diffuse, so should always work
	ambient *= ssaoFactor;
	
	// Technically, diffuse shouldn't use ssao, but it's more pronounced with it
	diffuse *= ssaoFactor;
	
	// Calculate shadow factor
	ShadowInfo shadow = shadowBuffer.shadows[lightIndex];
	int mapIndex = int(shadow.attributes[0]);
	float farClip = shadow.attributes[3];
	float shadowFactor = 0.0;
	if(lightType == 0){
		// Point light
		shadowFactor = mapIndex >= 0 ? pointShadowCalculation(lightIndex, normDotLight, pointShadowMap): 0.0;
	}
	else if(lightType == 1){
		// Directional light
		shadowFactor = mapIndex >= 0 ? shadowCalculation(lightIndex, normDotLight, dirShadowMap): 0.0;
	}
	else if(lightType == 2){
		// Spot light
		shadowFactor = mapIndex >= 0 ? shadowCalculation(lightIndex, normDotLight, spotShadowMap): 0.0;
	}
	
	return vec4((1.0 - shadowFactor) * (diffuse + specular) + ambient, 1.0f);
	// if(lightType == 0){
		// float sampleDepth = texture(pointShadowMap, vec4(fromLightVector.xyz, mapIndex)).r;
		// return vec4(vec3(sampleDepth / farClip), 1.0);
	// }
	// else{
		// return vec4(0.0, 0.0, 0.0, 1.0);
	// }
} 

// Perform lighting calculation for all lights =====================================
vec4 getLightingColor(){

	// Locating which cluster you are a part of, i.e., get slice
	// slice = |log(Z) * numSlices/log(farZ/nearZ) - numSlices*log(nearZ)/log(farZ/nearZ)|
	// float scale = tileGridSizes.z / log2(zFar / zNear);
	// float bias = -(tileGridSizes.z * log2(zNear) / log2(zFar / zNear));
    uint zCluster     = uint(max(log2(linearDepth(gl_FragCoord.z, zNear, zFar)) * scale + bias, 0.0));
    uvec3 clusters    = uvec3( uvec2( vec2(gl_FragCoord.x / tilePixelSizes[0], gl_FragCoord.y / tilePixelSizes[1] )), zCluster);
    uint clusterIndex = clusters.x +
                     tileGridSizes.x * clusters.y +
                     (tileGridSizes.x * tileGridSizes.y) * clusters.z;  
	
	// Check if cluster is active
	vec4 outColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	if(activeClusters[clusterIndex] == 1){
	
		// Get light index
		uint lightIndexOffset = lightGrid[clusterIndex].offset;
		uint lightCount       = lightGrid[clusterIndex].count;
		
		// Perform full lighting calculation
		vec3 unitVectorToCamera = normalize(toCameraVector);
		for(int i=0; i < lightCount; ++i){
			uint lightVectorIndex = globalLightIndexList[lightIndexOffset + i];
			outColor = outColor + calcLightcolor(lightVectorIndex, unitVectorToCamera);
		}	
	
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
	
	// vec2 fragCoords = (fragPos.xy/ fragPos.w);
	// vec2 screenCoords = fragCoords * 0.5 + 0.5;
	// float ssaoFactor = texture2D(ssaoTexture, screenCoords).r;
	// fColor = vec4(ssaoFactor, ssaoFactor, ssaoFactor, 1.0);
}

// See: https://stackoverflow.com/questions/51108596/linearize-depth
// https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer/6657284#6657284
float linearDepth(float depthSample, float zNear, float zFar){
	// Map depth back from [0, 1] to [-1, 1]
    float depthRange = 2.0 * depthSample - 1.0;
	
    // Retrieve true depth value (distance from camera) from depth buffer value
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
	return linear;
}

float depthSample(float linearDepth, float zNear, float zFar)
{
	// Normalize depth from true value to [-1, 1] range
    float nonLinearDepth = (zFar + zNear - 2.0 * zNear * zFar / linearDepth) / (zFar - zNear);
	
	// Map from [-1, 1] to [0, 1]
    nonLinearDepth = (nonLinearDepth + 1.0) / 2.0;
	
    return nonLinearDepth;
}

// See: https://stackoverflow.com/questions/38938498/how-do-i-convert-gl-fragcoord-to-a-world-space-point-in-a-fragment-shader
vec4 frag2ndc(vec4 fpos, vec4 viewport){
	vec4 ndcPos;
	ndcPos.xy = ((2.0 * fpos.xy) - (2.0 * viewport.xy)) / (viewport.zw) - 1;
	ndcPos.z = (2.0 * fpos.z - gl_DepthRange.near - gl_DepthRange.far) /
		(gl_DepthRange.far - gl_DepthRange.near);
	ndcPos.w = 1.0;
	
	return ndcPos;
}

vec4 frag2clip(vec4 frag, vec4 viewport){
	vec4 ndc = frag2ndc(frag, viewport);
	return ndc / frag.w;
}

vec4 clipToView(vec4 clip){
    //View space transform
	vec4 view = inverseProjection * clip;

    //Perspective projection
	view = view / view.w;
    
    return view;
}

vec2 screen2UV(vec4 screen){
    //Convert to NDC
    vec2 texCoord = screen.xy / screenDimensions.xy;
    return texCoord;
}

vec4 screen2Clip(vec4 screen){
    //Convert to NDC
    vec2 texCoord = screen2UV(screen);

    //Convert to clipSpace
    vec4 clip = vec4(vec2(texCoord.x, texCoord.y)* 2.0 - 1.0, screen.z, screen.w);

    return clip;
}

//Converts a point's coordinate system from screen space to view space
// See: https://stackoverflow.com/questions/26691029/glsl-compute-world-coordinate-from-eye-depth-and-screen-position
vec4 screen2View(vec4 screen){
    return clipToView(screen2Clip(screen));
}