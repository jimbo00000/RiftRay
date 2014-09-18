// raymarch.frag
#version 330

varying vec2 vfFragCoord;

// ShaderToy Inputs:
uniform vec3 iResolution;
uniform vec3 iChannelResolution[4]; // channel resolution (in pixels)

// Oculus-specific additions:
uniform float u_eyeballCenterTweak;
uniform float u_fov_y_scale;
uniform mat4 mvmtx;
uniform mat4 prmtx;
uniform mat4 obmtx;


// Simple ray marching example
// @var url https://www.shadertoy.com/view/ldB3Rw
// @var author gltracy
// @var license CC BY-NC-SA 3.0

// @var headSize 6.0
// @var eyePos -2.5952096 5.4259381 -20.277588

const int max_iterations = 255;
const float stop_threshold = 0.001;
const float grad_step = 0.1;
const float clip_far = 1000.0;

// math
const float PI = 3.14159265359;
const float DEG_TO_RAD = PI / 180.0;

// distance function
float dist_sphere( vec3 pos, float r ) {
    return length( pos ) - r;
}

float dist_box( vec3 pos, vec3 size ) {
    return length( max( abs( pos ) - size, 0.0 ) );
}

float dist_box( vec3 v, vec3 size, float r ) {
    return length( max( abs( v ) - size, 0.0 ) ) - r;
}

// get distance in the world
float dist_field( vec3 pos ) {
    // ...add objects here...

    // floor
    float thick = 0.00001;
    float d2 = dist_box( pos + vec3( 0.0, 0.0, 0.0 ), vec3( 10.0, thick, 10.0 ), 0.05 );
    float d3 = dist_box( pos + vec3( 0.0, -3.0, 0.0 ), vec3( 10.0, thick, 10.0 ), 0.05 );
    d2 = min( d2, d3 );

    // Manually tease out translation component
    vec3 trans = obmtx[3].xyz;
    pos -= trans;

    pos.y -= 1.5;
    // Multiplying this in reverse order(vec*mtx) is equivalent to transposing the matrix.
    pos = vec4( vec4(pos,1.0) * obmtx ).xyz;

    // Manually tease out scale component
    float scale = length(obmtx[0].xyz);
    pos /= scale * scale;

    // object 0 : sphere
    float d0 = dist_sphere( pos, 0.5*1.35 );

    // object 1 : cube
    float d1 = dist_box( pos, vec3( 0.5*1.0 ) );

    // union     : min( d0,  d1 )
    // intersect : max( d0,  d1 )
    // subtract  : max( d1, -d0 )
    return max( d1, -d0 );
    //return min( d2, max( d1, -d0 ) );
}

// phong shading
vec3 shading( vec3 v, vec3 n, vec3 eye ) {
    // ...add lights here...
    float shininess = 16.0;
    vec3 final = vec3( 0.0 );

    vec3 ev = normalize( v - eye );
    vec3 ref_ev = reflect( ev, n );

    // light 0
    {
        vec3 light_pos   = vec3( 20.0, 20.0, 20.0 );
        vec3 light_color = vec3( 1.0, 0.7, 0.7 );

        vec3 vl = normalize( light_pos - v );

        float diffuse  = max( 0.0, dot( vl, n ) );
        float specular = max( 0.0, dot( vl, ref_ev ) );
        specular = pow( specular, shininess );

        final += light_color * ( diffuse + specular );
    }

    // light 1
    {
        vec3 light_pos   = vec3( -20.0, -20.0, -20.0 );
        vec3 light_color = vec3( 0.3, 0.7, 1.0 );

        vec3 vl = normalize( light_pos - v );

        float diffuse  = max( 0.0, dot( vl, n ) );
        float specular = max( 0.0, dot( vl, ref_ev ) );
        specular = pow( specular, shininess );

        final += light_color * ( diffuse + specular );
    }

    return final;
}

// get gradient in the world
vec3 gradient( vec3 pos ) {
    const vec3 dx = vec3( grad_step, 0.0, 0.0 );
    const vec3 dy = vec3( 0.0, grad_step, 0.0 );
    const vec3 dz = vec3( 0.0, 0.0, grad_step );
    return normalize (
        vec3(
            dist_field( pos + dx ) - dist_field( pos - dx ),
            dist_field( pos + dy ) - dist_field( pos - dy ),
            dist_field( pos + dz ) - dist_field( pos - dz )
        )
    );
}

// ray marching
float ray_marching( vec3 origin, vec3 dir, float start, float end ) {
    float depth = start;
    for ( int i = 0; i < max_iterations; i++ ) {
        float dist = dist_field( origin + dir * depth );
        if ( dist < stop_threshold ) {
            return depth;
        }
        depth += dist;
        if ( depth >= end) {
            return end;
        }
    }
    return end;
}

///////////////////////////////////////////////////////////////////////////////
// Agreeing on certian entry points, we can patch in the Rift's heading:
//
// vec3 getEyePoint( mat4 mvmtx );
// vec3 getEyeDirection( vec2 uv, mat4 mvmtx );
// vec2 getSamplerUV( vec2 fragCoord );
// vec3 getSceneColor( in vec3 ro, in vec3 rd );
//
///////////////////////////////////////////////////////////////////////////////
vec3 getSceneColor( in vec3 ro, in vec3 rd, inout float depth )
{
    // ray marching
    depth = ray_marching( ro, rd, 0.0, clip_far );
    if ( depth >= clip_far ) {
        discard;
    }

    // shading
    vec3 pos = ro + rd * depth;
    vec3 n = gradient( pos );
    vec3 col = shading( pos, n, ro );

    return col;
}

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
        uv.x * u_fov_y_scale * aspect,
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
    vec2 uv  = getSamplerUV( vfFragCoord );
    vec3 ro  = getEyePoint( mvmtx );
    vec3 rd  = getEyeDirection( uv, mvmtx );
    float depth = 9999.0;
    vec3 col = getSceneColor( ro, rd, depth );

    // Write to depth buffer
    vec3 eyeFwd = getEyeDirection( vec2(0.0), mvmtx );
    float eyeHitZ = -depth;// * dot( rd, eyeFwd );

    float p10 = prmtx[2].z;
    float p11 = prmtx[3].z;
    // A little bit of algebra...
    float ndcDepth = -p10 + -p11 / eyeHitZ;

    gl_FragDepth = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
    gl_FragColor = vec4(col, 1.0);
}
