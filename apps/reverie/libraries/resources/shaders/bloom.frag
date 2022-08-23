// Shader for use with an FBO to render to an on-screen quad
out vec4 fColor;
  
in vec2 texCoords;

// in vec2 blurTextureCoords[11]; // Texture coordinates of blurred 11-pixel strip

// Default texture unit is 0, otherwise have to set uniform value to texture unit
// Equivalent of getting the uniform location for "screenTexture" and setting its uniform value to "0". 
// See: https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL) , binding points section
// TODO: Uniform names are hard-coded, fix this when parsing
layout (binding=0) uniform sampler2D screenTexture;
// layout (binding=1) uniform sampler2D depthTexture;
// layout (binding=2) uniform usampler2D stencilTexture;
layout (binding=3) uniform sampler2D sceneTexture; // texture for scene before this effect
layout (binding=4) uniform sampler2D checkpointTexture; // texture for scene before this effect

uniform float bloomIntensity;

void main()
{ 
    vec4 color = texture(screenTexture, texCoords);
	
	// Bloom
	// vec4 sceneColor = texture(sceneTexture, texCoords);
	vec4 sceneColor = texture(checkpointTexture, texCoords);
	fColor = sceneColor + color * bloomIntensity;
}