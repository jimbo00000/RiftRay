// rwwtt.frag
// from writing a basic raytracer 
// Iñigo Quilez

varying vec2 vfFragCoord;


// ShaderToy Inputs:
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
//uniform float     iChannelTime[4];       // channel playback time (in seconds)
//uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
//uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
//uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
//uniform vec4      iDate;                 // (year, month, day, time in seconds)

// Oculus-specific additions:
uniform float u_eyeballCenterTweak;
uniform mat4 mvmtx;
uniform mat4 prmtx;






#if 0

float pi = 3.14159265358979323846;

float iSphere( in vec3 ro, in vec3 rd )
{
    float r = 1.0;
    float b = 2.0*dot( ro, rd );
    float c = dot(ro,ro) - r*r;
    float h = b*b - 4.0*c;
    if( h<0.0 ) return -1.0;
    float t = (-b - sqrt(h))/2.0;
    return t;
}

float iPlane( in vec3 ro, in vec3 rd )
{
    return -ro.y/rd.y;
}

float intersect( in vec3 ro, in vec3 rd, out float resT )
{
    float id = -1.0;
    resT = 1000.0;

    float tsph = iSphere( ro, rd );
    float tpla = iPlane( ro, rd );
    if ( tsph>0.0 )
    {
        id = 1.0;
        resT = tsph;
    }
    if( tpla>0.0 && tpla<resT )
    {
        id = 2.0;
        resT = tpla;
    }
    return id;
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    float t;
    float id = intersect( ro, rd, t );

    vec3 hitPt = ro + t*rd;

    vec3 col = vec3(0.0);
    if( id>0.5 && id<1.5 )
    {
        col = vec3( 0.0, 0.0, 1.0 );
    }
    else if (id>1.5 )
    {
        float freq = 0.5 * pi;
        float lum = round(0.5 + 2.0*sin(freq*hitPt.x) * sin(freq*hitPt.z));
        col = vec3(lum);
    }
    return col;
}

#endif







#if 0

// https://www.shadertoy.com/view/MssGz2
// Created by Denis Antiga a.denis1 at yahoo.com
// Started from the sample of inigo quilez 
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Example of an interesting Join Operator in the distance field

float sdPlaneY( vec3 p )
{
    return p.y;
}

float sdSphere( vec3 p, float s )
{
    return length(p)-s;
}

float length2( vec2 p )
{
    return sqrt( p.x*p.x + p.y*p.y );
}

//----------------------------------------------------------------------
vec2 opU( vec2 d1, vec2 d2 )
{
    return (d1.x<d2.x) ? d1 : d2;
}

vec2 opLink(float d1,float d2,float r,float c1,float c2)
{
    float dmin;
    if (d1<d2)
        dmin=d1;
    else
        dmin=d2;

    float p2=(d1+d2+r*4.)*0.5;
    float r2=r*r;
    float ds1=sqrt(r2+r2)-r;
    float ds2=sqrt(r2*4.+r2)-r;
    float a=(sqrt(p2*(p2-(d1+r))*(p2-(d2+r))*(p2-2.*r)));

    if ((d1<=ds1+0.1&&d2<=ds2)||(d1<=ds2&&d2<=ds1+0.1))
        {
        dmin=a/r-r;
        return vec2(dmin,c2);
        }
    return vec2(dmin,c1);
}

//----------------------------------------------------------------------
vec2 map( in vec3 pos )
{
    vec2 res = opU( vec2( sdPlaneY(     pos), 1.0 ),
                    opLink(sdSphere(    pos-vec3( 0.0,0.25, 0.0), 0.25 ),
                            sdSphere(    pos-vec3( 0.0+cos(iGlobalTime*0.3)*0.9,0.25, 0.0), 0.25 ),
                            0.25,30.0,45.0) );
    return res;
}

vec2 castRay( in vec3 ro, in vec3 rd, in float maxd )
{
    float precis = 0.00001;
    float h=precis*2.0;
    float t = 0.0;
    float m = -1.0;
    for( int i=0; i<60; i++ )
    {
        if( abs(h)<precis||t>maxd ) continue;//break;
        t += h;
        vec2 res = map( ro+rd*t );
        h = res.x;
        m = res.y;
    }

    if( t>maxd ) m=-1.0;
    return vec2( t, m );
}

float softshadow( in vec3 ro, in vec3 rd, in float mint, in float maxt, in float k )
{
    float res = 1.0;
    float dt = 0.02;
    float t = mint;
    for( int i=0; i<30; i++ )
    {
        if( t<maxt )
        {
        float h = map( ro + rd*t ).x;
        res = min( res, k*h/t );
        t += max( 0.02, dt );
        }
    }
    return clamp( res, 0.0, 1.0 );
}

vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3( 0.001, 0.0, 0.0 );
    vec3 nor = vec3(
        map(pos+eps.xyy).x - map(pos-eps.xyy).x,
        map(pos+eps.yxy).x - map(pos-eps.yxy).x,
        map(pos+eps.yyx).x - map(pos-eps.yyx).x );
    return normalize(nor);
}

float calcAO( in vec3 pos, in vec3 nor )
{
    float totao = 0.0;
    float sca = 1.0;
    for( int aoi=0; aoi<5; aoi++ )
    {
        float hr = 0.01 + 0.05*float(aoi);
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos ).x;
        totao += -(dd-hr)*sca;
        sca *= 0.75;
    }
    return clamp( 1.0 - 4.0*totao, 0.0, 1.0 );
}

vec3 render( in vec3 ro, in vec3 rd )
{
    vec3 col = vec3(0.0);
    vec2 res = castRay(ro,rd,20.0);
    float t = res.x;
    float m = res.y;
    if( m>-0.5 )
    {
        vec3 pos = ro + t*rd;
        vec3 nor = calcNormal( pos );

        //col = vec3(0.6) + 0.4*sin( vec3(0.05,0.08,0.10)*(m-1.0) );
        col = vec3(0.6) + 0.4*sin( vec3(0.05,0.08,0.10)*(m-1.0) );
        
        float ao = calcAO( pos, nor );

        vec3 lig = normalize( vec3(-0.6, 0.7, -0.5) );
        float amb = clamp( 0.5+0.5*nor.y, 0.0, 1.0 );
        float dif = clamp( dot( nor, lig ), 0.0, 1.0 );
        float bac = clamp( dot( nor, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-pos.y,0.0,1.0);

        float sh = 1.0;
        if( dif>0.02 ) { sh = softshadow( pos, lig, 0.02, 10.0, 7.0 ); dif *= sh; }

        vec3 brdf = vec3(0.0);
        brdf += 0.20*amb*vec3(0.10,0.11,0.13)*ao;
        brdf += 0.20*bac*vec3(0.15,0.15,0.15)*ao;
        brdf += 1.20*dif*vec3(1.00,0.90,0.70);

        float pp = clamp( dot( reflect(rd,nor), lig ), 0.0, 1.0 );
        float spe = sh*pow(pp,16.0);
        float fre = ao*pow( clamp(1.0+dot(nor,rd),0.0,1.0), 2.0 );

        col = col*brdf + vec3(1.0)*col*spe + 0.2*fre*(0.5+0.5*col);
        
    }
    col *= exp( -0.01*t*t );
    return vec3( clamp(col,0.0,1.0) );
}

#endif




// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

vec3 hash3( float n )
{
    return fract(sin(vec3(n,n+1.0,n+2.0))*vec3(43758.5453123,22578.1459123,19642.3490423));
}

// ripped from Kali's Lonely Tree shader
mat3 rotationMat(vec3 v, float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}

vec3 axis = normalize( vec3(-0.3,-1.,-0.4) );

vec2 map( vec3 p )
{
	
    // animation	
	float atime = iGlobalTime+12.0;
	vec3 o = floor( 0.5 + p/50.0  );
	float o1 = hash( o.x*57.0 + 12.1234*o.y  + 7.1*o.z );
	float f = sin( 1.0 + (2.0*atime + 31.2*o1)/2.0 );
	p.y -= 2.0*(atime + f*f);
	p = mod( (p+25.0)/50.0, 1.0 )*50.0-25.0;
	if( abs(o.x)>0.5 )  p += (-1.0 + 2.0*o1)*10.0;
	mat3 roma = rotationMat(axis, 0.34 + 0.07*sin(31.2*o1+2.0*atime + 0.1*p.y) );

    // modeling	
	for( int i=0; i<16; i++ ) 
	{
		p = roma*abs(p);
		p.y-= 1.0;
	}
	
	float d = length(p*vec3(1.0,0.1,1.0))-0.75;
	float h = 0.5 + p.z;
		
	return vec2( d, h );
}

vec3 intersect( in vec3 ro, in vec3 rd )
{
	float maxd = 140.0;
	float precis = 0.001;
    float h=precis*2.0;
    float t = 0.0;
	float d = 0.0;
    float m = 1.0;
    for( int i=0; i<128; i++ )
    {
        if( abs(h)<precis||t>maxd ) continue;//break;
        t += h;
	    vec2 res = map( ro+rd*t );
        h = min( res.x, 5.0 );
		d = res.y;
    }

    if( t>maxd ) m=-1.0;
    return vec3( t, d, m );
}

vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3(0.2,0.0,0.0);

	return normalize( vec3(
           map(pos+eps.xyy).x - map(pos-eps.xyy).x,
           map(pos+eps.yxy).x - map(pos-eps.yxy).x,
           map(pos+eps.yyx).x - map(pos-eps.yyx).x ) );
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<48; i++ )
    {
        float h = map(ro + rd*t).x;
		h = max( h, 0.0 );
        res = min( res, k*h/t );
        t += clamp( h, 0.01, 0.5 );
    }
    return clamp(res,0.0,1.0);
}

