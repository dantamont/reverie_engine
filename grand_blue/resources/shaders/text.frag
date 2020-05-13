// Shader for GUIs, only takes in quads (from triangles)

in vec2 uvCoord;

out vec4 fColor;

uniform vec3 textColor;
uniform sampler2D guiTexture;

void main(void){

	fColor = vec4(textColor, texture(guiTexture, uvCoord).r);

}