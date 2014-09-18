// basictex.vert

attribute vec3 vPosition;
attribute vec2 vTexCoord;

varying vec2 vfTexCoord;

uniform mat4 mvmtx;
uniform mat4 prmtx;

void main()
{
    vfTexCoord = vTexCoord;
    gl_Position = prmtx * mvmtx * vec4(vPosition, 1.0);
}
