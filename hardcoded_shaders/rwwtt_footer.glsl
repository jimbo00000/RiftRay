// rwwtt_footer.glsl
// This plugs into the shader body via the function
//     vec3 getSceneColor( in vec3 ro, in vec3 rd )
// This can be thought of as an API - the purpose is to allow the uniforms
// passed in from the Rift SDK to change viewpoint and direction.

in vec2 vfUV;

///////////////////////////////////////////////////////////////////////////////
// Patch in the Rift's heading to raymarch shader writing out color and depth.
// http://blog.hvidtfeldts.net/

// Translate the origin to the camera's location in world space.
vec3 getEyePoint(mat4 mvmtx)
{
    vec3 ro = -mvmtx[3].xyz;
    return ro;
}

// Construct the usual eye ray frustum oriented down the negative z axis.
// http://antongerdelan.net/opengl/raycasting.html
vec3 getRayDirection(vec2 uv)
{
    vec4 ray_clip = vec4(uv.x, uv.y, -1., 1.);
    vec4 ray_eye = inverse(prmtx) * ray_clip;
    return normalize(vec3(ray_eye.x, ray_eye.y, -1.));
}

void main()
{
    vec2 uv = vfUV;
    vec3 ro = getEyePoint(mvmtx);
    vec3 rd = getRayDirection(uv);

    ro *= mat3(mvmtx);
    rd *= mat3(mvmtx);

    vec3 col = getSceneColor(ro, rd);
    fragColor = vec4(col, 1.0);
}