// Other World
/* created by okanovic dragan (abstract algorithm) */

// @var title Other World
// @var author okanovic dragan
// @var url https://www.shadertoy.com/view/Xs23R1

// @var eyePos 0.0 3.2 0.0

#define time iGlobalTime
//#define SHADING	// turn shading on/off
//#define SHADOWS	// turn shadows on/off, works only if shading is enabled

//----------------------------------------------------------------------
float sdSphere( vec3 p, float s ) {
  return length(p)-s;
}
float sdPlane( vec3 p ) {
    return p.y;
}
float sdCube( vec3 p, float a ) {
  return length(max(abs(p) - vec3(a),0.0));
}
float opU( float d1, float d2 ) {
    return (d1<d2) ? d1 : d2;
}
float smin( float a, float b) {
    float k=3.1;
    float res = exp( -k*a ) + exp( -k*b );
    return -log( res )/k;
    k = 0.9;
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float map( in vec3 pos ) {
#define REPEAT 4.
	float r = rand(vec2(ceil(.2*pos.x), ceil(.2*pos.z)));
    pos.x = mod(pos.x, REPEAT) - .5*REPEAT;
    pos.z = mod(pos.z, REPEAT) - .5*REPEAT;
    return opU( smin( sdPlane(pos), sdCube(pos-vec3( 0.0, 0.5, 0.0), 0.5 ) ),
			    sdSphere(pos-vec3(0,cos(40.*r+3.*time)*.5+2.5,0.), .5));
}
//----------------------------------------------------------------------
vec2 castRay( in vec3 ro, in vec3 rd, in float maxd ) {
    float precis = 0.001;   // when to call a hit
    float h=precis*2.0;     // how much to move along the ray
    float t = 0.0;          // moved already
    vec2 m = vec2(-1.0);    // color/uv, depends on use, here - uv
    vec3 pos = vec3(0.0);   // 3d position
    float mf = 0.0;
    for( int i=0; i<60; i++ )
    {
        if( abs(h)<precis||t>maxd ) continue;  // voila
        t += h;                 //  move
        pos = ro+rd*t;          // update current 3d position
        h = map( pos );         // get de
        mf += 1.;
    }
    
    // it was a good day :)
    return vec2( t, mf );
}
#ifdef SHADING
#ifdef SHADOW
// approximate shadow :: http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm
float softshadow( in vec3 ro, in vec3 rd, in float mint, in float maxt, in float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<60; i++ )
    {
        if( t<maxt )
        {
            float h = map( ro + rd*t );
            res = min( res, k*h/t );
            t += 0.02;
        }
    }
    return clamp( res, 0.0, 1.0 );
}
#endif


// approximate normal :: http://code4k.blogspot.com/2009/10/potatro-and-raymarching-story-of.html
vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3( 0.001, 0.0, 0.0 );
    vec3 nor = vec3(
        map(pos+eps.xyy) - map(pos-eps.xyy),
        map(pos+eps.yxy) - map(pos-eps.yxy),
        map(pos+eps.yyx) - map(pos-eps.yyx) );
    return normalize(nor);
}
// ambient occlussion
float calcAO( in vec3 pos, in vec3 nor )
{
    float totao = 0.0;
    float sca = 1.0;
    for( int aoi=0; aoi<5; aoi++ )
    {
        float hr = 0.01 + 0.05*float(aoi);
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos );
        totao += -(dd-hr)*sca;
        sca *= 0.75;
    }
    return clamp( 1.0 - 4.0*totao, 0.0, 1.0 );
}
#endif
// brdf thingy
vec3 render( in vec3 ro, in vec3 rd )
{ 
    vec3 col = vec3(0.0);
    vec2 res = castRay(ro,rd,20.0);
    float t = res.x;
    float mf = res.y;
    vec3 pos = ro + t*rd;
#ifdef SHADING
    vec3 nor = calcNormal( pos );
    float ao = calcAO( pos, nor );
#endif
    //col = step(t, 20.)*vec3(1., (60.-mf)/60., 0.);
	col = step(t, 20.)*vec3((mf)/60.);

#ifdef SHADING
    vec3 lig = normalize( vec3(-0.6, 0.7, -0.5) );  // light direction (0., 1., 0.)
    float amb = clamp( 0.5+0.5*nor.y, 0.0, 1.0 );
    float dif = clamp( dot( nor, lig ), 0.0, 1.0 );
    float bac = clamp( dot( nor, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-pos.y,0.0,1.0);

    float sh = 1.0;

#ifdef SHADOWS
    if( dif>0.02 ) { sh = softshadow( pos, lig, 0.02, 10.0, 7.0 ); dif *= sh; }
#endif

    vec3 brdf = vec3(0.0);
    brdf += 0.20*amb*vec3(0.10,0.11,0.13)*ao;
    brdf += 0.20*bac*vec3(0.15,0.15,0.15)*ao;
    brdf += 1.20*dif*vec3(1.00,0.90,0.70);

    float pp = clamp( dot( reflect(rd,nor), lig ), 0.0, 1.0 );
    float spe = sh*pow(pp,16.0);
    float fre = ao*pow( clamp(1.0+dot(nor,rd),0.0,1.0), 2.0 );

    col = col*brdf + /*vec3(1.0)*col*spe +*/ 0.2*fre*(0.5+0.5*col);
#endif

    col *= exp( -0.01*t*t );

    return vec3( clamp(col,0.0,1.0) );
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    // main thing
    vec3 col = sqrt(render( ro, rd ));
    
    // vignette
    //col *= 0.2 + 0.8*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );
	return col;
}


#ifndef RIFTRAY
void main( void )
{
    vec2 q = gl_FragCoord.xy/iResolution.xy;
    vec2 p = -1.0+2.0*q;
    p.x *= iResolution.x/iResolution.y;
    
    float time = 5.0 + 4.*iGlobalTime + 20.0*iMouse.x/iResolution.x;

    // camera   
    vec3 ro = vec3( 7.*cos(0.2*time),
                    1.3,
                    7.*sin(0.2*time) );     // camera position aka ray origin

    vec3 ta = vec3( 0.0, 1.2, 0.0 );        // camera look-at position
    
    // camera tx
    vec3 eye = normalize( ta-ro );                      // eye vector
    vec3 cp = vec3( 0.0, 1.0, 0.0 );                    // "up" vector
    vec3 hor = normalize( cross(eye, cp) );             // horizontal vector
    vec3 up = normalize( cross(hor, eye) );             // up vector
    vec3 rd = normalize( p.x*hor + p.y*up + 2.5*eye );  // ray direction
    
	vec3 col = getSceneColor( ro, rd );
    // ta-da!
    gl_FragColor=vec4( col, 1.0 );
}
#endif
