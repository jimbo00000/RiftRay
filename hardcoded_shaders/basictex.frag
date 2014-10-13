// basictex.frag
#version 330

in vec2 vfTexCoord;
out vec4 fragColor;

uniform sampler2D texImage;

void main()
{
    fragColor = texture2D(texImage, vfTexCoord);
}
