// caosdoar@gmail.com 2014

// @var title Lighting room
// @var author caosdoar
// @var url https://www.shadertoy.com/view/ld2XzV

// @var eyePos -0.9 0.0 -4.9

// Some defines to control the mouse inputs
#define MOUSE_INPUT 0
#define MOUSE_INPUT_MODE 0

// Anti-aliased step from Stefan Gustavson
float aastep(float threshold, float value) 
{
    float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
    return smoothstep(threshold-afwidth, threshold+afwidth, value);
}
   
// The inverted box for the room
float room(vec3 p)
{
    vec3 d = abs(p) - vec3(10.0, 3.5, 10.0);
    return -max(d.x, max(d.y, d.z));
}

float mapTerrain(vec3 p)
{
    return room(p);
}

float raymarch(vec3 ro, vec3 rd)
{
	float maxd = 30.0;
    float h = 1.0;
    float t = 0.1;
    for( int i=0; i<160; i++ )
    {
        if( h<(0.001*t)||t>maxd ) break;
	    h = mapTerrain( ro+rd*t );
        t += h;
    }

    if( t>maxd ) t=-1.0;
    return t;
}

vec3 calcNormal(vec3 pos)
{
    vec3 eps = vec3(0.001,0.0,0.0);
	return normalize( vec3(
           mapTerrain(pos+eps.xyy) - mapTerrain(pos-eps.xyy),
           mapTerrain(pos+eps.yxy) - mapTerrain(pos-eps.yxy),
           mapTerrain(pos+eps.yyx) - mapTerrain(pos-eps.yyx) ) );

}

// An approximation to calculate regular polygons
float poly(vec2 p, vec2 c, float l, float r)
{
    vec2 v = p - c;
    float v_l = length(v);
    vec2 n = v / v_l;
    float a = acos(dot(n, vec2(1.0, 0.0)));
    float pi_l = 3.14159265/l;
    a = mod(a, 2.0*pi_l);
    a = abs(a - pi_l);
    float d = cos(pi_l);
    return v_l - r * (d / cos(a));
}

float polylines(vec2 p, vec2 c, float l)
{
    vec2 v = p - c;
    float v_l = length(v);
    vec2 n = v / v_l;
    float a = acos(dot(n, vec2(1.0, 0.0))) + 3.14159265 / l;
    float line_angle = floor(a * l / 6.283185) * 6.283185 / l;
    vec2 line_dir = normalize(vec2(cos(line_angle), sin(line_angle)));
    return length((-v)-dot(-v,line_dir)*line_dir);
}

// A fancy circle
float circle_curves(vec2 p, float r, float l)
{
    vec2 n = normalize(p);
    float a = acos(dot(n, vec2(1.0, 0.0)));
    float pi_l = 3.14159265/l;
    a = mod(a, 2.0*pi_l);
    a = abs(a - pi_l);
    float s = a/pi_l;
    s *= s;
    return length(p) - (r - 0.01*r*s);
}

// Just a square
float square(vec2 p, float lh)
{
    vec2 d = abs(p) - lh;
    return max(d.x, d.y);
}

vec2 rotate_point(vec2 p, float a)
{
    float s = sin(a);
    float c = cos(a);
    return vec2(p.x*c-p.y*s,p.x*s+p.y*c);
}

