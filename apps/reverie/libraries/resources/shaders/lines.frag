// Shader for lines
out vec4 fColor;

uniform vec4 lineColor;
uniform bool fadeWithDistance;

in vec4 worldPos;
in vec4 camPos;
// in float uniformScale;

float log10(in float value){
	return log(value)/log(10.0);
}

void main(void){
	if(fadeWithDistance){
		float distance = abs(length(worldPos.xyz - camPos.xyz));
		float nearestTen = pow(10.0, ceil(log10(length(camPos))));
		fColor = lineColor;
		// fColor.a *= (nearestTen - distance)/nearestTen;
		fColor.a *= nearestTen / distance;
		fColor.a = min(fColor.a, lineColor.a);
	}
	else{
		fColor = lineColor;
	}
}