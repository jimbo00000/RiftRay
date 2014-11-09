// rwwtt_footer.glsl
// This plugs into the shader body via the function
//     vec3 getSceneColor( in vec3 ro, in vec3 rd )
// This can be thought of as an API - the purpose is to allow the uniforms
// passed in from the Rift SDK to change viewpoint and direction.

// http://blog.hvidtfeldts.net/
// Translate the origin to the camera's location in world space.
vec3 getEyePoint( mat4 mvmtx )
{
    return -(mvmtx[3].xyz) * mat3(mvmtx);
}

// Construct the usual eye ray frustum oriented down the negative z axis,
// then transform it by the modelview matrix to go from camera to world space.
vec3 getEyeDirection( vec2 uv, mat4 mvmtx )
{
    float aspect = iResolution.x / iResolution.y;

    vec3 dir = vec3(
        uv.x * u_fov_y_scale*aspect,
        uv.y * u_fov_y_scale,
        -1.0);
    dir *= mat3(mvmtx);
    return normalize(dir);
}

// Get a per-fragment location value in [-1,1].
// Also apply a stereo tweak based on a uniform variable.
vec2 getSamplerUV( vec2 fragCoord )
{
    vec2 uv = fragCoord.xy;
    uv = -1.0 + 2.0*uv;
    uv.x += u_eyeballCenterTweak;
    return uv;
}

void main()
{
#if 1
    vec2 fc = gl_FragCoord.xy / iResolution.xy;
    fc.x = fract(fc.x);
#else
    vec2 fc = vfFragCoord;
#endif
    vec2 uv  = getSamplerUV( fc );
    vec3 ro  = getEyePoint( mvmtx );
    vec3 rd  = getEyeDirection( uv, mvmtx );
    vec3 col = getSceneColor( ro, rd );
    gl_FragColor = vec4(col, 1.0);
}
