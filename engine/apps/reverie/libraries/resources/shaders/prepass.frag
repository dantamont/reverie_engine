#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

in vec2 uvCoord;

// Not needed for depth-write
// out vec4 fColor;

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


// Main ============================================================================
void main()
{
	vec4 textureColor = texture(material.diffuseTexture, uvCoord.st);
	// Discard fragments with low opacity
	if(textureColor.a <= 0.5){
		discard;
	}
	
	// fColor = vec4(1.0, 1.0, 1.0, 1.0);
    
}