// https://www.shadertoy.com/view/MdBGzG
// Canyon
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// @var title Canyon
// @var author iq
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/MdBGzG

// @var headSize 2.0

// @var tex0 tex09.jpg
// @var tex1 tex07.jpg
// @var tex2 tex16.png
// @var tex3 tex06.jpg
//-----------------------------------------------------------------------------------

#define LOWDETAIL
//#define HIGH_QUALITY_NOISE

float noise1( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
#ifndef HIGH_QUALITY_NOISE
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel2, (uv+ 0.5)/256.0, -100.0 ).yx;
#else
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z);
	vec2 rg1 = texture2D( iChannel2, (uv+ vec2(0.5,0.5))/256.0, -100.0 ).yx;
	vec2 rg2 = texture2D( iChannel2, (uv+ vec2(1.5,0.5))/256.0, -100.0 ).yx;
	vec2 rg3 = texture2D( iChannel2, (uv+ vec2(0.5,1.5))/256.0, -100.0 ).yx;
	vec2 rg4 = texture2D( iChannel2, (uv+ vec2(1.5,1.5))/256.0, -100.0 ).yx;
	vec2 rg = mix( mix(rg1,rg2,f.x), mix(rg3,rg4,f.x), f.y );
#endif	
	return mix( rg.x, rg.y, f.z );
}

//-----------------------------------------------------------------------------------
const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float displacement( vec3 p )
{
    float f;
    f  = 0.5000*noise1( p ); p = m*p*2.02;
    f += 0.2500*noise1( p ); p = m*p*2.03;
    f += 0.1250*noise1( p ); p = m*p*2.01;
	#ifndef LOWDETAIL
    f += 0.0625*noise1( p ); 
	#endif
    return f;
}

vec4 texcube( sampler2D sam, in vec3 p, in vec3 n )
{
	vec4 x = texture2D( sam, p.yz );
	vec4 y = texture2D( sam, p.zx );
	vec4 z = texture2D( sam, p.xy );
	return (x*abs(n.x) + y*abs(n.y) + z*abs(n.z))/(abs(n.x)+abs(n.y)+abs(n.z));
}


vec4 texture2DGood( sampler2D sam, vec2 uv, float bias )
{
    uv = uv*1024.0 - 0.5;
    vec2 iuv = floor(uv);
    vec2 f = fract(uv);
	vec4 rg1 = texture2D( sam, (iuv+ vec2(0.5,0.5))/1024.0, bias );
	vec4 rg2 = texture2D( sam, (iuv+ vec2(1.5,0.5))/1024.0, bias );
	vec4 rg3 = texture2D( sam, (iuv+ vec2(0.5,1.5))/1024.0, bias );
	vec4 rg4 = texture2D( sam, (iuv+ vec2(1.5,1.5))/1024.0, bias );
	return mix( mix(rg1,rg2,f.x), mix(rg3,rg4,f.x), f.y );
}
//-----------------------------------------------------------------------------------

float terrain( in vec2 q )
{
	float th = smoothstep( 0.0, 0.7, texture2D( iChannel0, 0.001*q, -100.0 ).x );
    float rr = smoothstep( 0.1, 0.5, texture2D( iChannel1, 2.0*0.03*q, -100.0 ).y );
	float h = 1.9;
	#ifndef LOWDETAIL
	h += (1.0-0.6*rr)*(1.5-1.0*th) * 0.2*(1.0-texture2D( iChannel0, 0.03*q, -100.0 ).x);
	#endif
	h += th*7.0;
    h += 0.3*rr;
    return -h;
}

float terrain2( in vec2 q )
{
	float th = smoothstep( 0.0, 0.7, texture2DGood( iChannel0, 0.001*q, -100.0 ).x );
    float rr = smoothstep( 0.1, 0.5, texture2DGood( iChannel1, 2.0*0.03*q, -100.0 ).y );
	float h = 1.9;
	h += th*7.0;
    return -h;
}


vec4 map( in vec3 p )
{
	float h = terrain( p.xz );
	float dis = displacement( 0.25*p*vec3(1.0,4.0,1.0) );
	dis *= 3.0;
	return vec4( (dis + p.y-h)*0.25, p.x, h, 0.0 );
}

vec4 intersect( in vec3 ro, in vec3 rd )
{
	const float maxd = 120.0;
	const float precis = 0.001;
    float h = precis*3.0;
    float t = 0.0;
    float m = 0.0;
	float l = 0.0;
	float r = 0.0;
    for( int i=0; i<180; i++ )
    {
        //if( h>precis && t<maxd )
		if( h<precis || t>maxd ) break;
		{
        t += h *(1.0+0.007*t);
	    vec4 res = map( ro+rd*t );
        h = res.x;
		l = res.y;
		r = res.z;
        m = res.w;			
		}
    }

    if( t>maxd ) m=-1.0;
    return vec4( t, l, m, r);
}

