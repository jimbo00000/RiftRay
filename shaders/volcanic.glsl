// Volcanic
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// @var title Volcanic
// @var url https://www.shadertoy.com/view/XsX3RB
// @var author iq
// @var license CC BY-NC-SA 3.0

// @var headSize 1.0

//#define STEREO 

//@var tex0 tex16.png
//@var tex1 tex06.jpg
//@var tex2 tex09.jpg

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel0, (uv+ 0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z );
}

float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
	vec2 uv = p.xy + f.xy*f.xy*(3.0-2.0*f.xy);
	return texture2D( iChannel0, (uv+118.4)/256.0, -100.0 ).x;
}

#ifdef STEREO
#define lodbias -5.0
#else
#define lodbias 0.0
#endif

vec4 texcube( sampler2D sam, in vec3 p, in vec3 n )
{
	vec4 x = texture2D( sam, p.yz, lodbias );
	vec4 y = texture2D( sam, p.zx, lodbias );
	vec4 z = texture2D( sam, p.xy, lodbias );
	return x*abs(n.x) + y*abs(n.y) + z*abs(n.z);
}

//=====================================================================

float lava( vec2 p )
{
	p += vec2(2.0,4.0);
    float f;
    f  = 0.5000*noise( p ); p = p*2.02;
    f += 0.2500*noise( p ); p = p*2.03;
    f += 0.1250*noise( p ); p = p*2.01;
    f += 0.0625*noise( p );
    return f;
}

const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float displacement( vec3 p )
{
	p += vec3(1.0,0.0,0.8);
	
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p ); 
	
	float n = noise( p*3.5 );
    f += 0.03*n*n;
	
    return f;
}

float mapTerrain( in vec3 pos )
{
	return pos.y*0.1 + (displacement(pos*vec3(0.8,1.0,0.8)) - 0.4)*(1.0-smoothstep(1.0,3.0,pos.y));
}

float raymarchTerrain( in vec3 ro, in vec3 rd )
{
	float maxd = 30.0;
	float precis = 0.001;
    float h = 1.0;
    float t = 0.1;
    for( int i=0; i<160; i++ )
    {
        if( abs(h)<precis||t>maxd ) continue;//break;
	    h = mapTerrain( ro+rd*t );
        t += h;
    }

    if( t>maxd ) t=-1.0;
    return t;
}

vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3(0.02,0.0,0.0);
	return normalize( vec3(
           mapTerrain(pos+eps.xyy) - mapTerrain(pos-eps.xyy),
           mapTerrain(pos+eps.yxy) - mapTerrain(pos-eps.yxy),
           mapTerrain(pos+eps.yyx) - mapTerrain(pos-eps.yyx) ) );

}

vec3 lig = normalize( vec3(-0.3,0.4,0.7) );
	
vec4 mapClouds( in vec3 pos )
{
	vec3 q = pos*0.5 + vec3(0.0,-iGlobalTime,0.0);
	
	float d;
    d  = 0.5000*noise( q ); q = q*2.02;
    d += 0.2500*noise( q ); q = q*2.03;
    d += 0.1250*noise( q ); q = q*2.01;
    d += 0.0625*noise( q );
		
	d = d - 0.55;
	d *= smoothstep( 0.5, 0.55, lava(0.1*pos.xz)+0.01 );

	d = clamp( d, 0.0, 1.0 );
	
	vec4 res = vec4( d );

	res.xyz = mix( vec3(1.0,0.8,0.7), 0.2*vec3(0.4,0.4,0.4), res.x );
	res.xyz *= 0.25;
	res.xyz *= 0.5 + 0.5*smoothstep( -2.0, 1.0, pos.y );
	
	return res;
}

vec4 raymarchClouds( in vec3 ro, in vec3 rd, in vec3 bcol, float tmax )
{
	vec4 sum = vec4( 0.0 );

	float sun = pow( clamp( dot(rd,lig), 0.0, 1.0 ),6.0 );
	float t = 0.0;
	for( int i=0; i<60; i++ )
	{
		if( t>tmax || sum.w>0.95 ) continue;
		vec3 pos = ro + t*rd;
		vec4 col = mapClouds( pos );
		
        col.xyz += vec3(1.0,0.7,0.4)*0.4*sun*(1.0-col.w);
		col.xyz = mix( col.xyz, bcol, 1.0-exp(-0.00006*t*t*t) );
		
		col.rgb *= col.a;

		sum = sum + col*(1.0 - sum.a);	

		t += max(0.1,0.05*t);
	}

	sum.xyz /= (0.001+sum.w);

	return clamp( sum, 0.0, 1.0 );
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<48; i++ )
    {
        float h = mapTerrain(ro + rd*t);
		h = max( h, 0.0 );
        res = min( res, k*h/t );
        t += clamp( h, 0.02, 0.5 );
    }
    return clamp(res,0.0,1.0);
}

