// basictex.vert

attribute vec4 vPosition;
attribute vec2 vTex;

varying vec2 vfTex;

uniform mat4 mvmtx;
uniform mat4 prmtx;

void main()
{
    vfTex = vTex;
    gl_Position = prmtx * mvmtx * vPosition;
}