vec3 calcNormal( in vec3 pos )
{
    const vec3 eps = vec3(0.01,0.0,0.0);
	return normalize( vec3(
           map(pos+eps.xyy).x - map(pos-eps.xyy).x,
           map(pos+eps.yxy).x - map(pos-eps.yxy).x,
           map(pos+eps.yyx).x - map(pos-eps.yyx).x ) );
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
	float h = 1.0;
    for( int i=0; i<64; i++ )
    {
        h = map(ro + rd*t).x;
        res = min( res, k*h/t );
		t += clamp( h, 0.3, 1.0 );
    }
    return clamp(res,0.0,1.0);
}

// Oren-Nayar
float Diffuse( in vec3 l, in vec3 n, in vec3 v, float r )
{
	
    float r2 = r*r;
    float a = 1.0 - 0.5*(r2/(r2+0.57));
    float b = 0.45*(r2/(r2+0.09));

    float nl = dot(n, l);
    float nv = dot(n, v);

    float ga = dot(v-n*nv,n-n*nl);

	return max(0.0,nl) * (a + b*max(0.0,ga) * sqrt((1.0-nv*nv)*(1.0-nl*nl)) / max(nl, nv));
}

vec3 cpath( float t )
{
	vec3 pos = vec3( 0.0, 0.0, 95.0 + t );
	
	float a = smoothstep(5.0,20.0,t);
	pos.xz += a*150.0 * cos( vec2(5.0,6.0) + 1.0*0.01*t );
	pos.xz -= a*150.0 * cos( vec2(5.0,6.0) );
	pos.xz += a* 50.0 * cos( vec2(0.0,3.5) + 6.0*0.01*t );
	pos.xz -= a* 50.0 * cos( vec2(0.0,3.5) );

	return pos;
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    //-----------------------------------------------------
	// render
    //-----------------------------------------------------

	vec3 klig = normalize(vec3(-1.0,0.19,0.4));

	float sun = clamp(dot(klig,rd),0.0,1.0 );

	vec3 hor = mix( 1.2*vec3(0.70,1.0,1.0), vec3(1.5,0.5,0.05), 0.25+0.75*sun );
	
    vec3 col = mix( vec3(0.2,0.6,.9), hor, exp(-(4.0+2.0*(1.0-sun))*max(0.0,rd.y-0.1)) );
    col *= 0.5;
	col += 0.8*vec3(1.0,0.8,0.7)*pow(sun,512.0);
	col += 0.2*vec3(1.0,0.4,0.2)*pow(sun,32.0);
	col += 0.1*vec3(1.0,0.4,0.2)*pow(sun,4.0);
	
	vec3 bcol = col;
	
	
	
	// clouds
	float pt = (1000.0-ro.y)/rd.y; 
	if( pt>0.0 )
	{
        vec3 spos = ro + pt*rd;
        float clo = texture2D( iChannel0, 0.00006*spos.xz ).x;	
        vec3 cloCol = mix( vec3(0.4,0.5,0.6), vec3(1.3,0.6,0.4), pow(sun,2.0))*(0.5+0.5*clo);
        col = mix( col, cloCol, 0.5*smoothstep( 0.4, 1.0, clo ) );
	}
	
	
	// raymarch
    vec4 tmat = intersect(ro,rd);
    if( tmat.z>-0.5 )
    {
        // geometry
        vec3 pos = ro + tmat.x*rd;
        vec3 nor = calcNormal(pos);
		vec3 ref = reflect( rd, nor );


		float occ = smoothstep( 0.0, 1.5, pos.y + 11.5 ) * (1.0 - displacement( 0.25*pos*vec3(1.0,4.0,1.0) ));

		// materials
		vec4 mate = vec4(0.5,0.5,0.5,0.0);
		
		//if( tmat.z<0.5 )
		{
			vec3 uvw = 1.0*pos;

			vec3 bnor;
			float be = 1.0/1024.0;
			float bf = 0.4;
			bnor.x = texcube( iChannel0, bf*uvw+vec3(be,0.0,0.0), nor ).x - texcube( iChannel0, bf*uvw-vec3(be,0.0,0.0), nor ).x;
			bnor.y = texcube( iChannel0, bf*uvw+vec3(0.0,be,0.0), nor ).x - texcube( iChannel0, bf*uvw-vec3(0.0,be,0.0), nor ).x;
			bnor.z = texcube( iChannel0, bf*uvw+vec3(0.0,0.0,be), nor ).x - texcube( iChannel0, bf*uvw-vec3(0.0,0.0,be), nor ).x;
			bnor = normalize(bnor);
			float amo = 0.2  + 0.25*(1.0-smoothstep(0.6,0.7,nor.y) );
			nor = normalize( nor + amo*(bnor-nor*dot(bnor,nor)) );

			vec3 te = texcube( iChannel0, 0.15*uvw, nor ).xyz;
			te = 0.05 + te;
			mate.xyz = 0.6*te;
			mate.w = 1.5*(0.5+0.5*te.x);
			float th = smoothstep( 0.1, 0.4, texcube( iChannel0, 0.002*uvw, nor ).x );
			vec3 dcol = mix( vec3(0.2, 0.3, 0.0), 0.4*vec3(0.65, 0.4, 0.2), 0.2+0.8*th );
			mate.xyz = mix( mate.xyz, 2.0*dcol, th*smoothstep( 0.0, 1.0, nor.y ) );
			mate.xyz *= 0.5;
			float rr = smoothstep( 0.2, 0.4, texcube( iChannel1, 2.0*0.02*uvw, nor ).y );
			mate.xyz *= mix( vec3(1.0), 1.5*vec3(0.25,0.24,0.22)*1.5, rr );
			mate.xyz *= 1.5*pow(texcube( iChannel3, 8.0*uvw, nor ).xyz,vec3(0.5));
            mate = mix( mate, vec4(0.7,0.7,0.7,.0), smoothstep(0.8,0.9,nor.y + nor.x*0.6*te.x*te.x ));
			
			
			mate.xyz *= 1.5;
		}
		
		vec3 blig = normalize(vec3(-klig.x,0.0,-klig.z));
		vec3 slig = vec3( 0.0, 1.0, 0.0 );
			
		// lighting
        float sky = 0.0;
        sky += 0.2*Diffuse( normalize(vec3( 0.0, 1.0, 0.0 )), nor, -rd, 1.0 );
        sky += 0.2*Diffuse( normalize(vec3( 3.0, 1.0, 0.0 )), nor, -rd, 1.0 );
        sky += 0.2*Diffuse( normalize(vec3(-3.0, 1.0, 0.0 )), nor, -rd, 1.0 );
        sky += 0.2*Diffuse( normalize(vec3( 0.0, 1.0, 3.0 )), nor, -rd, 1.0 );
        sky += 0.2*Diffuse( normalize(vec3( 0.0, 1.0,-3.0 )), nor, -rd, 1.0 );
		float dif = Diffuse( klig, nor, -rd, 1.0 );
		float bac = Diffuse( blig, nor, -rd, 1.0 );


		float sha = 0.0; if( dif>0.001 ) sha=softshadow( pos+0.01*nor, klig, 0.005, 64.0 );
        float spe = mate.w*pow( clamp(dot(reflect(rd,nor),klig),0.0,1.0),2.0)*clamp(dot(nor,klig),0.0,1.0);
		
		// lights
		vec3 lin = vec3(0.0);
		lin += 7.0*dif*vec3(1.4,0.5,0.25)*vec3(sha,sha*0.5+0.5*sha*sha, sha*sha );
		lin += 1.0*sky*vec3(0.10,0.5,0.70)*occ;
		lin += 2.0*bac*vec3(0.30,0.15,0.15)*occ;
	    lin += 0.5*vec3(spe)*sha*occ;
		
		// surface-light interacion
		col = mate.xyz * lin;

		// fog
        bcol = 0.7*mix( vec3(0.2,0.5,1.0)*0.82, bcol, 0.15+0.8*sun ); col = mix( col, bcol, 1.0-exp(-0.02*tmat.x) );		
	}
	

	col += 0.15*vec3(1.0,0.9,0.6)*pow( sun, 6.0 );
	
#if 0
	//-----------------------------------------------------
	// postprocessing
    //-----------------------------------------------------

    col *= 1.0 - 0.25*pow(1.0-clamp(dot(ww,klig),0.0,1.0),3.0);
	
	col = pow( clamp(col,0.0,1.0), vec3(0.45) );

	col *= vec3(1.1,1.0,1.0);
	col = col*col*(3.0-2.0*col);
	col = pow( col, vec3(0.9,1.0,1.0) );

	col = mix( col, vec3(dot(col,vec3(0.333))), 0.4 );
	col = col*0.3+0.7*col*col*(3.0-2.0*col);
	
	col *= 0.3 + 0.7*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );

    col *= smoothstep(0.0,2.5,iGlobalTime);
