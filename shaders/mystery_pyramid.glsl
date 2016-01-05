// Everyday004 - MysteryPyramid
// By David Ronai / @Makio64

// Thanks to XT95 to implement SSS in this shadertoy:
// https://www.shadertoy.com/view/MsdGz2
// Base on this papper :
// http://colinbarrebrisebois.com/2011/03/07/gdc-2011-approximating-translucency-for-a-fast

// @var title Everyday004 - MysteryPyramid
// @var author Makio64
// @var url https://www.shadertoy.com/view/Mdt3zs

// @var tex0 tex19.png
// @var eyePos -0.63 11.44 -26.0

//------------------------------------------------------------------ VISUAL QUALITY
//#define POSTPROCESS
#define RAYMARCHING_STEP 40
#define RAYMARCHING_JUMP 1.
//------------------------------------------------------------------ DEBUG
//#define RENDER_DEPTH
//#define RENDER_NORMAL
//#define RENDER_AO

const float PI = 3.14159265359;

//------------------------------------------------------------------  SIGNED PRIMITIVES
float pyramid( vec3 p, float h) {
	vec3 q=abs(p);
	return max(-p.y, (q.x*1.5+q.y+q.z*1.5-h)/3.0 );
}
//http://mercury.sexy/hg_sdf/
float vmax(vec3 v) {return max(max(v.x, v.y), v.z);}
float fBox(vec3 p, vec3 b) {
	vec3 d = abs(p) - b;
	return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
}
float pModPolar(inout vec2 p, float repetitions) {
	float angle = 2.*PI/repetitions;
	float a = atan(p.y, p.x) + angle/2.;
	float r = length(p);
	float c = floor(a/angle);
	a = mod(a,angle) - angle/2.;
	p = vec2(cos(a), sin(a))*r;
	if (abs(c) >= (repetitions/2.)) c = abs(c);
	return c;
}
void pR45(inout vec2 p) {
	p = (p + vec2(p.y, -p.x))*sqrt(0.5);
}

//------------------------------------------------------------------ MAP
float map( in vec3 pos ) {
    vec3 q = pos;
    pR45(q.xz);
	float d = pyramid(q,20.);
    pModPolar(q.xz,4.); 
    q.x-=11.;
	d = min(pyramid(q,5.),d); 
    d = min(-2.55+distance(pos.xyz,vec3(0.))/4.+pos.y-max(.1,texture2D(iChannel0,pos.xz/20.).x)*2.1,d);
    d = max(-fBox(pos+vec3(0.,-2.,5.5),vec3(1.,1.,5.5)),d);
    //d = min(-fSphere(pos,50.),d);
	return d;
}

//------------------------------------------------------------------ RAYMARCHING
#ifdef RENDER_DEPTH
float castRay( in vec3 ro, in vec3 rd, inout float depth )
#else
float castRay( in vec3 ro, in vec3 rd )
#endif
{
	float t = 15.0;
	float res;
	for( int i=0; i<RAYMARCHING_STEP; i++ )
	{
		vec3 pos = ro+rd*t;
		res = map( pos );
		if( res < 0.01 || t > 150. ) break;
		t += res*RAYMARCHING_JUMP;
		#ifdef RENDER_DEPTH
		depth += 1./float(RAYMARCHING_STEP);
		#endif
	}
	return t;
}

vec3 calcNormal(vec3 p) {
	float eps = 0.01;
	const vec3 v1 = vec3( 1.0,-1.0,-1.0);
	const vec3 v2 = vec3(-1.0,-1.0, 1.0);
	const vec3 v3 = vec3(-1.0, 1.0,-1.0);
	const vec3 v4 = vec3( 1.0, 1.0, 1.0);
	return normalize( v1 * map( p + v1*eps ) +
					  v2 * map( p + v2*eps ) +
					  v3 * map( p + v3*eps ) +
					  v4 * map( p + v4*eps ) );
}

float hash( float n ){
	return fract(sin(n)*3538.5453);
}

float calcAO( in vec3 p, in vec3 n, float maxDist, float falloff ){
	float ao = 0.0;
	const int nbIte = 6;
	for( int i=0; i<nbIte; i++ )
	{
		float l = hash(float(i))*maxDist;
		vec3 rd = n*l;
		ao += (l - map( p + rd )) / pow(1.+l, falloff);
	}
	return clamp( 1.-ao/float(nbIte), 0., 1.);
}

