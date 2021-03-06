// Shader for GUIs, only takes in quads (from triangles)

in vec2 uvCoord;

// out vec4 fColor;
layout (location=0) out vec4 fColor;
// location=1 is normals
layout (location=2) out vec4 fUniqueId; // elements from 0-255

uniform vec3 textColor;
uniform sampler2D guiTexture;
uniform vec4 g_colorId; // The ID of the draw command for this object, as a vector

void main(void){
	// Discard fragments with low opacity
	vec4 textureColor = texture(guiTexture, uvCoord);
	
	if(textureColor.r <= 0.5){ 
		discard;
	}
	
	fColor = vec4(textColor, textureColor.r);
	
	// Transparency was causing problems during blend, so opaque for now
	fUniqueId = vec4(g_colorId[0] / 255.0f, g_colorId[1] / 255.0f, g_colorId[2] / 255.0f, 1.0f); 
}