// Shader for GUIs, only takes in quads (from triangles)
layout(location = 0) in vec3 position;
//layout(location = 1) in vec4 color;
// layout(location = 2) in vec2 uv;

uniform mat4 worldMatrix;

// Uniform block for projection and view matrices
layout (std140) uniform CameraMatrices
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

void main(void){
	gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(position, 1.0);

}