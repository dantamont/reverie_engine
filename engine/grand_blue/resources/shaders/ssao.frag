// Uses quad.vert, taking in screen-space quad vertices

#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

in vec2 texCoords; // texture coordinates of screen-space quad

out float fragColor;
// out vec4 fragColor;

layout(binding=0) uniform sampler2D gDepthTexture;
layout(binding=1) uniform sampler2D gNormalTexture;
layout(binding=2) uniform sampler2D noiseTexture;

// tile noise texture over screen based on screen dimensinos divided by noise size
uniform int noiseSize;
uniform int kernelSize;
uniform float radius;
uniform float bias;
// uniform float biasLinear;
// uniform float biasSquared;

// Randomly sampled hemisphere
layout(std430, binding = 0) readonly buffer KernelBuffer {
	vec4 samples[];
} kernel;

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

// Function Prototypes ============================================
vec3 viewPosFromDepth(float depth);
vec3 worldPosFromDepth(float depth);
float linearDepth(float depthSample, float zNear, float zFar);

// Main ============================================================================
void main()
{
	// Get input for SSAO algorithm
	vec2 noiseScale = vec2(float(viewportDimensions.x)/float(noiseSize),
		float(viewportDimensions.y)/float(noiseSize));
		
	float depth = texture2D(gDepthTexture, texCoords).r;
	vec3 fragPos = viewPosFromDepth(depth);
	vec3 normal = texture2D(gNormalTexture, texCoords).xyz;
	vec3 randomVec = normalize(texture2D(noiseTexture, texCoords * noiseScale).xyz);   // requires repeat
	
	// Create TBN matrix to convert from tangent space to view space
	// Using a process called the Gramm-Schmidt process we create an orthogonal 
	// basis, each time slightly tilted based on the value of randomVec. 
	
	// Note that because we use a random vector for constructing the tangent vector,
	// there is no need to have the TBN matrix exactly aligned to the geometry's 
	// surface, thus no need for per-vertex tangent (and bitangent) vectors.
	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 
	
	//  Iterate over each of the kernel samples, transform the samples from tangent to view-space, add them to the current fragment position, and compare the fragment position's depth with the sample depth stored in the view-space position buffer
	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		// Get sample position
		vec3 samplePos = TBN * kernel.samples[i].xyz; // from tangent to view-space
		samplePos = fragPos + samplePos * radius; 
		
		// Project sample position (to sample texture) (to get position on screen/texture)
		vec4 offset = vec4(samplePos, 1.0);
		offset      = projectionMatrix * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  
		
		// Get sample depth (z-value from viewer's perspective)
		float sampleBufferDepth = texture(gDepthTexture, offset.xy).r; 
		float sampleDepth = viewPosFromDepth(sampleBufferDepth).z;
		
		// Obtain occlusion
		// Bias helps tweak the ffect and solves acne effects that may occur
		// Range check helps ensure that only objects near one another cause occlusion
		float linearDepth = linearDepth(depth, zNear, zFar);
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		// float scaledBias = bias + biasLinear * linearDepth + biasSquared * linearDepth * linearDepth;
		occlusion += float(sampleDepth >= samplePos.z * (1.0 + bias)) * rangeCheck;  
	} 
	
	// Normalize occlusion by kernel, subtracting from one to use directly for scaling ambient light
	occlusion = 1.0 - (occlusion / kernelSize);
	fragColor = occlusion;
	// fragColor = (randomVec.x + 1.0)/2.0;
	
	vec3 noiseTextureSample = normalize(texture2D(noiseTexture, texCoords).xyz);
	// fragColor = (noiseTextureSample.x + 1.0)/2.0;
	// fragColor = linearDepth(depth, 0.1, 1000.0)/1000.0;
	// fragColor = (normal.y + 1.0)/2.0;
	// fragColor = vec4(0.5, 0.8, 1.0, 1.0);
}

float linearDepth(float depthSample, float zNear, float zFar){
	// Map depth back from [0, 1] to [-1, 1]
    float depthRange = 2.0 * depthSample - 1.0;
	
    // Retrieve true depth value (distance from camera) from depth buffer value
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
	return linear;
}


// Get the view position from the depth buffer
// See: https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
vec3 viewPosFromDepth(float depth) {
	// Convert from [0, 1] to [-1, 1]
    float z = depth * 2.0 - 1.0;
	
	// Get clip and view position from uv coords of screen quad
    vec4 clipSpacePosition = vec4(texCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invProjectionMatrix * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

// Get the world position from the depth buffer
// See: https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value
vec3 worldPosFromDepth(float depth) {
	// Convert from [0, 1] to [-1, 1]
    float z = depth * 2.0 - 1.0;
	
	// Get clip and view position from uv coords of screen quad
    vec4 clipSpacePosition = vec4(texCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invProjectionMatrix * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = invViewMatrix * viewSpacePosition;

    return worldSpacePosition.xyz;
}