// basictex.frag
#version 330

in vec2 vfTexCoord;
out vec4 fragColor;

uniform sampler2D texImage;

void main()
{
    float texLum = texture(texImage, vfTexCoord).r;
    if (texLum < 0.5)
        discard;
    fragColor = vec4(vec3(texLum),1.0);
}
