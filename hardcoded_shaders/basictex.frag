// basictex.frag

varying vec2 vfTexCoord;

uniform sampler2D texImage;

void main()
{
    gl_FragColor = texture2D(texImage, vfTexCoord);
}
