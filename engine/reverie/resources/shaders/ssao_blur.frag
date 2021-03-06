// Uses quad.vert, taking in screen-space quad vertices

#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

// Write out to second FBO attachment
out float fragColor;

in vec2 texCoords;

layout(binding = 3) uniform sampler2D ssaoInput;

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture2D(ssaoInput, texCoords + offset).r;
        }
    }
	
    fragColor = result / (4.0 * 4.0);
	// fragColor = texture2D(ssaoInput, texCoords).r;
}  