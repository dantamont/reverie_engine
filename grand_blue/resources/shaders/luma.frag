// Shader for use with an FBO to render to an on-screen quad
out vec4 fColor;
  
in vec2 texCoords;

// in vec2 blurTextureCoords[11]; // Texture coordinates of blurred 11-pixel strip

// Default texture unit is 0, otherwise have to set uniform value to texture unit
// Equivalent of getting the uniform location for "screenTexture" and setting its uniform value to "0". 
// See: https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL) , binding points section
layout (binding=0) uniform sampler2D screenTexture;
// layout (binding=1) uniform sampler2D depthTexture;
// layout (binding=2) uniform usampler2D stencilTexture;

void main()
{ 
    vec4 color = texture(screenTexture, texCoords);
	
	// Luma brightness conversion
	float brightness = (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);
	fColor = color * brightness;
}