#endif

	return col;
}

#ifndef RIFTRAY
void main( void )
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 p = -1.0 + 2.0 * q;
    p.x *= iResolution.x/iResolution.y;
    vec2 m = vec2(0.5);
	if( iMouse.z>0.0 ) m = iMouse.xy/iResolution.xy;

	
    //-----------------------------------------------------
    // camera
    //-----------------------------------------------------

	float an = 1.0*(iGlobalTime-5.0) + 12.0*(m.x-0.5);
	vec3 ro = cpath( an + 0.0 );
	vec3 ta = cpath( an + 10.0 *1.0);
	ta = mix( ro + vec3(0.0,0.0,1.0), ta, smoothstep(5.0,25.0,an) );
    ro.y = terrain2( ro.xz ) - 0.5;
	ta.y = ro.y - 0.1;

    // camera matrix/ray
	float rl = -0.1*cos(0.05*6.2831*an);
    vec3 ww = normalize( ta - ro );
    vec3 uu = normalize( cross(ww,vec3(sin(rl),cos(rl),0.0) ) );
    vec3 vv = normalize( cross(uu,ww));
	vec3 rd = normalize( p.x*uu + p.y*vv + 2.0*ww );

	vec3 col = getSceneColor( ro, rd );
	gl_FragColor = vec4( col, 1.0 );
}
#endif
