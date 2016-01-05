// @var title CaveStructure
// @var author dila
// @var url https://www.shadertoy.com/view/4l2XW1

// @var tex0 tex07.jpg

// @var eyePos 0.41 0.45 0.79

const float pi = 3.14159;

mat3 xrot(float t)
{
    return mat3(1.0, 0.0, 0.0,
                0.0, cos(t), -sin(t),
                0.0, sin(t), cos(t));
}

mat3 yrot(float t)
{
    return mat3(cos(t), 0.0, -sin(t),
                0.0, 1.0, 0.0,
                sin(t), 0.0, cos(t));
}

mat3 zrot(float t)
{
    return mat3(cos(t), -sin(t), 0.0,
                sin(t), cos(t), 0.0,
                0.0, 0.0, 1.0);
}

float sdBoxXY( vec3 p, float b )
{
  vec2 d = abs(p.xy) - b;
  return min(max(d.x,d.y),0.0) +
         length(max(d,0.0));
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

float sphere(vec3 p, float r)
{
 	return length(p) - r;
}

float map(vec3 p)
{
    p.xy += vec2(sin(p.z), cos(p.z)) * 0.25;
    
	vec3 q = fract(p) * 2.0 - 1.0;
    
    vec3 f = floor(p + 0.5);
    
    vec3 k = abs(normalize(q));
    
    float a = -sdBox(q, k+0.1);
    
    float b = -sphere(q, 1.3);
    
    float d = max(-b, a);
    
    float tt = 0.5+0.5*sin(p.z);
    float tr = mix(0.125, 1.0, tt);
    
    float c = sdBoxXY(p - 0.5, tr);
    
    return max(-c, d);
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
    }
    return t;
}

vec3 times(float n)
{
    float k = fract(iGlobalTime*n) * 3.0;
    vec3 t = vec3(clamp(k, 0.0, 1.0),
                clamp(k-1.0, 0.0, 1.0),
                clamp(k-2.0, 0.0, 1.0));
    return floor(iGlobalTime*n) + smoothstep(0.0, 1.0, t);
}

vec3 textureTL(vec3 p)
{
 	vec3 ta = texture2D(iChannel0, p.yz).xyz;
    vec3 tb = texture2D(iChannel0, p.xz).xyz;
    vec3 tc = texture2D(iChannel0, p.xy).xyz;
    return (ta + tb + tc) / 3.0;
}

vec3 getSceneColor( in vec3 o, in vec3 r )
{
    float t = trace(o, r);
    vec3 w = o + r * t;
    vec3 sn = normal(w);
    float fd = map(w);
    
    float prod = clamp(dot(r, -sn), 0.0, 1.0);
    
    vec3 colfar = vec3(1.0, 0.0, 0.0);
    vec3 colnear = vec3(1.0, 1.0, 1.0);
    
    float colk = 1.0 / (1.0 + t * t * 0.1);
    vec3 col = mix(colfar, colnear, colk);

    col *= textureTL(w * 0.1) * colk;
    
    float aoc = 1.0 / (1.0 + fd * 100.0);

    vec3 fc = sqrt(col) * aoc * prod;
    return fc;
}

#ifndef RIFTRAY
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;
    
    vec3 r = normalize(vec3(uv, 1.0 - dot(uv,uv) * 0.33));
    vec3 o = vec3(0.5, 0.5, iGlobalTime);
    o.xy -= vec2(sin(o.z), cos(o.z)) * 0.25;
    
    vec3 ts = times(0.213);
    r *= xrot(ts.y+ts.z) * yrot(ts.x+ts.z) * zrot(ts.x+ts.y);
    
    vec3 fc = getSceneColor(o, r);
	fragColor = vec4(fc,1.0);
}
#endif
