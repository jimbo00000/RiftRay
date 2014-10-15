// rwwtt.vert
#version 330
in vec2 vPos;
in vec2 vTex;

out vec2 vfFragCoord;

void main()
{
    vfFragCoord = vTex;
    gl_Position = vec4(vPos, 0.0, 1.0);
}
