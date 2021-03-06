// Shader for use with an FBO to render to an on-screen quad
out vec4 fColor;
  
in vec2 texCoords;


// Default texture unit is 0, otherwise have to set uniform value to texture unit
// Equivalent of getting the uniform location for "screenTexture" and setting its uniform value to "0". 
// See: https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL) , binding points section
layout (binding=0) uniform sampler2D screenTexture;
layout (binding=1) uniform sampler2D depthTexture;
// layout (binding=2) uniform usampler2D stencilTexture;

// See: https://stackoverflow.com/questions/51108596/linearize-depth
// https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer/6657284#6657284
// https://learnopengl.com/Advanced-OpenGL/Depth-testing
float linearDepth(float depthSample, float zNear, float zFar){
	// Map depth back from [0, 1] to [-1, 1]
    float depthRange = 2.0 * depthSample - 1.0;
	
    // Retrieve true depth value (distance from camera) from depth buffer value
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
	return linear;
}


void main()
{ 
	// Render depth buffer
	float depth = texture(depthTexture, texCoords).r;
	depth = linearDepth(depth, 0.1, 1000.0) / 1000;
	fColor = vec4(vec3(depth), 1.0);
}