// Defines:
#define MAX_LIGHTS 20
#define MAX_BONES 100

// See: https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
// The use of layout forgos the use of glBindAttribLocation entirely. 
// If you try to combine the two and they conflict, the layout qualifier always wins.
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in ivec4 boneIDs;
layout(location = 6) in vec4 boneWeights;

// Could use an interface block, instead of having to pull in all the uniforms individually
// out NAME{
// vec3 c;
// } outName;

out vec3 pos;
out vec4 vColor;
out vec2 uvCoord;
out vec3 surfaceNormal; // applies world matrix
out vec3 toLightPositions[MAX_LIGHTS];
out vec3 toCameraVector;

uniform int shaderMode;
uniform float diffuseColorScale;

uniform mat4 worldMatrix;

// Uniform block for projection and view matrices
layout (std140) uniform CameraMatrices
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

uniform mat4 boneTransforms[MAX_BONES];
uniform mat4 globalInverseTransform;
uniform mat4 inverseBindPoseTransforms[MAX_BONES];
uniform bool isAnimated;
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


// struct Light {
	// vec4 position;
	// vec4 direction;
	// vec4 ambientColor;
	// vec4 diffuseColor;
	// vec4 specularColor;
	// vec4 attributes;
	// vec4 typeAndIntensity;
// };

// struct VisibleIndex {
	// int index;
// };

// Shader storage buffer objects
// layout(std430, binding = 0) readonly buffer LightBuffer {
	// Light data[];
// } lightBuffer;

// layout(std430, binding = 1) readonly buffer VisibleLightIndicesBuffer {
	// VisibleIndex data[];
// } visibleLightIndicesBuffer;



// Struct representing a material
struct Material{
    bool useNormalMap; // If true, use unitNormal map for reflections, otherwise use geometry
	float shininess;
    vec3 specularity;
	float pad0; // vec3 are treated as vec4 in glsl, so need implicit paddiing for buffer
	mediump sampler2D diffuseTexture;
	// TODO, specular and other maps
    bool useDiffuseTexture;
	mediump sampler2D normalMap;
};
uniform Material material;

vec3 calcMapNormal()
{
	vec3 unitNormal = normalize(normal);
	if(material.useNormalMap){
		// See: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		// http://ogldev.atspace.co.uk/www/tutorial26/tutorial26.html
		// Calculate unitNormal from map
		vec3 unitTangent = normalize(tangent);

		// Gramm-schmidt method to re-orthogonalize
		unitTangent = normalize(unitTangent - dot(unitTangent, unitNormal) * unitNormal);
		vec3 bitangent = cross(unitTangent, unitNormal);

		// obtain unitNormal from unitNormal map in range [0,1]
		vec3 mapNormal = texture(material.normalMap, uvCoord.st).rgb;
		// transform mapNormal vector to range [-1, 1]
		mapNormal = normalize(2.0 * mapNormal - 1.0);

		// Obtain TBN matrix
		mat3 TBN = mat3(unitTangent, bitangent, unitNormal);

		// Obtain normal vector (object-space)
		vec3 newNormal = TBN * mapNormal;
		newNormal = normalize(newNormal);
		return newNormal;
	}else{
		return unitNormal;
	}
}

void main()
{
	// Assign GL position
	vec4 posL = vec4(position, 1.0);
	surfaceNormal = (worldMatrix * vec4(calcMapNormal(), 0.0)).xyz;

	if(isAnimated){
		mat4 boneTransform = boneTransforms[boneIDs[0]] * inverseBindPoseTransforms[boneIDs[0]] * boneWeights[0];
		boneTransform     += boneTransforms[boneIDs[1]] * inverseBindPoseTransforms[boneIDs[1]] * boneWeights[1];
		boneTransform     += boneTransforms[boneIDs[2]] * inverseBindPoseTransforms[boneIDs[2]] * boneWeights[2];
		boneTransform     += boneTransforms[boneIDs[3]] * inverseBindPoseTransforms[boneIDs[3]] * boneWeights[3];
		posL = globalInverseTransform * boneTransform * posL;

		// Want to reorient, but be position-independent
		surfaceNormal = (boneTransform * vec4(surfaceNormal, 0.0)).xyz;
	}

	vec4 worldPosition = worldMatrix * posL;

    gl_Position = projectionMatrix * viewMatrix * worldPosition;
  
	// Assign output variables
	pos = gl_Position.xyz;
	uvCoord = uv;
	toCameraVector = (inverse(viewMatrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz - worldPosition.xyz;
	
	if(shaderMode == 1){
		// Colored shader mode
		vColor = color * diffuseColorScale;
	}
	else if(shaderMode == 3 || shaderMode == 4){
		// Assign output variables
		for(int i=0; i < lightCount; ++i){
			// Directional light do not need a position
			// Need to make direction TOWARDS the light source, is specified as FROM the light source
			toLightPositions[i] = lightPositions[i].xyz - worldPosition.xyz;
		}
	}
	else{
		// Default to white color
		vColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
}