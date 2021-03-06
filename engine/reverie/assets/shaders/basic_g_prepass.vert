// Defines:
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

out GeoData{
	vec3 pos;
	vec4 worldPosition;
	vec2 uvCoord;
};

uniform mat4 worldMatrix;

// Uniform block for projection and view matrices
layout (std140) uniform CameraBuffer
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	float zNear;
	float zFar;
};

uniform mat4 boneTransforms[MAX_BONES];
uniform mat4 globalInverseTransform; // TODO:  inverse transform is constant, so use a different buffer for each skeleton
uniform mat4 inverseBindPoseTransforms[MAX_BONES]; // TODO: Inverse bind pose is constant, so use a buffer for each skeleton
uniform bool isAnimated;



void main()
{
	// Assign GL position
	vec4 posL = vec4(position, 1.0);

	if(isAnimated){
		mat4 boneTransform = boneTransforms[boneIDs[0]] * inverseBindPoseTransforms[boneIDs[0]] * boneWeights[0];
		boneTransform     += boneTransforms[boneIDs[1]] * inverseBindPoseTransforms[boneIDs[1]] * boneWeights[1];
		boneTransform     += boneTransforms[boneIDs[2]] * inverseBindPoseTransforms[boneIDs[2]] * boneWeights[2];
		boneTransform     += boneTransforms[boneIDs[3]] * inverseBindPoseTransforms[boneIDs[3]] * boneWeights[3];
		posL = globalInverseTransform * boneTransform * posL;
	}

	worldPosition = worldMatrix * posL;

    gl_Position = projectionMatrix * viewMatrix * worldPosition;
  
	// Assign output variables
	pos = gl_Position.xyz;
	uvCoord = uv;
}