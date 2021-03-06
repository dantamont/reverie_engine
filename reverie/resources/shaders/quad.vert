// Shader for use with an FBO to render to an on-screen quad

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uvCoord;
out vec2 texCoords;

uniform vec3 offsets; // Specified from center of screen
uniform vec2 scale;

void main()
{
	float depth = offsets.z;
    gl_Position = vec4(position.x * scale.x + (offsets.x), position.y * scale.y + (offsets.y), depth, 1.0f); 
    texCoords = uvCoord;
}  