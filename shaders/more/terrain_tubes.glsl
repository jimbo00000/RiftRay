// Created by inigo quilez - iq/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// @var title Terrain Tubes
// @var author iq
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/4sjXzG
// @var headSize 4.0
// @var eyePos 0.0 112.0 0.0
// @var tex0 tex03.jpg
// @var tex1 tex16.png
// @var tex2 tex09.jpg

// @var float sep 400. 100. 1000. 2.
uniform float sep;
// @var vec3 light1 -0.8 0.2 0.5 dir
uniform vec3 light1;
//float sep = 400.0;
//vec3 light1 = normalize( vec3(-0.8,0.2,0.5) );

#define USE_COSINE

const mat2 m2 = mat2(1.6,-1.2,1.2,1.6);

float tri( in vec2 p )
{
#ifdef USE_COSINE
    return 0.5*(cos(6.2831*p.x) + cos(6.2831*p.y));
#else
    vec2 q = 2.0*abs(fract(p)-0.5);
    q = q*q*(3.0-2.0*q);
    return -1.0 + q.x + q.y;
#endif    
}

float terrainLow( vec2 p )
{
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<2; i++ )
	{
        t += s*tri( p );
		s *= 0.45 + 0.15*t;
        p = 0.97*m2*p + (t-0.5)*0.2;
	}
	return t*55.0;
}

float terrainMed( vec2 p )
{
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<6; i++ )
	{
        t += s*tri( p );
		s *= 0.5 + 0.1*t;
        p = 0.97*m2*p + (t-0.5)*0.2;
	}
            
    return t*55.0;
}

float terrainHigh( vec2 p )
{
    vec2 q = p;
    p *= 0.0013;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<8; i++ )
	{
        t += s*tri( p );
		s *= 0.5 + 0.1*t;
        p = 0.97*m2*p + (t-0.5)*0.2;
	}
    
    t += 0.03*texture2D( iChannel0, 0.005*q ).x;
    t += t*0.03*texture2D( iChannel0, 0.02*q ).x;

	return t*55.0;
}

float tubes( in vec3 pos, float time )
{
    vec3 qos = mod( pos + sep*0.5, sep ) - sep*0.5; 
    qos.y = pos.y - 70.0;
    qos.x += sep*0.3*cos( 0.01*pos.z);
    qos.y += sep*0.1*cos( 0.01*pos.x );

    float sph = length( qos.xy ) - sep*0.012;

    sph -= sep*0.007*(-0.2+texture2D( iChannel0, 0.003*pos.xy*vec2(0.5,1.5) ).y);
    
    return sph;
}


vec2 map( in vec3 pos, float time )
{
    float m = 0.0;
	float h = pos.y - terrainMed(pos.xz);

#if 1
    float sph = tubes( pos, time );
    float k = 60.0;
    float w = clamp( 0.5 + 0.5*(h-sph)/k, 0.0, 1.0 );
    h = mix( h, sph, w ) - k*w*(1.0-w);
    m = mix( m, 1.0, w ) - 1.0*w*(1.0-w);
    m = clamp(m,0.0,1.0);
#endif    
    return vec2( h, m );
}

float mapH( in vec3 pos, in float time )
{
    float y = terrainHigh(pos.xz);
        
    float h = pos.y - y;
    
#if 1
    float sph = tubes( pos, time );
    float k = 60.0;
    float w = clamp( 0.5 + 0.5*(h-sph)/k, 0.0, 1.0 );
    h = mix( h, sph, w ) - k*w*(1.0-w);
#endif    

    return h;
}


vec2 interesct( in vec3 ro, in vec3 rd, in float tmin, in float tmax, in float time )
{
    float t = tmin;
    float  m = 0.0;
	for( int i=0; i<200; i++ )
	{
        vec3 pos = ro + t*rd;
        
        vec2 res = map( pos, time );
        m = res.y;
        
        if( res.x<(0.001*t) || t>tmax  ) break;
        t += res.x * 0.5;
	}

	return vec2( t, m );
}

float calcShadow(in vec3 ro, in vec3 rd )
{
	vec2  eps = vec2(150.0,0.0);
    float h1 = terrainMed( ro.xz );
    float h2 = terrainLow( ro.xz );
    
    float d1 = 10.0;
    float d2 = 80.0;
    float d3 = 200.0;
    float s1 = clamp( 1.0*(h1 + rd.y*d1 - terrainMed(ro.xz + d1*rd.xz)), 0.0, 1.0 );
    float s2 = clamp( 0.5*(h1 + rd.y*d2 - terrainMed(ro.xz + d2*rd.xz)), 0.0, 1.0 );
    float s3 = clamp( 0.2*(h2 + rd.y*d3 - terrainLow(ro.xz + d3*rd.xz)), 0.0, 1.0 );

    return min(min(s1,s2),s3);
}

