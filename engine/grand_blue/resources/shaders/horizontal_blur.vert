// Shader for use with an FBO to render to an on-screen quad

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uvCoord;
out vec2 texCoords;

uniform vec3 offsets; // Specified from center of screen
uniform vec2 scale;
// uniform uvec2 screenDimensions; // Easier than plugging in a buffer

out vec2 blurTextureCoords[11]; // Texture coordinates of blurred 11-pixel strip

// SSBO with inverse projection matrix, tile sizes, and screen dimensions
layout (std430, binding = 2) buffer ScreenToViewBuffer{
    mat4 inverseProjection;
    uvec4 tileGridSizes;
	uvec4 tilePixelSizes;
    uvec2 screenDimensions;
};

void main()
{
	// Horizontal blur
	// vec2 centerTexCoords = vec2(gl_Position.x, gl_Position.y)*0.5 + 0.5; // Coordinates of center of texture
	float pixelSize = 1.0 / float(screenDimensions.x);

	// Fill out texture coord array
	for(int i=-5; i<=5; i++){
		blurTextureCoords[i+5] = texCoords + vec2(pixelSize * float(i), 0.0);
	}
}  