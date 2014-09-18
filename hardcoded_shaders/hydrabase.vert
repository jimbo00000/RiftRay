// hydrabase.vert

attribute vec4 vPosition;
attribute vec4 vColor;

varying vec3 vfColor;

uniform mat4 mvmtx;
uniform mat4 prmtx;

void main()
{
    float radius = 0.0254;
    vfColor = vColor.xyz;
    gl_Position = prmtx * mvmtx * vec4(radius * normalize(vPosition.xyz), 1.0);
}
