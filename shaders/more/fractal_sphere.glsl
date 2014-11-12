// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// Based on Weird Thing by inigo quilez : https://www.shadertoy.com/view/XsB3Wc

// @var title Fractal sphere
// @var author guil
// @var url https://www.shadertoy.com/view/4sB3Dc

// @var eyePos 0.0 0.0 -1.8

vec2 iSphere( in vec3 ro, in vec3 rd, in vec4 sph )
{
	vec3 oc = ro - sph.xyz;
	float b = dot( oc, rd );
	float c = dot( oc, oc ) - sph.w*sph.w;
	float h = b*b - c;
	if( h<0.0 ) return vec2(-1.0);
	h = sqrt(h);
	return vec2(-b-h, -b+h );
}

const int MaxIter = 12;
vec3 sundir = vec3(0.0,0.5,-1.0);
float g=.8;
float h=1.;



vec4 map( vec3 p)
{
	float dr = 1.0;
	vec3 ot = vec3(1000.0); 
	float r2;
	for( int i=0; i<MaxIter;i++ )
	{
            
        r2 = dot(p,p);
        if(r2>100.)continue;
		
        ot = min( ot, abs(p) );


		float k = max(h/r2,1.)*g;
		p  *= k;
		dr *= k;
		p=abs(p-0.5)-1.;		
		
	}
	
	float d;
	//d = (abs(p.x)+abs(p.y))*length(p)/dr;	
	//d = (length(p.xz)*abs(p.y)+length(p.xy)*abs(p.z)+length(p.yz)*abs(p.x))/dr;
	//d = 1.5*(length(p.xz))*length(p.xy)/dr;
	//d = 1.*length(p)*log(length(p))/dr;
	//d =1.*length(p)/dr;
    d=abs(p.x)/dr;
	return vec4(ot,d);
	
}

vec4 raymarch( in vec3 ro, in vec3 rd , in vec2 tminmax )
{
	vec4 sum = vec4(0, 0, 0, 0);

	float t = tminmax.x;
	for(int i=0; i<64; i++)
	{
		if( sum.a > 0.99 || t>tminmax.y ) continue;

		vec3 pos = ro + t*rd;		
		vec4 col = map( pos );
		float d = col.a;
		col.a = min(0.002/d,1.);
		col.rgb *= col.a;

		sum = sum + col*(1.0 - sum.a);
        t += min(0.1,d*.3);	
                		
	}

	sum.xyz /= (0.001+sum.w);

	return clamp( sum, 0.0, 1.0 );
}

vec3 getSceneColor( vec3 ro, vec3 rd )
{
	vec3 col = vec3(0.08,0.09,0.18);
    vec2 seg = iSphere( ro, rd, vec4(0.0,0.0,0.0,1.0) );
	if( seg.x>0.0 )
	{
        vec4 res = raymarch( ro, rd, seg );
	    col = mix( col, res.xyz, res.w );
	}
	
	col = mix( col, vec3(dot(col,vec3(0.333))), -0.1 );
	
	col = pow( col, vec3(0.45) ) * 1.2;

	//col *= sqrt( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y) );
	return col;
}

#ifndef RIFTRAY
void main(void)
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 p = -1.0 + 2.0*q;
    p.x *= iResolution.x/ iResolution.y;
    vec2 mo = iMouse.xy / iResolution.xy;
    float an = 2.0 + 0.2*iGlobalTime - mo.x;

	vec3 ro = 2.0*vec3(cos(an), 0.17, sin(an));
	vec3 ta = vec3(0.0, 0.0, 0.0);
    vec3 ww = normalize( ta - ro);
    vec3 uu = normalize( cross( vec3(0.0,1.0,0.0), ww ) );
    vec3 vv = normalize( cross(ww,uu) );
    vec3 rd = normalize( p.x*uu + p.y*vv + 2.0*ww );

    vec3 col = getSceneColor( ro, rd );
    gl_FragColor = vec4( col, 1.0 );
}
#endif
