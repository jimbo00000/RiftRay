// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// I can't recall where I learnt about this fractal.
//
// Coloring and fake occlusions are done by orbit trapping, as usual (unless somebody has invented
// something new in the last 4 years that i'm unaware of, that is)
//

// @var title Apollonian
// @var author iq
// @var url https://www.shadertoy.com/view/4ds3zn

// @var headSize 0.2
// @var eyePos2 -1.0, 1.2, -1.0
// @var eyePos3 -1.582 0.113 -0.555
// @var eyePos -2.304 0.110 -0.599
// @var vec3 light1 0.577 0.577 -0.577 dir
// @var vec3 light2 -0.707 0.000  0.707 dir
// @var vec3 col1 1.0 0.80 0.2 color
// @var vec3 col2 1.0 0.55 0.0 color
// @var float ss 1.1 0.5 2.0 0.01
// @var float km 0.1 0.0 20.0 0.01

#ifdef RIFTRAY
uniform vec3 light1;
uniform vec3 light2;
uniform vec3 col1;
uniform vec3 col2;
uniform float ss;
uniform float km;
#else
vec3 light1 = vec3(0.577, 0.577, -0.577);
vec3 light2 = vec3(-0.707, 0.000, 0.707);
vec3 col1 = vec3(1.0,0.80,0.92);
vec3 col2 = vec3(1.0,0.55,0.90);
float ss = 1.1;
float km = 0.1;
#endif

vec4 orb = vec4(1000.0);
float map( vec3 p )
{
	float scale = 1.0;

	orb = vec4(1000.0);
	
	for( int i=0; i<8;i++ )
	{
		p = -1.0 + 2.0*fract(0.5*p+0.5);

		float r2 = dot(p,p);
		
        orb = min( orb, vec4(abs(p),r2) );
		
		float k = max(ss/r2,km);
		p     *= k;
		scale *= k;
	}
	
	return 0.25*abs(p.y)/scale;
}

float trace( in vec3 ro, in vec3 rd )
{
	float maxd = 100.0;
	float precis = 0.001;
    float h=precis*2.0;
    float t = 0.0;
    for( int i=0; i<200; i++ )
    {
        if( abs(h)<precis||t>maxd ) continue;//break;
        t += h;
	    h = map( ro+rd*t );
    }

    if( t>maxd ) t=-1.0;
    return t;
}

vec3 calcNormal( in vec3 pos )
{
	vec3  eps = vec3(.0001,0.0,0.0);
	vec3 nor;
	nor.x = map(pos+eps.xyy) - map(pos-eps.xyy);
	nor.y = map(pos+eps.yxy) - map(pos-eps.yxy);
	nor.z = map(pos+eps.yyx) - map(pos-eps.yyx);
	return normalize(nor);
}

vec3 getSceneColor( vec3 ro, vec3 rd )
{
    // trace	
	vec3 col = vec3(0.0);
	float t = trace( ro, rd );
	if( t>0.0 )
	{
		vec4 tra = orb;
		vec3 pos = ro + t*rd;
		vec3 nor = calcNormal( pos );
		
		// lighting
        //vec3  light1 = vec3(  0.577, 0.577, -0.577 );
        //vec3  light2 = vec3( -0.707, 0.000,  0.707 );
		float key = clamp( dot( light1, nor ), 0.0, 1.0 );
		float bac = clamp( 0.2 + 0.8*dot( light2, nor ), 0.0, 1.0 );
		float amb = (0.7+0.3*nor.y);
		float ao = pow( clamp(tra.w*2.0,0.0,1.0), 1.2 );

		vec3 brdf  = 1.0*vec3(0.40,0.40,0.40)*amb*ao;
			 brdf += 1.0*vec3(1.00,1.00,1.00)*key*ao;
			 brdf += 1.0*vec3(0.40,0.40,0.40)*bac*ao;

        // material		
		vec3 rgb = vec3(1.0);
		rgb = mix( rgb, col1, clamp(6.0*tra.y,0.0,1.0) );
		rgb = mix( rgb, col2, pow(clamp(1.0-2.0*tra.z,0.0,1.0),8.0) );

		// color
		col = rgb*brdf*exp(-0.2*t);
	}

	col = sqrt(col);
	
	col = mix( col, smoothstep( 0.0, 1.0, col ), 0.25 );
    return col;
}

#ifndef RIFTRAY
void main(void)
{
	vec2 p = -1.0 + 2.0*gl_FragCoord.xy / iResolution.xy;
    p.x *= iResolution.x/iResolution.y;

	float time = iGlobalTime*0.25 + 0.01*iMouse.x;
	
    // camera
	vec3 ro = vec3( 2.8*cos(0.1+.33*time), 0.4 + 0.30*cos(0.37*time), 2.8*cos(0.5+0.35*time) );
	vec3 ta = vec3( 1.9*cos(1.2+.41*time), 0.4 + 0.10*cos(0.27*time), 1.9*cos(2.0+0.38*time) );
	float roll = 0.2*cos(0.1*time);
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(roll), cos(roll),0.0);
	vec3 cu = normalize(cross(cw,cp));
	vec3 cv = normalize(cross(cu,cw));
	vec3 rd = normalize( p.x*cu + p.y*cv + 2.0*cw );

    vec3 col = getSceneColor( ro, rd );
	
	gl_FragColor=vec4(col,1.0);
}
#endif
