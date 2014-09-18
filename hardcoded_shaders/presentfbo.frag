// presentfbo.frag

varying vec2 vfTex;

uniform float fboScale;
uniform sampler2D fboTex;

void main()
{
    gl_FragColor = texture2D(fboTex, vfTex * fboScale);
}