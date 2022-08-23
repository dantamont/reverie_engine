// Shader for performing a prepass that populates a cubemap
// Used for point light shadow mapping via SphereCamera

// Defines:
// #define MAX_LIGHTS 20
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

// Matrices to convert into projected space for each cubemap face
layout(std430, binding = 0) readonly buffer LightMatrices {
	mat4 matrices[];
} lightMatrices;


// out	vec2 uvCoords;
out vec2 uvCoordFrag; // UvCoord from GS (output per EmitVertex)
out vec4 fragPos; // FragPos from GS (output per Emitvertex)
// out int cubemapLayerFrag;

uniform mat4 worldMatrix;

uniform mat4 boneTransforms[MAX_BONES];
uniform mat4 globalInverseTransform;
uniform mat4 inverseBindPoseTransforms[MAX_BONES];
uniform bool isAnimated;

/// The index of the point light cubemap being used
uniform int pointLightIndex;

void main()
{
	// Set layer based on instance rendered
	// This decides which cubemap face is rendered to
	// Requires AMD_vertex_shader_layer: https://www.khronos.org/registry/OpenGL/extensions/AMD/AMD_vertex_shader_layer.txt
	int face = gl_InstanceID;
	int lightMatrixIndex = pointLightIndex*6 + face;
	gl_Layer = lightMatrixIndex;

	// Assign GL position
	vec4 posL = vec4(position, 1.0);

	if(isAnimated){
		mat4 boneTransform = boneTransforms[boneIDs[0]] * inverseBindPoseTransforms[boneIDs[0]] * boneWeights[0];
		boneTransform     += boneTransforms[boneIDs[1]] * inverseBindPoseTransforms[boneIDs[1]] * boneWeights[1];
		boneTransform     += boneTransforms[boneIDs[2]] * inverseBindPoseTransforms[boneIDs[2]] * boneWeights[2];
		boneTransform     += boneTransforms[boneIDs[3]] * inverseBindPoseTransforms[boneIDs[3]] * boneWeights[3];
		posL = globalInverseTransform * boneTransform * posL;
	}
	
	vec4 worldPosition = worldMatrix * posL;
	
	uvCoordFrag = uv;

	fragPos = worldPosition;
    gl_Position = lightMatrices.matrices[lightMatrixIndex] * worldPosition;
}