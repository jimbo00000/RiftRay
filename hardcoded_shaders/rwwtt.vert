// rwwtt.vert
#version 330
in vec3 vPos;
in vec2 vTex;

out vec2 vfFragCoord;

uniform mat4 paneMatrix;

void main()
{
    vfFragCoord = vTex;
    gl_Position = paneMatrix * vec4(vPos, 1.0);
}
