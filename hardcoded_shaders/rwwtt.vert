// rwwtt.vert
#version 330

uniform mat4 paneMatrix;
uniform float u_panePointScale;

const vec3 pts[4] = vec3[4](
    vec3(-1., -1., 0.),
    vec3(1., -1., 0.),
    vec3(1., 1., 0.),
    vec3(-1., 1., 0.)
);

void main()
{
    vec3 pos = pts[gl_VertexID];
    gl_Position = paneMatrix * vec4(u_panePointScale * pos, 1.0);
}
