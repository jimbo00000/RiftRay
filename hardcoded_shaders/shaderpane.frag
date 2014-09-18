// shaderpane.frag

varying vec2 vfTexCoord;

uniform sampler2D texImage;
uniform float u_brightness;

void main()
{
    gl_FragColor = u_brightness * texture2D(texImage, vfTexCoord);
}
