// Shader for use with an FBO to render to an on-screen quad
out vec4 fColor;
  
in vec2 texCoords;

// Default texture unit is 0, otherwise have to set uniform value to texture unit
layout (binding=0) uniform sampler2D screenTexture;
// layout (binding=1) uniform sampler2D depthTexture;
// layout (binding=2) uniform usampler2D stencilTexture;

void main()
{ 
	float contrast = 0.2;
    fColor = texture(screenTexture, texCoords);
	fColor.rgb = (fColor.rgb - 0.5) * (1.0 + contrast) + 0.5;
}