// basicplane.frag
// Apply a simple black and white checkerboard pattern to a quad
// with texture coordinates in the unit interval.
#version 330

in vec2 vfTexCoord;
out vec4 fragColor;

float pi = 3.14159265358979323846;

void main()
{
    float freq = 8.*8.0 * pi;
    float sx = sin(freq*vfTexCoord.x);
    float sy = sin(freq*vfTexCoord.y);
    sx = max(0.,sx-.5);
    sy = max(0.,sy-.5);
    float lum = 1.3*max(sx, sy);
    fragColor = vec4(vec3(lum), 1.0);
}
