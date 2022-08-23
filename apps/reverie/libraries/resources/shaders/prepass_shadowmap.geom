#define MAX_CUBEMAPS 4
// TODO: Set this dynamically by checking the actual maximum allowed
// value of max_vertices:
// See: https://community.khronos.org/t/confusion-about-maximum-output-from-geometry-shaders/73199
// GLint max_vertices, max_components;
// glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_vertices);
// glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &max_components);
layout (triangles) in; // Take in a triangle
layout (triangle_strip, max_vertices=18 * MAX_CUBEMAPS) out; // Put out six triangles

// Matrices to convert into projected space for each cubemap face
layout(std430, binding = 0) readonly buffer LightMatrices {
	mat4 matrices[];
} lightMatrices;

in vec2 uvCoords[];
out vec2 uvCoordFrag; // UvCoord from GS (output per EmitVertex)
out vec4 fragPos; // FragPos from GS (output per Emitvertex)
out int cubemapLayerFrag;

// The index of the cubemap to use in the cubemap array
// uniform int numCubemapLayers;
uniform int cubemapLayer;

void main()
{

	// Perform layered rendering with the cubemaps
	for(int cubemapLayer = 0; cubemapLayer < numCubemapLayers; ++cubemapLayer){
		for(int face = 0; face < 6; ++face)
		{	
			// See:  https://gamedev.stackexchange.com/questions/109199/how-to-attach-a-framebuffer-to-a-whole-cube-map-from-a-gl-texture-cube-map-array
			int faceLayer = cubemapLayer*6 + face;
			for(int i = 0; i < 3; ++i) // for each triangle vertex
			{   
				// Output variables are invalidated on each EmitVertex call, so reassign each time
				gl_Layer = faceLayer; // built-in variable that specifies to which face we render.
				uvCoordFrag = uvCoords[i];
				cubemapLayerFrag = cubemapLayer;
				fragPos = gl_in[i].gl_Position;
				gl_Position = lightMatrices.matrices[faceLayer] * fragPos;
				EmitVertex();
			}    
			EndPrimitive();
		}
	}
}  