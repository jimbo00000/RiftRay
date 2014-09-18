// presentmesh.vert

uniform vec2 EyeToSourceUVScale;
uniform vec2 EyeToSourceUVOffset;

attribute vec4 vPosition;
attribute vec2 vTexR;
attribute vec2 vTexG;
attribute vec2 vTexB;

varying vec2 vfTexR;
varying vec2 vfTexG;
varying vec2 vfTexB;
varying float vfColor;

void main()
{
    vfTexR = vTexR * EyeToSourceUVScale + EyeToSourceUVOffset;
    vfTexG = vTexG * EyeToSourceUVScale + EyeToSourceUVOffset;
    vfTexB = vTexB * EyeToSourceUVScale + EyeToSourceUVOffset;
    vfColor = vPosition.w;
    gl_Position = vec4(vPosition.xy, 0.5, 1.0);
}
