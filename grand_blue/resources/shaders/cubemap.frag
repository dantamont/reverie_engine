#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

in vec3 normalAndTexCoord;

out vec4 fColor;

uniform vec3 diffuseColor;
uniform mediump samplerCube cubeTexture;

void main()
{
	vec4 textureColor = texture(cubeTexture, normalAndTexCoord);
	fColor = textureColor * vec4(diffuseColor, 1.0);
}