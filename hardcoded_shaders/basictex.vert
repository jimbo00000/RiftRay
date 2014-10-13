// basictex.vert
#version 330

in vec3 vPosition;
in vec2 vTexCoord;

out vec2 vfTexCoord;

uniform mat4 mvmtx;
uniform mat4 prmtx;

void main()
{
    vfTexCoord = vTexCoord;
    gl_Position = prmtx * mvmtx * vec4(vPosition, 1.0);
}
