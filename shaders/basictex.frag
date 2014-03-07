// basictex.frag

varying vec2 vfTex;

uniform sampler2D iChannel0;

void main()
{
    gl_FragColor = texture2D(iChannel0, vfTex);
}
