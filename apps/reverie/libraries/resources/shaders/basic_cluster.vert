// Defines:
#define MAX_BONES 125

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

out vec4 fragPos;
out vec4 worldPosition;
out vec4 vColor;
out vec2 uvCoord;
out vec3 surfaceNormal; // applies world matrix
out vec3 toCameraVector;
out vec4 posLightSpace;

uniform int shaderMode;
uniform float diffuseColorScale;

uniform mat4 worldMatrix;

// Uniform block for projection and view matrices
layout (std140) uniform CameraBuffer
{
	mat4 viewMatrix;
	mat4 invViewMatrix;
	mat4 projectionMatrix;
	mat4 invProjectionMatrix;
	float zNear;
	float zFar;
	uvec2 viewportDimensions;
	vec4 screenPercentage; // Screen percentage of viewport
};

uniform mat4 boneTransforms[MAX_BONES];
uniform mat4 globalInverseTransform;
uniform mat4 inverseBindPoseTransforms[MAX_BONES];
uniform bool isAnimated;

// Shadows
// uniform mat4 lightSpaceMatrix;

// Struct representing a material
struct Material{
	float shininess;
    vec3 specularity;
	float pad0; // vec3 are treated as vec4 in glsl, so need implicit paddiing for buffer
	mediump sampler2D diffuseTexture;
	mediump sampler2D normalMap;
	mediump sampler2D specularTexture;
	mediump sampler2D opacityTexture;
};
uniform Material material;

// Could alternatively pass in TBN matrix
vec3 calcMapNormal()
{
	vec3 unitNormal = normalize(normal);
	vec3 mapNormal = texture(material.normalMap, uv).rgb;
	
	// Check if normal map texture is blank (nlank is 1x1 white texture)
	if(textureSize(material.normalMap, 0).x > 1){
	// if(false){
		// See: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		// http://ogldev.atspace.co.uk/www/tutorial26/tutorial26.html
		// Calculate unitNormal from map
		vec3 unitTangent = normalize(tangent);

		// Gramm-schmidt method to re-orthogonalize
		unitTangent = normalize(unitTangent - dot(unitTangent, unitNormal) * unitNormal);
		vec3 bitangent = cross(unitTangent, unitNormal);

		// obtain unitNormal from unitNormal map in range [0,1]
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
	surfaceNormal = calcMapNormal();

	if(isAnimated){
		mat4 boneTransform = boneTransforms[boneIDs[0]] * inverseBindPoseTransforms[boneIDs[0]] * boneWeights[0];
		boneTransform     += boneTransforms[boneIDs[1]] * inverseBindPoseTransforms[boneIDs[1]] * boneWeights[1];
		boneTransform     += boneTransforms[boneIDs[2]] * inverseBindPoseTransforms[boneIDs[2]] * boneWeights[2];
		boneTransform     += boneTransforms[boneIDs[3]] * inverseBindPoseTransforms[boneIDs[3]] * boneWeights[3];
		posL = globalInverseTransform * boneTransform * posL;

		// Want to reorient, but be position-independent
		surfaceNormal = (boneTransform * vec4(surfaceNormal, 0.0)).xyz;
	}
	
	// Fix non-uniform scaling on normal while converting to world space
	// https://gamedev.stackexchange.com/questions/168407/normal-matrix-in-plain-english
	// TODO: Move to CPU, this is not performant at all
	mat3 normalMatrix = mat3(transpose(inverse(worldMatrix)));
	// surfaceNormal = normalize(mat3(worldMatrix) * surfaceNormal);
	surfaceNormal = normalize(normalMatrix * surfaceNormal);

	worldPosition = worldMatrix * posL;

    gl_Position = projectionMatrix * viewMatrix * worldPosition;
  
	// Assign output variables
	fragPos = gl_Position;
	uvCoord = uv;
	toCameraVector = (inverse(viewMatrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz - worldPosition.xyz;
	
	// posLightSpace = lightSpaceMatrix * worldPosition;
	
	if(shaderMode == 1){
		// Colored shader mode
		vColor = color * diffuseColorScale;
	}
	else{
		// Default to white color
		vColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
}