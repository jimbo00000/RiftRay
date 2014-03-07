// rwwtt.vert

attribute vec4 vPosition;
attribute vec2 vTex;

varying vec2 vfFragCoord;

void main()
{
    vfFragCoord = vTex;
    gl_Position = vPosition;
}
