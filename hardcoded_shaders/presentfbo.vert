// presentfbo.vert
#version 330

in vec2 vPosition;
in vec2 vTex;

out vec2 vfTex;

uniform mat4 mvmtx;
uniform mat4 prmtx;

void main()
{
    vfTex = vTex;
    gl_Position = prmtx * mvmtx * vec4(vPosition, 0.0, 1.0);
}
