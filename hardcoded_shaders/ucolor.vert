// ucolor.vert

attribute vec4 vPosition;

uniform mat4 mvmtx;
uniform mat4 prmtx;

void main()
{
    gl_Position = prmtx * mvmtx * vPosition;
}
