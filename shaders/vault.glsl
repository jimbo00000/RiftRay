// @var title Vault
// @var author dila
// @var url https://www.shadertoy.com/view/ltBSDw

// @var tex0 tex00.jpg

float cosh(float x, float a)
{
    float xa = x / a;
 	return 0.5 * a * (exp(xa) + exp(-xa));   
}

float map(vec3 p)
{
    vec3 fp = fract(p);
    vec3 pa = p + 0.5;
    vec3 fpa = fract(pa);
    vec3 q = vec3(fpa.x, pa.y, fpa.z) * 2.0 - 1.0;
    vec3 cq = vec3(fp.x, p.y, fp.z) * 2.0 - 1.0;
    
    float ctx = cosh(q.x, 0.5);
    float ctz = cosh(q.z, 0.5);
    q.y += mix(ctx, ctz, 0.5);
    
    float cx = length(q) - 2.0;
    
    float blend = 1.0 / (1.0 + max(p.y + 0.5, 0.0));
    
    float vp = length(cq.xz) - 0.3;
    
    cx = mix(-cx, vp, blend);
    
    float cl = p.y + 0.5;

    return min(cx, cl);
}

vec3 normal(vec3 p)
{
	vec3 o = vec3(0.01, 0.0, 0.0);
    return normalize(vec3(map(p+o.xyy) - map(p-o.xyy),
                          map(p+o.yxy) - map(p-o.yxy),
                          map(p+o.yyx) - map(p-o.yyx)));
}

float trace(vec3 o, vec3 r)
{
 	float t = 0.0;
    for (int i = 0; i < 64; ++i) {
     	vec3 p = o + r * t;
        float d = map(p);
        t += d * 0.3;
        if (d < 0.001) {
            break;
        }
    }
    return t;
}

mat2 rot(float t)
{
	return mat2(cos(t), sin(t), -sin(t), cos(t));
}

vec3 dolight(vec3 o, vec3 w, vec3 r, vec3 sn)
{
    vec3 lp = vec3(0.0, 0.0, 0.0);
    vec3 del = lp - w;
    float len = dot(del, del);
    float ld = sqrt(len);
    del /= ld;
    float lt = trace(lp+sn*0.01, -del);
    float lm = 1.0;
    if (lt < ld) {
    	lm = 0.0;
    }
    vec3 lf = -r;
    float la = pow(max(dot(lf, del), 0.0), 4.0);
    float fo = 1.0 / (1.0 + len * 0.01);
    float fl = la * lm * fo * max(dot(sn, del), 0.0);
    vec3 lcol = vec3(1.0, 1.0, 0.5);
    vec3 wc = mix(vec3(0.9,0.9,1.0), lcol, 0.5+0.5*sn.y);
    vec3 fc = mix(wc, lcol*fl, 0.95);
    return fc;
}

vec3 textureTL(vec3 pp)
{
 	vec3 ta = texture2D(iChannel0, pp.yz).xyz;
    vec3 tb = texture2D(iChannel0, pp.xz).xyz;
    vec3 tc = texture2D(iChannel0, pp.xy).xyz;
    return (ta + tb + tc) / 3.0;
}

vec3 getSceneColor( in vec3 o, in vec3 r )
{
    float t = trace(o, r);
    vec3 w = o + r * t;
    vec3 sn = normal(w);
    float fd = map(w);

    vec3 tex = textureTL(w);
    float bmp = dot(tex*2.0-1.0,vec3(1.0));
    sn.y = sign(sn.y) * abs(sn.y * bmp);
    sn = normalize(sn);
    
    vec3 lit = dolight(o, w, r, sn);
    float fog = 1.0 / (1.0 + t * t * 0.1 + fd * 100.0);
    vec3 fc = fog * tex * lit;
    return sqrt(fc);
}

#ifndef RIFTRAY
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;
    
    vec3 r = normalize(vec3(uv, 1.0 - dot(uv, uv) * 0.3));
    vec3 o = vec3(0.0, 0.0, -1.3);
    
    r.yz *= rot(3.14159*0.125);
    o.xz *= rot(iGlobalTime*0.75);
    r.xz *= rot(iGlobalTime*0.75);
    
    vec3 fc = getSceneColor( o, r );
	fragColor = vec4(fc,1.0);
}
#endif
