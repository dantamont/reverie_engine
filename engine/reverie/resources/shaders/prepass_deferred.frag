#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

in vec2 uvCoord;
in vec3 viewNormal;
// in vec3 fragPos;

// layout (location = 0) out vec3 gPosition; // Isn't actually set
layout (location = 1) out vec3 gNormal; // Write view-space normal to second color attachment

// Struct representing a material
//uniform bool useMaterial;
struct Material{
	float shininess;
    vec3 specularity;
	float pad0; // vec3 are treated as vec4 in glsl, so need implicit padding for buffer
	mediump sampler2D diffuseTexture;
	mediump sampler2D normalMap;
	mediump sampler2D specularTexture;
};
uniform Material material;


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


// Main ============================================================================
void main()
{

	vec4 textureColor = texture(material.diffuseTexture, uvCoord.st);
	// Discard fragments with low opacity
	if(textureColor.a <= 0.3){
		discard;
	}

	// gPosition = vec3(0.0, 0.0, 0.0);

	// Could do depth only, but would cause artifacts
	//https://stackoverflow.com/questions/37627254/how-to-reconstruct-normal-from-depth-without-artifacts-on-edge
	// https://gamedev.stackexchange.com/questions/162248/correctly-transforming-normals-for-g-buffer-in-deferred-rendering
	// https://www.gamedev.net/tutorials/_/technical/graphics-programming-and-theory/a-simple-and-practical-approach-to-ssao-r2753/
	// Note, need floating point FBO to support ranges outside of [0, 1]
	// gNormal = normalize(viewNormal);
	gNormal = viewNormal;
}