vec3 calcNormalHigh( in vec3 pos, float t, in float time )
{
    vec2 e = vec2(1.0,-1.0)*0.001*t;

    return normalize( e.xyy*mapH( pos + e.xyy, time ) + 
					  e.yyx*mapH( pos + e.yyx, time ) + 
					  e.yxy*mapH( pos + e.yxy, time ) + 
					  e.xxx*mapH( pos + e.xxx, time ) );
}


vec3 calcNormalMed( in vec3 pos, float t )
{
	float e = 0.005*t;
    vec2  eps = vec2(e,0.0);
    float h = terrainMed( pos.xz );
    return normalize(vec3( terrainMed(pos.xz-eps.xy)-h, e, terrainMed(pos.xz-eps.yx)-h ));
}

vec3 camPath( float time )
{
    vec2 p = 1100.0*vec2( cos(0.0+0.23*time), cos(1.5+0.205*time) );
	return vec3( p.x, 0.0, p.y );
}

vec3 dome( in vec3 rd, in vec3 light1 )
{
	float sundotc = clamp(dot(rd,light1),0.0,1.0);
	float sundota = clamp(0.5 + 0.5*dot(rd,light1),0.0,1.0);

    float cho = max(rd.y,0.0);
    vec3 bgcol = mix( mix(vec3(0.00,0.40,0.60)*0.7, 
                          vec3(0.80,0.70,0.20), pow(1.0-cho,3.0 + 4.0-4.0*sundota)), 
                          //vec3(0.63,0.30,0.14), pow(1.0-cho,10.0+ 18.0-18.0*sundota) );
                          vec3(0.43+0.2*sundota,0.4-0.1*sundota,0.4-0.25*sundota), pow(1.0-cho,10.0+ 8.0-8.0*sundota) );
    bgcol *= 0.8+0.2*sundota;
    return bgcol*0.75;
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    float camid = floor((0.0+iGlobalTime-0.0)/9.0);

    float time = 16.5 + (0.0+iGlobalTime-0.0)*0.25 + 70.5*camid;
    
    float tmin = 10.0;
    float tmax = 5000.0;
#if 0
    // intersct boundg plane
    float maxh = 130.0;
    float tp = (maxh-ro.y)/rd.y;
    if( tp>0.0 )
    {
        if( ro.y>maxh ) tmin = max( tmin, tp );
        else            tmax = min( tmax, tp );
    }
#endif
    
	float sundotc = clamp(dot(rd,light1),0.0,1.0);
    
    // background    
    vec3 bgcol = dome( rd, light1 );

    vec3 col = bgcol;
    
    vec2 res = interesct( ro, rd, tmin, tmax, time );
    float t = res.x;
    if( t>tmax )
    {
        // sky		
		col += 0.2*0.12*vec3(1.0,0.5,0.1)*pow( sundotc,5.0 );
		col += 0.2*0.12*vec3(1.0,0.6,0.1)*pow( sundotc,64.0 );
		col += 0.2*0.12*vec3(2.0,0.4,0.1)*pow( sundotc,512.0 );

        // clouds
  		vec2 sc = ro.xz + rd.xz*(1000.0-ro.y)/rd.y;
		col = mix( col, 0.25*vec3(0.5,0.9,1.0), 0.4*smoothstep(0.0,1.0,texture2D(iChannel0,0.000005*sc).x) );

        // sun scatter
        col += 0.2*0.2*vec3(1.5,0.7,0.4)*pow( sundotc, 4.0 );//*(1.0-exp(-0.003*t));
    }
	else
	{
        // mountains		
		vec3 pos = ro + t*rd;
        vec3 nor = calcNormalHigh( pos, t, time );
        vec3 sor = calcNormalMed( pos, t );
        vec3 ref = reflect( rd, nor );

        // rock
        col = vec3(0.07,0.06,0.05);
        col *= 0.2 + sqrt( texture2D( iChannel2, 0.1*pos.xy*vec2(0.5,1.0) ).x *
                           texture2D( iChannel2, 0.1*pos.zy*vec2(0.5,1.0) ).x );
        
        vec3 col2 = vec3(1.0,0.2,0.1)*0.01;
        col = mix( col, col2, 0.5*res.y );
        
        // grass
        float s = smoothstep(0.6,0.7,nor.y - 0.01*(pos.y-20.0));        
        s *= smoothstep( 0.15,0.2,0.01*nor.x+texture2D(iChannel0, 0.001*pos.zx).x);
        //col = mix( col, 0.12*vec3(0.25,0.3,0.01)*1.0, s );
        col = mix( col, 0.13*vec3(0.20,0.20,0.04)*1.0, s );
        //nor = mix( nor, sor, 0.14*s );
        vec3 ptnor = nor;

        // trees
        s = smoothstep(0.9,0.95,nor.y - 0.01*(pos.y-20.0));        
        s *= smoothstep( 0.1,0.13,-0.17+texture2D(iChannel0, 0.001*pos.zx).x);
        vec3 tor = -1.0 + 2.0*texture2D( iChannel1, 0.015*pos.xz ).xyz;
        tor.y = 1.5;
        tor = normalize(tor);
        //col = mix( col, 0.11*vec3(0.24,0.28,0.02)*1.0, s );
        col = mix( col, 0.13*vec3(0.22,0.25,0.02)*1.0, s );
        nor = mix( nor, tor, 0.7*s );
        
		// snow
        s = ptnor.y + 0.008*pos.y - 0.2 + 0.2*(texture2D(iChannel1,0.00015*pos.xz+0.0*sor.y).x-0.5);
        float sf = fwidth(s) * 1.5;
        s = smoothstep(0.84-sf, 0.84+sf, s );
        col = mix( col, 0.15*vec3(0.43,0.6,0.8), s );
        nor = mix( nor, sor, 0.5*smoothstep(0.9, 0.95, s ) );

        
        // lighting		
        float amb = clamp( nor.y,0.0,1.0);
		float dif = clamp( dot( light1, nor ), 0.0, 1.0 );
		float bac = clamp( dot( normalize( vec3(-light1.x, 0.0, light1.z ) ), nor ), 0.0, 1.0 );
		float sha = mix( calcShadow( pos, light1 ), 1.0, res.y );
        float spe = pow( clamp( dot(ref,light1), 0.0, 1.0 ), 8.0 ) * dif;
		
		vec3 lin  = vec3(0.0);
        lin += dif*vec3(11.0,6.00,3.00)*vec3( sha, sha*sha*0.5+0.5*sha, sha*sha*0.8+0.2*sha );
		lin += amb*vec3(0.25,0.30,0.40);
        lin += bac*vec3(0.35,0.40,0.50);
        lin += res.y*spe*2.0;
        
        col *= lin;
          
		// fog
        col = mix( col, 0.25*mix(vec3(0.4,0.75,1.0),vec3(0.3,0.3,0.3), sundotc*sundotc), 1.0-exp(-0.0000008*t*t) );

        // sun scatter
        col += 0.15*vec3(1.0,0.8,0.3)*pow( sundotc, 8.0 )*(1.0-exp(-0.003*t));

        col = mix( col, bgcol, 1.0-exp(-0.00000004*t*t) );
    }
    
    // gamma
	col = pow( col, vec3(0.45) );
        
    // color grading    
    col = col*1.4*vec3(1.0,1.0,1.0) + vec3(0.0,0.0,0.15);
    col = col*0.1 + 0.9*col*col*(3.0-2.0*col);
    col = mix( col, vec3(dot(col,vec3(0.333))), 0.25 );
    return col;
}