vec3 path( float time )
{
	return vec3( 16.0*cos(0.2+0.5*.1*time*1.5), 1.5, 16.0*sin(0.1+0.5*0.11*time*1.5) );
	
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{

	
	
	
    // sky	 
	vec3 col = vec3(0.32,0.36,0.4) - rd.y*0.4;
    float sun = clamp( dot(rd,lig), 0.0, 1.0 );
	col += vec3(1.0,0.8,0.4)*0.2*pow( sun, 6.0 );
    col *= 0.9;

	vec3 bcol = col;
    

    // terrain	
	float t = raymarchTerrain(ro, rd);
    if( t>0.0 )
	{
		vec3 pos = ro + t*rd;
		vec3 nor = calcNormal( pos );
		vec3 ref = reflect( rd, nor );

		vec3 bn = -1.0 + 2.0*texcube( iChannel0, 3.0*pos/4.0, nor ).xyz;
		nor = normalize( nor + 0.6*bn );
		
		float hh = 1.0 - smoothstep( -2.0, 1.0, pos.y );

        // lighting
		float sun = clamp( dot( nor, lig ), 0.0, 1.0 );
		float sha = 0.0; if( sun>0.01) sha=softshadow(pos,lig,0.01,32.0);
		float bac = clamp( dot( nor, normalize(lig*vec3(-1.0,0.0,-1.0)) ), 0.0, 1.0 );
		float sky = 0.5 + 0.5*nor.y;
        float lav = smoothstep( 0.5, 0.55, lava(0.1*pos.xz) )*hh*clamp(0.5-0.5*nor.y,0.0,1.0);
		float occ = pow( (1.0-displacement(pos*vec3(0.8,1.0,0.8)))*1.6-0.5, 2.0 );

		float amb = 1.0;

		col = vec3(0.8);

		vec3 lin  = sun*vec3(1.64,1.27,0.99)*pow(vec3(sha),vec3(1.0,1.2,1.5))*(0.8+0.2*occ);
		     lin += sky*vec3(0.16,0.20,0.28)*occ;
		     lin += bac*vec3(0.40,0.28,0.20)*occ;
		     lin += amb*vec3(0.18,0.15,0.15)*occ;
		     lin += lav*vec3(3.00,0.61,0.00);


        // surface shading/material		
		col = texcube( iChannel1, 0.5*pos, nor ).xyz;
		col = col*(0.2+0.8*texcube( iChannel2, 4.0*vec3(2.0,8.0,2.0)*pos, nor ).x);
		vec3 verde = vec3(1.0,0.9,0.2);
		verde *= texture2D( iChannel2, pos.xz, lodbias ).xyz;
		col = mix( col, 0.8*verde, hh );
		
		float vv = smoothstep( 0.0, 0.8, nor.y )*smoothstep(0.0, 0.1, pos.y-0.8 );
		verde = vec3(0.2,0.45,0.1);
		verde *= texture2D( iChannel2, 30.0*pos.xz, lodbias ).xyz;
		verde += 0.2*texture2D( iChannel2, 1.0*pos.xz, lodbias ).xyz;
		vv *= smoothstep( 0.0, 0.5, texture2D( iChannel2, 0.1*pos.xz + 0.01*nor.x ).x );
		col = mix( col, verde*1.1, vv );
		
        // light/surface interaction		
		col = lin * col;
		
		// atmospheric
		col = mix( col, (1.0-0.7*hh)*bcol, 1.0-exp(-0.00006*t*t*t) );
	}

	// sun glow
    col += vec3(1.0,0.6,0.2)*0.23*pow( sun, 2.0 )*clamp( (rd.y+0.4)/(0.0+0.4),0.0,1.0);
	
    // smoke	
	{
	if( t<0.0 ) t=600.0;
    vec4 res = raymarchClouds( ro, rd, bcol, t );
	col = mix( col, res.xyz, res.w );
	}

    // gamma	
	col = pow( clamp( col, 0.0, 1.0 ), vec3(0.45) );

    // contrast, desat, tint and vignetting	
	col = col*0.3 + 0.7*col*col*(3.0-2.0*col);
	col = mix( col, vec3(col.x+col.y+col.z)*0.33, 0.2 );
	col *= 1.3*vec3(1.06,1.1,1.0);
	//col *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );

	return col;
}


#ifndef RIFTRAY
void main( void )
{
	#ifdef STEREO
	float eyeID = mod(gl_FragCoord.x + mod(gl_FragCoord.y,2.0),2.0);
    #endif

    vec2 q = gl_FragCoord.xy / iResolution.xy;
	vec2 p = -1.0 + 2.0*q;
	p.x *= iResolution.x / iResolution.y;
	
	
    // camera	
	float off = step( 0.001, iMouse.z )*6.0*iMouse.x/iResolution.x;
	float time = 2.7+iGlobalTime + off;
	vec3 ro = path( time+0.0 );
	vec3 ta = path( time+1.6 );
	//ta.y *= 0.3 + 0.25*cos(0.11*time);
	ta.y *= 0.35 + 0.25*sin(0.09*time);
	float roll = 0.3*sin(1.0+0.07*time);
	
	// camera tx
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(roll), cos(roll),0.0);
	vec3 cu = normalize(cross(cw,cp));
	vec3 cv = normalize(cross(cu,cw));
	
	float r2 = p.x*p.x*0.32 + p.y*p.y;
    p *= (7.0-sqrt(37.5-11.5*r2))/(r2+1.0);

	vec3 rd = normalize( p.x*cu + p.y*cv + 2.1*cw );

	#ifdef STEREO
	vec3 fo = ro + rd*7.0; // put focus plane behind Mike
	ro -= 0.2*cu*eyeID;    // eye separation
	rd = normalize(fo-ro);
    #endif


	
	vec3 col = getSceneColor( ro, rd );
	
    #ifdef STEREO	
    col *= vec3( eyeID, 1.0-eyeID, 1.0-eyeID );	
	#endif
	
	
	gl_FragColor = vec4( col, 1.0 );
}
#endif