// calculate local thickness
// base on AO but : inverse the normale & inverse the color
float thickness( in vec3 p, in vec3 n, float maxDist, float falloff )
{
	float ao = 0.0;
	const int nbIte = 6;
	for( int i=0; i<nbIte; i++ )
	{
		float l = hash(float(i))*maxDist;
		vec3 rd = -n*l;
		ao += (l + map( p + rd )) / pow(1.+l, falloff);
	}
	return clamp( 1.-ao/float(nbIte), 0., 1.);
}

//------------------------------------------------------------------ POSTEFFECTS

#ifdef POSTPROCESS
vec3 postEffects( in vec3 col, in vec2 uv, in float time )
{
	// gamma correction
	// col = pow( clamp(col,0.0,1.0), vec3(0.45) );
	// vigneting
	col *= 0.4+0.6*pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.5 );
	return col;
}
#endif

vec3 render( in vec3 ro, in vec3 rd, in vec2 uv )
{
	vec3 col = vec3(.0,.0,1.2);

	#ifdef RENDER_DEPTH
	float depth = 0.;
	float t = castRay(ro,rd,depth);
	#else
	float t = castRay(ro,rd);
	#endif

	#ifdef RENDER_DEPTH
	return vec3(depth/10.,depth/5.,depth);
	#endif

	vec3 pos = ro + t * rd;
	vec3 nor = calcNormal(pos);

	#ifdef RENDER_NORMAL
	return nor;
	#endif

	float ao = calcAO(pos,nor,10.,1.2);
	#ifdef RENDER_AO
	return vec3(ao);
	#endif

    float thi = thickness(pos, nor, 6., 1.5);

    vec3 lpos1 = vec3(0.0,10.,0.0);
	vec3 ldir1 = normalize(lpos1-pos);
	float latt1 = pow( length(lpos1-pos)*.1, 1.5+.2*abs(sin(iGlobalTime/2.)) );
    float trans1 =  pow( clamp( dot(-rd, -ldir1+nor), 0., 1.), 1.) + 1.;
	vec3 diff1 = vec3(.0,.5,1.) * (max(dot(nor,ldir1),0.) ) / latt1;
	col =  diff1;
	col += vec3(.3,.2,.05) * (trans1/latt1)*thi;
    if(pos.y<=15.){
    	vec3 lpos2 = vec3(0.);
		vec3 ldir2 = normalize(lpos2-pos);
		float latt2 = pow( length(lpos2-pos)*.1, 2.);
    	float trans2 =  pow( clamp( dot(-rd, -ldir2+nor), 0., 1.), 1.) + 1.;
		vec3 diff2 = vec3(.0,.5,1.) * (max(dot(nor,ldir2),0.) ) / latt2;
		col *= .1;
        col +=  diff2;
		col += vec3(.1,.2,.3) * (trans2/latt2)*thi;
    }
    float d = distance(pos.xyz,vec3(0.));
	col = max(vec3(.05),col);
	col *= ao;
	return col;
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
	return render( ro, rd, vec2(0.) );
}

#ifndef RIFTRAY
mat3 setCamera( in vec3 ro, in vec3 ta, float cr )
{
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
	return mat3( cu, cv, cw );
}

vec3 orbit(float phi, float theta, float radius)
{
	return vec3(
		radius * sin( phi ) * cos( theta ),
		radius * cos( phi ),
		radius * sin( phi ) * sin( theta )
	);
}

//------------------------------------------------------------------ MAIN
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 p = -1. + 2. * uv;
    p.x *= iResolution.x / iResolution.y;
    
    //Camera
	float radius = 50.;
	vec3 ro = orbit(PI/2.-.5,PI/2.+iGlobalTime,radius);
   // ro.z -= iGlobalTime*4.;
	vec3 ta  = vec3(0.0, 0., 0.0);
	mat3 ca = setCamera( ro, ta, 0. );
	vec3 rd = ca * normalize( vec3(p.xy,1.7) );

	// Raymarching
	vec3 color = render( ro, rd, uv );
	#ifdef POSTPROCESS
	color = postEffects( color, uv, iGlobalTime );
	#endif
	fragColor = vec4(color,1.0);
}
#endif
