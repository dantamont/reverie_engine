// Shader for use with an FBO to render to an on-screen quad
out vec4 fColor;
  
in vec2 texCoords;

// in vec2 blurTextureCoords[11]; // Texture coordinates of blurred 11-pixel strip

// Default texture unit is 0, otherwise have to set uniform value to texture unit
// Equivalent of getting the uniform location for "screenTexture" and setting its uniform value to "0". 
// See: https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL) , binding points section
layout (binding=0) uniform sampler2D screenTexture;
// layout (binding=1) uniform sampler2D depthTexture;
// layout (binding=2) uniform usampler2D stencilTexture;

uniform bool horizontal;

void main()
{ 
    vec4 color = texture(screenTexture, texCoords);
	
	// Blur
	float weights[11] = float[](0.0093, 0.028002, 0.065984, 0.121703, 0.175713,
		0.198596, 0.175713, 0.121703, 0.065984, 0.028002, 0.0093);
		
	// Fill out texture coord array
	vec2 blurTextureCoords[11];
	if(horizontal){
		float pixelWidth = 1.0 / textureSize(screenTexture, 0).x;
		for(int i=-5; i<=5; i++){
			blurTextureCoords[i+5] = texCoords + vec2(pixelWidth * float(i), 0.0);
		}
	}else{
		float pixelHeight = 1.0 / textureSize(screenTexture, 0).y;
		for(int i=-5; i<=5; i++){
			blurTextureCoords[i+5] = texCoords + vec2(0.0, pixelHeight * float(i));
		}
	}
	
	fColor = vec4(0.0);
	for(int i = 0; i < 11; i++){
		fColor += clamp(texture(screenTexture, blurTextureCoords[i]), 0.0, 1.0) * weights[i];
	}
	fColor.a = color.a;
}