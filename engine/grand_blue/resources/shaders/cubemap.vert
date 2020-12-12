// See: https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
// The use of layout forgos the use of glBindAttribLocation entirely. 
// If you try to combine the two and they conflict, the layout qualifier always wins.
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uvCoord;
layout(location = 3) in vec3 normal;

out vec3 normalAndTexCoord;

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

void main()
{
	// Assign GL position
	vec4 worldPosition = worldMatrix * vec4(position, 1.0);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
  
	// Assign output variables
	normalAndTexCoord = normalize(position);
}