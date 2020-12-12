layout (triangles) in; // Take in a triangle
layout (triangle_strip, max_vertices=18) out; // Put out six triangles

// Matrices to convert into projected space for each cubemap face
layout(std430, binding = 0) readonly buffer LightMatrices {
	mat4 matrices[];
} lightMatrices;

in vec2 uvCoords[];
out vec2 uvCoordFrag; // UvCoord from GS (output per EmitVertex)
out vec4 fragPos; // FragPos from GS (output per Emitvertex)

// The index of the cubemap to use in the cubemap array
// uniform int numCubemapLayers;
uniform int cubemapLayer;

void main()
{


	// Perform layered rendering with the cubemaps
	// for(int cubemapLayer = 0; cubemapLayer < numCubemapLayers; ++cubemapLayer){
		for(int face = 0; face < 6; ++face)
		{	
			// See:  https://gamedev.stackexchange.com/questions/109199/how-to-attach-a-framebuffer-to-a-whole-cube-map-from-a-gl-texture-cube-map-array
			for(int i = 0; i < 3; ++i) // for each triangle vertex
			{   
				int faceLayer = cubemapLayer*6 + face;
				// faceLayer = 6 + face ;
				// Output variables are invalidated on each EmitVertex call, so reassign each time
				gl_Layer = faceLayer; // built-in variable that specifies to which face we render.
				uvCoordFrag = uvCoords[i];
				fragPos = gl_in[i].gl_Position;
				gl_Position = lightMatrices.matrices[faceLayer] * fragPos;
				EmitVertex();
			}    
			EndPrimitive();
		}
	// }
}  