// Defines:
#define MAX_BONES 100
layout(location = 0) in vec3 position;
// layout(location = 1) in vec4 color;
// layout(location = 2) in vec2 uv;
// layout(location = 3) in vec3 normal;
// layout(location = 4) in vec3 tangent;
layout(location = 5) in ivec4 boneIDs; // Bone ID is first entry of vec4

// Could use an interface block, instead of having to pull in all the uniforms individually
// out NAME{
// vec3 c;
// } outName;

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
uniform bool isAnimated;
uniform float pointSize;
uniform float screenPixelWidth;

flat out int boneID;

void main()
{
	// Assign GL position
	vec4 posL = vec4(position, 1.0);
	
	boneID = boneIDs[0];
	if(isAnimated){
		mat4 boneTransform = boneTransforms[boneID];
		posL = boneTransform * posL;
	}

	vec4 worldPosition = worldMatrix * posL;

    gl_Position = projectionMatrix * viewMatrix * worldPosition;
	gl_PointSize = pointSize * screenPixelWidth;
}