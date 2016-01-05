// @var title first mapping test
// @var author RenoM
// @var url https://www.shadertoy.com/view/MsdGRS

// @var eyePos -0.66 1.24 -3.25

#define NB_OBJ 8
#define T iGlobalTime
#define PI 3.14159265
#define EPS .01
#define MAX 50.

vec3 lp = vec3(3.*cos(.8*T),.5,3.*sin(.8*T));

vec2 rot(vec2 p, float t)
{
    vec2 s = vec2(cos(t), sin(t));
    return p * mat2(s, -s.y, s.x);
}

float map(vec3 p, int id)
{
    if(id == 0) return length(p.xz) - 1.;
    if(id == 1) return abs(p.y + 1.2);
    if(id == 2) return abs(p.y - 2.2);
    if(id == 3) return length(p-lp) - .1;
    if(id == 4) return abs(p.z - 5.);
    if(id == 5) return abs(p.z + 5.);
    if(id == 6) return abs(p.x - 5.);
    if(id == 7) return abs(p.x + 5.);
    return MAX;
}

float march(vec3 ro, vec3 rd, out float d, int id)
{
    float t = .0;
    for(int i = 0; i < 64; i++)
    {
        d = map(ro+t*rd, id);
        if(d < EPS || t > MAX) break;
        t += d;
    }
    return t;
}

int inter(vec3 ro, vec3 rd, out float t)
{
    int id = -1;
    float d,l;
    t = MAX;
    for(int i = 0; i < NB_OBJ; i++)
    {
        l = march(ro, rd, d, i);
        if(d < EPS && l < t)
        {
            t = l;
            id = i;
        }
    }
    return id;
}

vec3 normal(vec3 p, int id)
{
    vec2 q = vec2(0,EPS);
    return normalize(vec3(map(p+q.yxx, id) - map(p-q.yxx, id),
                          map(p+q.xyx, id) - map(p-q.xyx, id),
                          map(p+q.xxy, id) - map(p-q.xxy, id)));
}

float hash12(vec2 p)
{
    return  fract(sin(dot(p.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 hash32(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * vec3(443.8975,397.2973, 491.1871));
    p3 += dot(p3, p3.yxz+19.19);
    return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

float isInSuperellipse(vec2 uv, vec2 o, float r, float n)
{
    float res = pow(abs((uv.x - o.x) / r), n) + pow(abs((uv.y - o.y) / r), n);
    return (res <= 1. ? sqrt(1. - res) : .0);
}

vec3 segrid(vec2 p)
{
    vec2 frac = fract(p);
    p = floor(p);
    float time = ceil(T);
    float res = isInSuperellipse(frac, vec2(.5), .5, 4. * hash12(p));
    vec3 hash = hash32(p);
	vec3 col = .7 * hash * res;
    float h = floor(200. - hash12(p) * 200.);
    if(floor(mod(T * 2.,50.)) == h) col *= 3.;
    return col;
}

vec3 col(vec3 p, int id)
{
    if(id == 0) return segrid(60. * vec2(atan(p.z,p.x), p.y) / PI);
    if(id == 1 || id == 2) return segrid(p.xz*15.);
    if(id == 4 || id == 5) return segrid(p.xy*15.);
    if(id == 6 || id == 7) return segrid(p.yz*15.);
    return vec3(0);
}

vec3 shade(vec3 p, int id)
{
    if(id == 3) return vec3(1);
    vec3 rd = normalize(p-lp);
    float f;
    f = inter(lp+rd, rd, f) != id ? .2 : 1.;
    vec3 n = normal(p,id);
    f *= max(.1,dot(n,-rd));
    return f * col(p,id);
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    float t;
    int id = inter(ro, rd, t);
    
    vec3 c = id == -1 ? vec3(0) : shade(ro+t*rd, id);
    c *= 9. / (1.+t);
    return c;
}

#ifndef RIFTRAY
void mainImage( out vec4 O, in vec2 U )
{
    vec2 R = iResolution.xy,
         p = tan(.5236) * (U+U-R) / R.y;
    vec3 ro = vec3(0,0,-4.9),
         rd = normalize(vec3(p,1));
    
    ro.xz = rot(ro.xz, .8*T);
    rd.xz = rot(rd.xz, .8*T);
    vec3 c = getSceneColor(ro, rd);
    O.xyz = clamp(c, .0, 1.);
}
#endif