// WIP test for the middle pattern
float alahambra_pattern(vec2 p)
{
    /*p *= 1.5;
    
    float a = poly(p, vec2(0.0), 8.0, 0.3);
    float b = poly(p, vec2(0.3, 0.3), 8.0, 0.3);
    float c = poly(p, vec2(-0.3, 0.3), 8.0, 0.3);
    float d = poly(p, vec2(0.3, -0.3), 8.0, 0.3);
    float e = poly(p, vec2(-0.3, -0.3), 8.0, 0.3);
    float f = poly(p, vec2(0.3, 0.0), 8.0, 0.3);
    float g = poly(p, vec2(-0.3, 0.0), 8.0, 0.3);
    float h = poly(p, vec2(0.0, 0.3), 8.0, 0.3);
    float i = poly(p, vec2(0.0, -0.3), 8.0, 0.3);
    
    float lines = 
        min(abs(a), 
        min(abs(b), 
        min(abs(c), 
        min(abs(d), 
        min(abs(e),
        min(abs(f),
        min(abs(g),
        min(abs(h),
        abs(i)))))))));
   	lines = min(1.0, lines * 100.0);
    return lines;*/
   
    float a = poly(p, vec2(0.0), 12.0, 0.1);
    
    float b = poly(p, vec2(0.0), 6.0, 0.15);
    float b1 = poly(p, vec2(0.225, 0.13), 6.0, 0.15);
    float b2 = poly(p, vec2(-0.225, 0.13), 6.0, 0.15);
    float b3 = poly(p, vec2(0.225, -0.13), 6.0, 0.15);
    float b4 = poly(p, vec2(-0.225, -0.13), 6.0, 0.15);
    float b5 = poly(p, vec2(0.0, 0.26), 6.0, 0.15);
    float b6 = poly(p, vec2(0.0, -0.26), 6.0, 0.15);
    
    float c = polylines(p, vec2(0.0), 12.0);
    //float lines = min(abs(a), abs(b) * (c) );
    
    float s0 = square(rotate_point(p, 3.14159/12.0), 0.085);
    float s1 = square(rotate_point(p, 3.14159*5.0/12.0), 0.085);
    float s2 = square(rotate_point(p, 3.14159*9.0/12.0), 0.085);
    
    float lines = abs(abs(b) - 0.01) * 300.0;
    lines = min(lines, abs(abs(b1) - 0.01) * 300.0);
    lines = min(lines, abs(abs(b2) - 0.01) * 300.0);
    lines = min(lines, abs(abs(b3) - 0.01) * 300.0);
    lines = min(lines, abs(abs(b4) - 0.01) * 300.0);
    lines = min(lines, abs(abs(b5) - 0.01) * 300.0);
    lines = min(lines, abs(abs(b6) - 0.01) * 300.0);
    
    lines = min(lines, abs(s0) * 300.0);    
    lines = min(lines, abs(s1) * 300.0);    
    lines = min(lines, abs(s2) * 300.0);
    
    return lines;
    //return min(1.0, lines * 100.0);
}

// One of the faces of the shadow box
float shadowPlane(vec2 p)
{
    //return aastep(0.5, alahambra_pattern(p));
    
    vec2 p_abs = abs(p);
    float p_abs_min = min(p_abs.x, p_abs.y);
    float d = length(p);
    
    float c0 = min(1.0, abs(d - 0.35) * 80.0);
    float c1 = min(1.0, abs(d - 0.48) * 80.0);
    float c2 = min(1.0, abs(circle_curves(p, 0.55, 40.0)) * 120.0);
    float c3 = min(1.0, abs(circle_curves(p, 0.59, 40.0)) * 80.0);
    float s0 = min(1.0, abs(square(p, 0.595)) * 90.0);
    float s1 = min(1.0, abs(square(p, 0.8)) * 90.0);
    float s2 = min(1.0, abs(square(p, 1.0)) * 10.0);
    /*s2 *= s2;
    s2 *= s2;*/
    
    float lines = min(c0, min(c1, min(c2, min(c3, min(s0, min(s1, s2))))));
    
    
    vec2 p45 = vec2(p.x-p.y,p.x+p.y) * 0.7071;
    
    float g0 = min(1.0, abs(square(fract(p45 * 40.0) * 2.0 - 1.0, 1.0)) * 2.0);
    g0 = mix(1.0, g0, step(0.0, square(p, 0.8)));
    float g1 = alahambra_pattern(p);
    g1 = mix(1.0, g1, step(d - 0.35, 0.0));
    
    float patterns = min(g0, g1);
    
    return aastep(0.5, min(lines, patterns));
}

