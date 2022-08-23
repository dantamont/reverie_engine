// Shader for GUIs, only takes in quads (from triangles)

in vec2 uvCoord;

// out vec4 fColor;
layout (location=0) out vec4 fColor;
// location=1 is normals
layout (location=2) out vec4 fUniqueId; // elements from 0-255

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

uniform vec2 texOffset; // For sprite-sheet sampling
uniform vec2 texScale; // For sprite-sheet sampling

uniform vec3 textColor;
// uniform sampler2D guiTexture;
uniform vec4 g_colorId; // The ID of the draw command for this object, as a vector

void main(void){
	vec2 texCoord = texOffset + uvCoord * texScale; 
	vec4 textureColor = texture(material.diffuseTexture, texCoord);
	float opacity = min(texture(material.opacityTexture, texCoord).r, textureColor.a);
	// vec4 textureColor = textureSize(guiTexture, 0).x == 1 ? texture(material.diffuseTexture, texCoord) : texture(guiTexture, texCoord);
	
	// Discard fragments with low opacity
	// if(textureColor.a <= 0.2){ 
	if(opacity <= 0.01){ 
		discard;
	}
	
	// If alpha channel isn't set to 1.0, blending issues happen
	fColor = vec4(textColor * textureColor.xyz, opacity);
	
	// Transparency was causing problems during blend, so opaque for now
	fUniqueId = vec4(g_colorId[0] / 255.0f, g_colorId[1] / 255.0f, g_colorId[2] / 255.0f, 1.0f); 
}