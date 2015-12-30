// rwwtt.vert
#version 330

out vec2 vfUV;

uniform mat4 paneMatrix;
uniform float u_panePointScale;
uniform float u_fboScale;

float y_ = 1. - 2./u_fboScale;
vec3 pts[4] = vec3[4](
    vec3(-1., y_, 0.),
    vec3( 1., y_, 0.),
    vec3( 1., 1., 0.),
    vec3(-1., 1., 0.)
);

void main()
{
    vec3 pos = pts[gl_VertexID];
    vfUV = pos.xy;
    gl_Position = paneMatrix * vec4(u_panePointScale * pos, 1.0);
}
