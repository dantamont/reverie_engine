// Geometry shader, acts on individual primitives
// See: https://stackoverflow.com/questions/49585794/how-to-pass-information-from-vertex-shader-to-fragment-shader-if-there-is-a-geom

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in GeoData{
	vec3 pos;
	vec4 worldPosition;
	vec4 vColor;
	vec2 uvCoord;
	vec3 surfaceNormal; // applies world matrix
	vec3 toCameraVector;
} geoData[]; // Will recieve an array of these (one for each vertex of the current primitive)

out FragData{
	vec3 pos;
	vec4 worldPosition;
	vec4 vColor;
	vec2 uvCoord;
	vec3 surfaceNormal; // applies world matrix
	vec3 toCameraVector;
}; // Will recieve an array of these (one for each vertex of the current primitive)

// Uniform block for projection and view matrices
layout (std140) uniform CameraBuffer
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	float zNear;
	float zFar;
};

float random (in vec2 _st) {
    return fract(sin(dot(_st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}


void main()
{
  for(int i = 0; i < gl_in.length(); i++)
  {
	// Make a weird vertex
	pos = geoData[i].pos;
	worldPosition = geoData[i].worldPosition;
	worldPosition = worldPosition + 0.1*vec4(random(worldPosition.xy), 0.0, 0.0, 0.0);
	vColor = geoData[i].vColor;
	uvCoord = geoData[i].uvCoord;
	surfaceNormal = geoData[i].surfaceNormal;
	toCameraVector = geoData[i].toCameraVector;
	
    gl_Position = projectionMatrix * viewMatrix * worldPosition;

    // done with the vertex
    EmitVertex();
	
  
     // copy attributes
    gl_Position = gl_in[i].gl_Position;
	
	
	pos = geoData[i].pos;
	worldPosition = geoData[i].worldPosition;
	vColor = geoData[i].vColor;
	uvCoord = geoData[i].uvCoord;
	surfaceNormal = geoData[i].surfaceNormal;
	toCameraVector = geoData[i].toCameraVector;

    // done with the vertex
    EmitVertex();
  }
}