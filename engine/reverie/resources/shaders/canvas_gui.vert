// Shader for GUIs, only takes in quads (from triangles)
layout(location = 0) in vec3 position;
//layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

out vec2 uvCoord;

//uniform vec2 glyphSize;
//uniform vec2 glyphOrigin;
// uniform vec2 screenOffset;
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

uniform bool onTop;

void main(void){
	mat4 modelView = viewMatrix * worldMatrix;
	vec4 viewPos = modelView * vec4(position, 1.0);
	// mat4 distMat = mat4(1.0);
	// if(scaleWithDistance){
		// float dist = -viewPos.z;
		// distMat = mat4(mat3(dist));
	// }
	// mat4 scale = mat4(perspectiveInverseScale) * distMat;
	mat4 scale = mat4(1.0);


	// Since perspective makes things smaller at a distance, need to scale up for const size
	vec4 pos = projectionMatrix * viewMatrix * worldMatrix * scale * vec4(position, 1.0);

	if(onTop){
		pos.z = -1.0;
	}

	gl_Position = vec4(pos.x, pos.y, pos.z, pos.w);
	// gl_Position = vec4(pos.x + screenOffset.x, pos.y + screenOffset.y, pos.z, pos.w);
	
	uvCoord = uv;

}