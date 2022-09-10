// Shader for lines, rendered using triangles
// See: https://mattdesl.svbtle.com/drawing-lines-is-hard
// https://forum.libcinder.org/topic/smooth-thick-lines-using-geometry-shader
layout(location = 0) in vec3 position;
layout(location = 3) in vec3 prevPosition;
layout(location = 4) in vec3 nextPosition;
layout(location = 5) in ivec4 pairDirAttr; // miscInt attr
// layout(location = 6) in vec4 distanceAlongLine_ // miscReal attr

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

uniform float thickness;
uniform bool useMiter;
uniform bool constantScreenThickness;
const float PI = 3.1415926535897932384626433832795;

out vec4 worldPos;
out vec4 camPos;
out float uniformScale;

// get 2D screen space with W divide and aspect correction
vec2 fix(vec4 projectedVec, float aspectRatio){
	vec2 curScreen = projectedVec.xy / projectedVec.w;
	curScreen.x *= aspectRatio;
	return curScreen;
}

void main() {	
	int pairDir = pairDirAttr[0];
	// float distanceAlongLine = distanceAlongLine_[0];
	worldPos = worldMatrix * vec4(position, 1.0);
    worldPos = worldPos / worldPos.w; // The only difference between frustum lines and lines shader
	camPos = viewMatrix[3];

	// Correct for world-space scaling
	mat4 modelView = viewMatrix * worldMatrix;

	// Get projected positions
	// mat4 vpm = projectionMatrix * modelView * inverseScale;
	mat4 vpm = projectionMatrix * modelView;
	vec4 prevProjectedPosition = vpm * vec4(prevPosition, 1.0);
	vec4 projectedPosition = vpm * vec4(position, 1.0);
	vec4 nextProjectedPosition = vpm * vec4(nextPosition, 1.0);
	
	// This is NDC space [-1 ... 1]
	float aspect = projectionMatrix[1][1] / projectionMatrix[0][0];
	float inverseAspect = 1.0 / aspect;
	vec2 currentScreen = fix(projectedPosition, aspect);
	vec2 nextScreen = fix(nextProjectedPosition, aspect);
	vec2 prevScreen = fix(prevProjectedPosition, aspect);

	
	// float uniformScale = scaling[0][0];
	// uniformScale = 1.0;
	
	// Set width
	// float width = thickness / uniformScale;
	float width = thickness;
	if(constantScreenThickness){
		width *= projectedPosition.w;
	}

	// Get direction of the current line segment in screen space
	vec2 screenDir = vec2(0.0);
	if (currentScreen == prevScreen) {
		// Start point
		screenDir = normalize(nextScreen - currentScreen);
	} 
	else if (currentScreen == nextScreen) {
		// End point
		screenDir = normalize(currentScreen - prevScreen);
	}
	else {
		// Any other point
		
		// Get directions from (C - B) and (B - A)
		vec2 dirA = normalize((currentScreen - prevScreen));
		if (useMiter) {
		  vec2 dirB = normalize((nextScreen - currentScreen));
		  float angle = acos(dot(dirA, dirB)); 
		  if(angle > 1.4*PI/2.0){
			  // don't use miter if angle is too sharp
			  screenDir = dirA;
		  }
		  else{
			  //now compute the miter join normal and length
			  vec2 tangent = normalize(dirA + dirB);
			  vec2 perp = vec2(-dirA.y, dirA.x);
			  vec2 miter = vec2(-tangent.y, tangent.x);
			  screenDir = tangent;
			  width = thickness / dot(miter, perp);
		  }
		} 
		else {
		  screenDir = dirA;
		}
	}
	
	// Get normal vector (in direction of line thickness) for thickness offset
	vec2 normal = vec2(-screenDir.y, screenDir.x);
	normal *= width/2.0;
	normal.x /= aspect;
	
	// Get the screen-space offset to render line
	vec4 offset = vec4(normal * pairDir, 0.0, 1.0);
	
	// Set vertex position
	projectedPosition.xy += offset.xy;
	gl_Position = projectedPosition;
}