// Box in the middle of the room that creates the shadows
float shadowBox(vec3 l)
{
    vec2 uv;
    vec3 labs = abs(l);
    float lmax = max(labs.x, max(labs.y, labs.z));
    if (labs.x == lmax) uv = l.yz;
    if (labs.y == lmax) uv = l.xz;
    if (labs.z == lmax) uv = l.xy;
    
	uv /= lmax;
    
    return shadowPlane(uv);
    
    /*uv = floor(uv * 5.0);
    return mod(uv.x+uv.y, 2.0) * 0.5 + 0.5;
    return length(uv);*/
}

float mapLamp(vec3 p)
{
    float l = 2.0;
    vec3 d = abs(p) - l;
  	return min(max(d.x,max(d.y,d.z)),0.0)+length(max(d,0.0));
}

float raymarchLamp(vec3 ro, vec3 rd)
{
	float maxd = 30.0;
    float h = 1.0;
    float t = 0.1;
    for( int i=0; i<160; i++ )
    {
        if( h<(0.001*t)||t>maxd ) break;
	    h = mapLamp( ro+rd*t );
        t += h;
    }

    if( t>maxd ) t=-1.0;
    return t;
}

// Simplest lighting possible
float lighting(vec3 p, vec3 n)
{
    vec3 l = -normalize(p);
    float c = dot(n, l);
    float shadow = shadowBox(-l);
    c *= shadow;
    return c;
}

// Transform from spherical to cartesian coordinates
vec3 sphe2cart(vec2 p)
{
    vec2 s  = sin(p);
    vec2 c  = cos(p);
    return normalize(vec3(c.x * s.y, c.y, s.x * s.y));
}

vec3 getSceneColor(in vec3 co, in vec3 rd)
{
    float a = raymarch(co, rd);
    vec3 p0 = co + rd * a;
    vec3 n = calcNormal(p0);
    
    // Profit!
    vec3 colour = lighting(p0, n) * vec3(0.85, 0.75, 0.6);
    
    // Raymarch lamp
    float a1 = raymarchLamp(co, rd);
    if (a1 > 0.0)
    {
    	vec3 p1 = co + rd * a1;
        colour = mix(vec3(0.1), colour, shadowBox(p1));
    }
    return colour;
}

#ifndef RIFTRAY
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    
#if MOUSE_INPUT
    
#if MOUSE_INPUT_MODE == 0 
    // Rotate camera around the center
    vec2 camSphe = vec2(0.0, 1.175) + iMouse.xy / iResolution.xy * vec2(5.0, 0.7);
    vec3 co = sphe2cart(camSphe) * 9.0;
    vec3 cd = vec3(0.0, 0.0, 0.0);
#else
    // Rotate the camera from the corner
    vec3 co = vec3(8.5, -2.0, 8.5);
    vec2 camSphe = vec2(0.0, 1.57) + (vec2(iMouse.x, -iMouse.y) / iResolution.xy - vec2(0.5)) * vec2(3.0, 1.5);
    vec3 cd = co + sphe2cart(vec2(-2.4, 1.3) + camSphe);
#endif

#else
    
    vec2 camSphe;
    if (iMouse.z < 1.0)
    {
        camSphe = vec2(iGlobalTime * 0.1, sin(iGlobalTime * 0.2) * 0.2 + 1.5);
    }
    else
    {
        camSphe = vec2(0.0, 1.175) + iMouse.xy / iResolution.xy * vec2(5.0, 0.7);
    }
    
    vec3 co = sphe2cart(camSphe) * 9.0;
    vec3 cd = vec3(0.0, 0.0, 0.0);
    
#endif
    
    // Camera
    vec3 cf = normalize(cd - co);
    vec3 cu = vec3(0.0,1.0,0.0);
    vec3 cr = normalize(cross(cf, cu));
    cu = cross(cr, cf);
    
    // Raymarch
    uv -= vec2(0.5);
    uv.x *= iResolution.x / iResolution.y;
    uv *= 1.0;
    vec3 rd = normalize(cf + cr * uv.x + cu * uv.y);
    vec3 colour = getSceneColor(co, rd);
	gl_FragColor = vec4(vec3(colour),1.0);
}
#endif
