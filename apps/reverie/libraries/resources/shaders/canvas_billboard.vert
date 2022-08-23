// Shader for GUIs, only takes in quads (from triangles)
layout(location = 0) in vec3 position;
//layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

out vec2 uvCoord;

// TODO: Use UBO
// Struct representing a material
//uniform bool useMaterial;
struct Material{
	float shininess;
    vec3 specularity;
	float pad0; // vec3 are treated as vec4 in glsl, so need implicit padding for buffer
	mediump sampler2D diffuseTexture;
	mediump sampler2D normalMap;
	mediump sampler2D specularTexture;
	mediump sampler2D opacityTexture;
};
uniform Material material;

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

uniform bool scaleWithDistance;
uniform bool faceCamera;
uniform bool onTop;

// Inverts scaling from perspective projection matrix (for billboards)
uniform mat3 perspectiveInverseScale;

void main(void){
	mat4 modelView = viewMatrix * worldMatrix;
	vec4 viewPos = modelView * vec4(position, 1.0);
	mat4 distMat = mat4(1.0);
	if(scaleWithDistance){
		float dist = -viewPos.z;
		distMat = mat4(mat3(dist));
	}
	mat4 scale = mat4(perspectiveInverseScale) * distMat;

	if(faceCamera){
		mat3 rotation = mat3(modelView);
		mat3 inverseScaling;
		inverseScaling[0][0] = length(rotation[0]);
		inverseScaling[1][1] = length(rotation[1]);
		inverseScaling[2][2] = length(rotation[2]); // indexing is column, row, alternatively
		rotation = inverseScaling * rotation;
		mat4 inverseRotation = transpose(mat4(rotation));
		scale = inverseRotation * scale;
	}

	// Since perspective makes things smaller at a distance, need to scale up for const size
	vec4 pos = projectionMatrix * viewMatrix * worldMatrix * scale * vec4(position, 1.0);
//	vec4 pos = projectionMatrix * (viewPos + vec4(screenOffset * dist, 0.0, 0.0));
//	pos /= pos.w; // Always done by OpenGL, as part of vertex post-processing

	if(onTop){
		pos.z = -1.0;
	}

	gl_Position = vec4(pos.x, pos.y, pos.z, pos.w);
	// gl_Position = vec4(pos.x + screenOffset.x, pos.y + screenOffset.y, pos.z, pos.w);
	
	uvCoord = uv;

}