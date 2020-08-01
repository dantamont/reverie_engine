// Shader for use with an FBO to render to an on-screen quad
out vec4 fColor;
  
in vec2 texCoords;

uniform sampler2D screenTexture;

void main()
{ 
    fColor = texture(screenTexture, texCoords);
}