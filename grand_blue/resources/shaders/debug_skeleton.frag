// Shader for point color shading
#define MAX_BONES 100

uniform vec4 color;
uniform vec4 colors[MAX_BONES];
out vec4 fColor;
flat in int boneID;

void main(void){

    fColor = colors[boneID];
    float r = 0.0, delta = 0.0;
    vec2 cxy = 2.0 * gl_PointCoord - 1.0; // gl_PointCoord is a fragment language input variable that contains the two-dimensional coordinates indicating where within a point primitive the current fragment is located
    r = dot(cxy, cxy);
#ifdef GL_OES_standard_derivatives
    delta = fwidth(r); //  returns the maximum change in a given fragment shader variable in the neighbourhood of the current pixel (8 surrounding pixels)
    fColor.a = max(0, fColor.a - smoothstep(1.0 - delta, 1.0 + delta, r));
#endif

}