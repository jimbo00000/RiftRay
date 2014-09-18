// basicplane.frag
// Apply a simple black and white checkerboard pattern to a quad
// with texture coordinates in the unit interval.

varying vec2 vfTexCoord;

float pi = 3.14159265358979323846;

void main()
{
    float freq = 16.0 * pi;
    float lum = floor(1.0 + 2.0*sin(freq*vfTexCoord.x) * sin(freq*vfTexCoord.y));
    gl_FragColor = vec4(vec3(lum), 1.0);
}
