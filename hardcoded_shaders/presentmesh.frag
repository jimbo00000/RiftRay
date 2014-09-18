// presentmesh.frag

uniform float fboScale;
uniform sampler2D fboTex;

varying vec2 vfTexR;
varying vec2 vfTexG;
varying vec2 vfTexB;
varying float vfColor;

vec2 scaleAndFlip(vec2 tc)
{
    return fboScale * vec2(tc.x, 1.0-tc.y);
}

void main()
{
    float resR = texture2D(fboTex, scaleAndFlip(vfTexR)).r;
    float resG = texture2D(fboTex, scaleAndFlip(vfTexG)).g;
    float resB = texture2D(fboTex, scaleAndFlip(vfTexB)).b;
    gl_FragColor = vec4(vfColor * vec3(resR, resG, resB), 1.0);
}
