// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// @var author iq
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/4sX3Rn

// @var headSize 7.0

float maxcomp(in vec3 p ) { return max(p.x,max(p.y,p.z));}
float sdBox( vec3 p, vec3 b )
{
  vec3  di = abs(p) - b;
  float mc = maxcomp(di);
  return min(mc,length(max(di,0.0)));
}

mat3 ma = mat3( 0.60, 0.00,  0.80,
                0.00, 1.00,  0.00,
               -0.80, 0.00,  0.60 );

vec4 map( in vec3 p )
{
#ifdef RIFTRAY
	p = vec4( obmtx * vec4(p,1.0) ).xyz;
#endif

   float d = sdBox(p,vec3(1.0));
   vec4 res = vec4( d, 1.0, 0.0, 0.0 );

   float s = 1.0;
   for( int m=0; m<4; m++ )
   {

      p += 1.5*sin( 15.0*float(m)+0.0*iGlobalTime );
      p = ma*p;	   
	   
      vec3 a = mod( p*s, 2.0 )-1.0;
      s *= 3.0;
      vec3 r = abs(1.0 - 3.0*abs(a));
      float da = max(r.x,r.y);
      float db = max(r.y,r.z);
      float dc = max(r.z,r.x);
      float c = (min(da,min(db,dc))-1.0)/s;

      if( c>d )
      {
          d = c;
          res = vec4( d, min(res.y,0.2*da*db*dc), (1.0+float(m))/4.0, 0.0 );
       }
   }

   return res;
}

// GLSL ES doesn't seem to like loops with conditional break/return...
#if 0
vec4 intersect( in vec3 ro, in vec3 rd )
{
    float t = 0.0;
    for(int i=0;i<64;i++)
    {
        vec4 h = map(ro + rd*t);
        if( h.x<0.002 )
            return vec4(t,h.yzw);
        t += h.x;
    }
    return vec4(-1.0);
}
#else
vec4 intersect( in vec3 ro, in vec3 rd )
{
    float t = 0.0;
    vec4 res = vec4(-1.0);
    for(int i=0;i<64;i++)
    {
        vec4 h = map(ro + rd*t);
        if( h.x<0.002 )
        {
            if( res.x<0.0 ) res = vec4(t,h.yzw);
        }
//if( h.x>0.0 )
        t += h.x;
    }
    return res;
}
#endif

vec3 calcNormal(in vec3 pos)
{
    vec3  eps = vec3(.001,0.0,0.0);
    vec3 nor;
    nor.x = map(pos+eps.xyy).x - map(pos-eps.xyy).x;
    nor.y = map(pos+eps.yxy).x - map(pos-eps.yxy).x;
    nor.z = map(pos+eps.yyx).x - map(pos-eps.yyx).x;
    return normalize(nor);
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    // light
    vec3 light = normalize(vec3(1.0,0.9,0.3));
    vec3 col = vec3(0.0);
	
	col = mix( vec3(0.3,0.25,0.2), vec3(0.8, 0.9, 1.0), 0.5 + 0.5*rd.y );
	
    vec4 tmat = intersect(ro,rd);
    if( tmat.x>0.0 )
    {
        vec3 pos = ro + tmat.x*rd;
        vec3 nor = calcNormal(pos);

        float dif1 = max(0.4 + 0.6*dot(nor,light),0.0);
        float dif2 = max(0.4 + 0.6*dot(nor,vec3(-light.x,light.y,-light.z)),0.0);

        // shadow
        float ldis = 4.0;
        vec4 shadow = intersect( pos + light*ldis, -light );
        if( shadow.x>0.01 && shadow.x<(ldis-0.01) ) dif1=0.0;

        float ao = tmat.y;
        col  = 0.60*ao*vec3(0.15,0.17,0.2);
        col += 2.00*(0.7+0.3*ao)*dif1*vec3(1.0,0.97,0.85);
        col += 0.15*(0.5+0.5*ao)*dif2*vec3(1.0,0.97,0.85);
        col += 0.40*(0.5+0.5*ao)*(0.5+0.5*nor.y)*vec3(0.1,0.15,0.2);


        vec3 matcol = vec3(
            0.6+0.4*cos(5.0+6.2831*tmat.z),
            0.6+0.4*cos(5.4+6.2831*tmat.z),
            0.6+0.4*cos(5.7+6.2831*tmat.z) );
        col *= matcol;
        col *= 1.5*exp(-0.5*tmat.x);
    }

    col = sqrt(col);
	return col;
}

#ifndef RIFTRAY
void main(void)
{
    vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
    p.x *= 1.33;

    // light
    vec3 light = normalize(vec3(1.0,0.9,0.3));

    float ctime = iGlobalTime;
    // camera
    vec3 ro = 1.1*vec3(2.5*sin(0.25*ctime),1.0+1.0*cos(ctime*.13),2.5*cos(0.25*ctime));
    vec3 ww = normalize(vec3(0.0) - ro);
    vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
    vec3 vv = normalize(cross(ww,uu));
    vec3 rd = normalize( p.x*uu + p.y*vv + 2.5*ww );

	vec3 col = getSceneColor( ro, rd );
    gl_FragColor = vec4(col,1.0);
}
#endif
