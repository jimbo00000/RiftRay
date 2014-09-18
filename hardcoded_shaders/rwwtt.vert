// rwwtt.vert

attribute vec2 vPos;
attribute vec2 vTex;

varying vec2 vfFragCoord;

void main()
{
    vfFragCoord = vTex;
    gl_Position = vec4(vPos, 0.0, 1.0);
}
