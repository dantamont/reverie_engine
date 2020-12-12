// Shader for GUIs, only takes in quads (from triangles)

in vec2 uvCoord;

out vec4 fColor;

uniform vec3 textColor;
uniform sampler2D guiTexture;

void main(void){
	// Discard fragments with low opacity
	vec4 textureColor = texture(guiTexture, uvCoord);
	
	if(textureColor.r <= 0.5){ 
		discard;
	}
	
	fColor = vec4(textColor, textureColor.r);

}