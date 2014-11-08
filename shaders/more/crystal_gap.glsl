// Crystal Gap
// by srtuss, 2013
// messing around with voronoi noise
// requires more work though, but i'm too lazy atm ;)

// @var title Crystal Gap
// @var author srtuss
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/Msl3DM

// rotate position around axis
vec2 rotate(vec2 p, float a)
{
	return vec2(p.x * cos(a) - p.y * sin(a), p.x * sin(a) + p.y * cos(a));
}

// 1D random numbers
float rand(float n)
{
    return fract(sin(n) * 43758.5453123);
}

// 2D random numbers
vec2 rand2(in vec2 p)
{
	return fract(vec2(sin(p.x * 591.32 + p.y * 154.077), cos(p.x * 391.32 + p.y * 49.077)));
}

// 1D noise
float noise1(float p)
{
	float fl = floor(p);
	float fc = fract(p);
	return mix(rand(fl), rand(fl + 1.0), fc);
}

// voronoi distance noise, based on iq's articles
vec2 voronoi(in vec2 x)
{
	vec2 p = floor(x);
	vec2 f = fract(x);
	
	vec2 res = vec2(8.0);
	for(int j = -1; j <= 1; j ++)
	{
		for(int i = -1; i <= 1; i ++)
		{
			vec2 b = vec2(i, j);
			vec2 r = vec2(b) - f + rand2(p + b);
			
			// chebyshev distance - one of many ways to do this
			float d = max(abs(r.x), abs(r.y));
			
			if(d < res.x)
			{
				res.y = res.x;
				res.x = d;
			}
			else if(d < res.y)
			{
				res.y = d;
			}
		}
	}
	return res;
}

float scene(vec3 p)
{
	vec2 ns = voronoi(p.yz);
	return 0.5 - abs(p.x) + (ns.y - ns.x) * 0.5 + (sin(p.x * 1.5) * sin(p.y * 1.5)) * 0.3;
}

vec3 normal(vec3 p)
{
	float v = scene(p);
	vec3 n;
	vec2 d = vec2(0.01, 0.0);
	n.x = v - scene(p + d.xyy);
	n.y = v - scene(p + d.yxy);
	n.z = v - scene(p + d.yyx);
	return normalize(n);
}

float intersect(inout vec3 ray, vec3 dir)
{
	float dist = 0.0;
	for(int i = 0; i < 30; i ++)
	{
		dist += scene(ray + dir * dist);
		if(dist > 5.0)
			return dist;
	}
	ray += dir * dist;
	return dist;
}

vec3 shade(vec3 ray, vec3 dir, vec3 nml, float dist)
{
	vec3 col = vec3(0.0, 0.0, 0.0);
	vec2 ns = voronoi(ray.yz);
	
	vec3 light = normalize(vec3(0.2, 1.0, 0.3));
	
	// lightig
	float diff = dot(nml, -light) * 0.4 + 0.6;
	vec3 ref = reflect(dir, nml);
	float spec = max(dot(ref, light), 0.0);
	spec = pow(spec, 8.0);
	
	// fake ambient occlusion
	float occ = exp(-(ns.y - ns.x));//exp(-abs(ray.x) * 1.5) * 2.0;
	col = vec3(0.0, 0.7, 1.0);
	//col += textureCube(iChannel0, ref).xyz * 0.3;
	col *= diff * 1.0;
	col += spec;
	col *= occ;
	
	// distance blackness
	col *= exp(-dist * 0.3);
	
	// simulate a torch or something
	col += exp(-dist * 0.9 - noise1(iGlobalTime * 2.0) * 0.5);
	
	return col;
}

vec3 getSceneColor( in vec3 ray, in vec3 dir )
{
	vec3 col = vec3(0.0, 0.0, 0.0);
	float dist = intersect(ray, dir);
	
	
	if(dist < 5.0)
	{
		vec3 nml, ref = dir;
		
		
		nml = normal(ray);
		col += 0.8 * shade(ray, ref, nml, dist);
		ref = reflect(ref, nml);
		ray += ref * 0.1;
		
		dist += intersect(ray, ref);
		if(dist < 5.0)
		{
			nml = normal(ray);
			col += 0.2 * shade(ray, ref, nml, dist);
		}
	}
	
	// more contrast
	col = pow(col, vec3(3.0));
	
	// gamma correction
	col = 1.0 - exp(-col * 4.0);
	
	return col;
}

#ifndef RIFTRAY
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = 2.0 * uv - 1.0;
	uv.x *= iResolution.x / iResolution.y;
	
	
	vec3 eye = vec3(0.0, 0.0, -2.0);
	float t = iGlobalTime * 2.5;
	//float fr = 0.0;
	//fr = sin(t) + sin(t * 2.0) * 0.5 + sin(t * 4.0) * 0.25;
	eye.y = -t * 0.1;// - fr * 0.2;
	
	vec3 ray = eye;
	
	vec3 dir = normalize(vec3(uv, 1.0));
	
	dir.yz = rotate(dir.yz, sin(iGlobalTime * 0.7) * 0.5 + 0.5);
	dir.xz = rotate(dir.xz, sin(iGlobalTime * 0.2) * 0.5);
	
	vec3 col = getSceneColor( ray, dir );
	gl_FragColor = vec4(col, 1.0);
}
#endif
