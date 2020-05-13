// Shader for GUIs, only takes in quads (from triangles)
layout(location = 0) in vec3 position;

out vec2 uvCoord;

uniform mat4 worldMatrix; // governs size

void main(void){

	gl_Position = worldMatrix * vec4(position, 1.0);
	
	// Get UV from position on quad
	uvCoord = vec2((position.x + 1.0)/2.0, 1 - (position.y + 1.0)/2.0);
}