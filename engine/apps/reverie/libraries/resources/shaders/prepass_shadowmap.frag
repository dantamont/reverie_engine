#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

in vec2 uvCoordFrag;
in vec4 fragPos;
// flat in int cubemapLayerFrag; // No interpolation with 'flat'
// Not needed for depth write
// out vec4 fColor;

// Attributes of each Point light
layout(std430, binding = 1) readonly buffer PointLightAttributes {
	vec4 farPlane_cameraPos[];// Sphere camera range (float) and Sphere camera origin
} pointLightAttributes;

// Struct representing a material
//uniform bool useMaterial;
struct Material{
	float shininess;
    vec3 specularity;
	float pad0; // vec3 are treated as vec4 in glsl, so need implicit padding for buffer
	mediump sampler2D diffuseTexture;
	mediump sampler2D normalMap;
};
uniform Material material;

/// The index of the point light cubemap being used
uniform int pointLightIndex;

// Functions ============================================================================

float linearDepth(float depthSample, float zNear, float zFar){
	// Map depth back from [0, 1] to [-1, 1]
    float depthRange = 2.0 * depthSample - 1.0;
	
    // Retrieve true depth value (distance from camera) from depth buffer value
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
	return linear;
}

float depthSample(float linearDepth, float zNear, float zFar)
{
	// Normalize depth from true value to [-1, 1] range
    float nonLinearDepth = (zFar + zNear - 2.0 * zNear * zFar / linearDepth) / (zFar - zNear);
	
	// Map from [-1, 1] to [0, 1]
    nonLinearDepth = (nonLinearDepth + 1.0) / 2.0;
	
    return nonLinearDepth;
}

// Main ============================================================================
void main()
{
	vec4 textureColor = texture(material.diffuseTexture, uvCoordFrag.st);
	// Discard fragments with low opacity
	if(textureColor.a <= 0.5){
		discard;
	}
	
	// get distance between fragment and light source
	vec4 farPlane_pos = pointLightAttributes.farPlane_cameraPos[pointLightIndex];
	float farPlane = farPlane_pos.x;
	vec3 sphereCameraPos = farPlane_pos.yzw;
    float lightDistance = length(fragPos.xyz - sphereCameraPos);
    
    // map to [0;1] range by using depth sample
	// Sample is unnecessary, just need to make sure that far plane is large enough to accommodate all geometry in texture (bounded from [0, 1])
    // lightDistance = depthSample(lightDistance, nearPlane, farPlane);
	lightDistance = lightDistance / farPlane;

    // write this as modified depth
    gl_FragDepth = lightDistance;
	
	// fColor = vec4(1.0, 1.0, 1.0, 1.0);
    
}