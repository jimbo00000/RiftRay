// presentmesh.vert
#version 330

uniform vec2 EyeToSourceUVScale;
uniform vec2 EyeToSourceUVOffset;

in vec4 vPosition;
in vec2 vTexR;
in vec2 vTexG;
in vec2 vTexB;

out vec2 vfTexR;
out vec2 vfTexG;
out vec2 vfTexB;
out float vfColor;

void main()
{
    vfTexR = vTexR * EyeToSourceUVScale + EyeToSourceUVOffset;
    vfTexG = vTexG * EyeToSourceUVScale + EyeToSourceUVOffset;
    vfTexB = vTexB * EyeToSourceUVScale + EyeToSourceUVOffset;
    vfColor = vPosition.w;
    gl_Position = vec4(vPosition.xy, 0.5, 1.0);
}
