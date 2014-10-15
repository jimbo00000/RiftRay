// shaderpane.frag
#version 330

in vec2 vfTexCoord;
out vec4 fragColor;

uniform sampler2D texImage;
uniform float u_brightness;

void main()
{
    fragColor = u_brightness * texture(texImage, vfTexCoord);
}
