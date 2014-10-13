// presentfbo.frag
#version 330

in vec2 vfTex;
out vec4 fragColor;

uniform float fboScale;
uniform sampler2D fboTex;

void main()
{
    fragColor = texture(fboTex, vfTex * fboScale);
}