#ifndef RIFTRAY
void main( void )
{
    vec2 xy = -1.0 + 2.0*gl_FragCoord.xy/iResolution.xy;

	vec2 s = xy*vec2(iResolution.x/iResolution.y,1.0);

    float camid = floor((0.0+iGlobalTime-0.0)/9.0);

    float time = 16.5 + (0.0+iGlobalTime-0.0)*0.25 + 20.0*iMouse.x/iResolution.x + 70.5*camid;
	
    

	vec3 ro = camPath( time + 0.0 );
	vec3 ta = camPath( time + 3.0 );

    ro.y = terrainLow( ro.xz ) + 60.0 + 30.0*sin(1.0*(time-14.4));
	ta.y = ro.y - 200.0;
    
	float cr = 0.18*sin(-0.1*time);
	vec3  cw = normalize(ta-ro);
	vec3  cp = vec3(sin(cr), cos(cr),0.0);
	vec3  cu = normalize( cross(cw,cp) );
	vec3  cv = normalize( cross(cu,cw) );
	vec3  rd = normalize( s.x*cu + s.y*cv + 1.35*cw );
    
    vec3 col = getSceneColor(ro, rd);
    
    // vignetting	
 	col *= 0.5 + 0.5*pow( (xy.x+1.0)*(xy.y+1.0)*(xy.x-1.0)*(xy.y-1.0), 0.1 );
 
    //col *= smoothstep( 0.0, 5.0, iGlobalTime );
    col *= smoothstep( 0.0, 0.1, 2.0*abs(fract(0.5+iGlobalTime/9.0)-0.5) );
   
	gl_FragColor = vec4( col, 1.0 );
}
#endif
