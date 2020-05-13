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

uniform mat4 xConeTransform;
uniform mat4 yConeTransform;
uniform mat4 zConeTransform;
uniform mat4 xCylTransform;
uniform mat4 yCylTransform;
uniform mat4 zCylTransform;

uniform int axis;
uniform bool isCone;

void main(void){
	mat4 transform;
	if(axis == 0){
		if(isCone){
			transform = xConeTransform;
		}else{
			transform = xCylTransform;
		}
	}
	else if(axis == 1){
		if(isCone){
			transform = yConeTransform;
		}else{
			transform = yCylTransform;
		}
	}
	else{
		if(isCone){
			transform = zConeTransform;
		}else{
			transform = zCylTransform;
		}
	}
	
	gl_Position = projectionMatrix * viewMatrix * worldMatrix * transform * vec4(position, 1.0);

}