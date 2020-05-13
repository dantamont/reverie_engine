// Shader for GUIs, only takes in quads (from triangles)

in vec2 uvCoord;

out vec4 fColor;

uniform sampler2D guiTexture;

void main(void){

	fColor = texture(guiTexture, uvCoord);

}