#ifdef GL_ES 

// Set default precision to medium 
precision mediump int; 
precision mediump float; 

#endif 

out vec4 fColor;

// Main ============================================================================
void main()
{
	// vec4 textureColor = texture(material.diffuseTexture, uvCoord.st);
	// Discard fragments with low opacity
	// if(textureColor.a <= 0.2){
		// discard;
	// }
	
	fColor = vec4(1.0, 1.0, 1.0, 1.0);
    
}