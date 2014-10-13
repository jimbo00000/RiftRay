// presentmesh.frag
#version 330

uniform float fboScale;
uniform sampler2D fboTex;

in vec2 vfTexR;
in vec2 vfTexG;
in vec2 vfTexB;
in float vfColor;

out vec4 fragColor;

vec2 scaleAndFlip(vec2 tc)
{
    return fboScale * vec2(tc.x, 1.0-tc.y);
}

void main()
{
    float resR = texture(fboTex, scaleAndFlip(vfTexR)).r;
    float resG = texture(fboTex, scaleAndFlip(vfTexG)).g;
    float resB = texture(fboTex, scaleAndFlip(vfTexB)).b;
    fragColor = vec4(vfColor * vec3(resR, resG, resB), 1.0);
}