float calcAO( in vec3 pos, in vec3 nor )
{
	float totao = 0.0;
    for( int aoi=0; aoi<16; aoi++ )
    {
		vec3 aopos = -1.0+2.0*hash3(float(aoi)*213.47);
		aopos *= sign( dot(aopos,nor) );
		aopos = pos + aopos*0.5;
        float dd = clamp( map( aopos ).x*4.0, 0.0, 1.0 );
        totao += dd;
    }
	totao /= 16.0;
	
    return clamp( totao*totao*1.5, 0.0, 1.0 );
}

vec3 lig = normalize(vec3(-0.5,0.7,-1.0));



vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
	// render
	vec3 bgc = 0.6*vec3(0.8,0.9,1.0)*(0.5 + 0.3*rd.y);
    vec3 col = bgc;

	// raymarch
    vec3 tmat = intersect(ro,rd);
    if( tmat.z>-0.5 )
    {
        // geometry
        vec3 pos = ro + tmat.x*rd;
        vec3 nor = calcNormal(pos);

        // material
		vec3 mate = 0.5 + 0.5*mix( sin( vec3(1.2,1.1,1.0)*tmat.y*3.0 ),
		                           sin( vec3(1.2,1.1,1.0)*tmat.y*6.0 ), 
								   1.0-abs(nor.y) );
		// lighting
		float occ = calcAO( pos, nor );
        float amb = 0.8 + 0.2*nor.y;
        float dif = max(dot(nor,lig),0.0);
        float bac = max(dot(nor,normalize(vec3(-lig.x,0.0,-lig.z))),0.0);
		float sha = 0.0; if( dif>0.001 ) sha=softshadow( pos, lig, 0.1, 32.0 );
        float fre = pow( clamp( 1.0 + dot(nor,rd), 0.0, 1.0 ), 2.0 );

		// lights
		vec3 brdf = vec3(0.0);
        brdf += 1.0*dif*vec3(1.00,0.90,0.65)*pow(vec3(sha),vec3(1.0,1.2,1.5));
		brdf += 1.0*amb*vec3(0.05,0.05,0.05)*occ;
		brdf += 1.0*bac*vec3(0.03,0.03,0.03)*occ;
        brdf += 1.0*fre*vec3(1.00,0.70,0.40)*occ*(0.2+0.8*sha);
        brdf += 1.0*occ*vec3(1.00,0.70,0.30)*occ*max(dot(-nor,lig),0.0)*pow(clamp(dot(rd,lig),0.0,1.0),64.0)*tmat.y*2.0;
		
        // surface-light interacion		
		col = mate * brdf;

        // fog		
		col = mix( col, bgc, clamp(1.0-1.2*exp(-0.0002*tmat.x*tmat.x ),0.0,1.0) );
    }
	else
	{
        // sun		
	    vec3 sun = vec3(1.0,0.8,0.5)*pow( clamp(dot(rd,lig),0.0,1.0), 32.0 );
	    col += sun;
	}

    // sun scatter	
	col += 0.6*vec3(0.2,0.14,0.1)*pow( clamp(dot(rd,lig),0.0,1.0), 5.0 );


	// postprocessing
	
    // gamma
	col = pow( col, vec3(0.45) );
	
	// contrast/brightness
	col = 1.3*col-0.1;
	
    // tint	
	col *= vec3( 1.0, 1.04, 1.0);
    
    return col;
}





void main()
{
    vec2 uv = vfFragCoord.xy;
    uv = -1.0+2.0*uv;
    uv.x += u_eyeballCenterTweak;

    float aspect = iResolution.x / iResolution.y;
    float fov_y_scale = 1.0;

    // http://blog.hvidtfeldts.net/
    vec3 eye = -(mvmtx[3].xyz) * mat3(mvmtx);
    vec3 dir = vec3(
        uv.x*fov_y_scale*aspect,
        uv.y*fov_y_scale,
        -1.0)
      * mat3(mvmtx);
    vec3 cameraForward = vec3(0,0,-1.0) * mat3(mvmtx);
    vec3 cameraUp = vec3(0,1.0,0) * mat3(mvmtx);
    vec3 rayDirection = normalize(dir);

    vec3 ro = eye;
    vec3 rd = rayDirection;

    vec3 col = getSceneColor( ro, rd );

    gl_FragColor = vec4(col, 1.0);
}
