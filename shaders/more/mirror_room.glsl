// @var title Mirror Room
// @var author DiLemming
// @var url https://www.shadertoy.com/view/4sS3zc

// @var eyePos 0.6 2.1 -3.0

#define REFLECTIONS 16

#define FOG_STRENGTH 1.0
#define REFLECTION_STRENGTH 100000.0
#define COLOR_STRENGTH 10.0
#define BRIGHTNESS 6.0

struct ray {
	vec3 p;
	vec3 d;
};

struct material {
	vec3 color;
	vec3 emmision;
	float diffusion;
};

struct hit {
	vec3 p;
	vec3 n;
	float t;
	material m;
};

void plane (vec3 v, float f, ray r, material m, inout hit h) {
	float t = (f - dot (v, r.p)) / dot (v, r.d);
	
	if (t < 0.0)
		return;
	
	if (t < h.t) {
		h.t = t;
		h.p = r.p + r.d * t;
		h.n = /*normalize*/ (faceforward (v, v, r.d));
		
		h.m = m;
	}
}

void sphere (vec3 v, float f, ray r, material m, inout hit h) {
	vec3 d = r.p - v;
	
	float a = dot (r.d, r.d);
	float b = dot (r.d, d);
	float c = dot (d, d) - f * f;
	
	float g = b*b - a*c;
	
	if (g < 0.0)
		return;
	
	float t = (-sqrt (g) - b) / a;
	
	if (t < 0.0)
		return;
	
	if (t < h.t) {
		h.t = t;
		h.p = r.p + r.d * t;
		h.n = (h.p - v) / f;
		
		h.m = m;
	}
}

hit scene (ray r) {
	hit h;
	h.t = 1e20;
	
	material m0 = material (vec3 (1), vec3 (0), 0.5);
	
	plane	(vec3 ( 1.0, 0.1, 0.0), 3.0, r, m0, h);
	plane	(vec3 ( 1.0, 0.4, 0.0),-3.0, r, m0, h);
	plane	(vec3 ( 0.0, 1.0, 0.0), 0.0, r, m0, h);
	plane	(vec3 ( 0.0, 1.0, 0.0), 5.0, r, m0, h);
	plane	(vec3 ( 0.0, 0.0, 1.0), 5.0, r, m0, h);
	plane	(vec3 ( 1.0, 0.0, 1.0),-6.0, r, m0, h);
	sphere	(vec3 (-1.0, sin (iGlobalTime) + 3.0,0), 1.0, r, m0, h);
	sphere	(vec3 ( 1.0, 1.0, 2.0), 1.0, r, m0, h);
	
	h.m.color *= h.n * h.n;
	
	return h;
}

ray getRay (in vec3 origin, in vec3 forward, in vec3 up, in vec2 uv) {
	ray r;
	
	r.p = origin;
	vec3 right = cross (up, forward);
	up = cross (forward, right);
	r.d = normalize (right * uv.x + up * uv.y + forward);
	
	return r;
}

vec3 surface (ray r) {
	vec3 color = vec3 (0);
	
	float depth = 0.0;
	
	float l = 0.0;
	
	for (int i = 0; i < REFLECTIONS; i++) {
		hit h = scene (r);
		
		float c = dot (h.n, r.d);
		depth += (1.0 / REFLECTION_STRENGTH) + h.t * FOG_STRENGTH;
		
		r = ray (h.p + h.n * 0.0001, reflect (normalize (r.d), h.n));
		
		float d = 1.0 / (depth * depth);
		color = (color + c*c*d) * (1.0 - h.m.color * d * COLOR_STRENGTH);
	}
	
	return color * BRIGHTNESS;
}

vec3 getSceneColor( vec3 ro, vec3 rd )
{
    ray r;
    r.p = ro;
    r.d = rd;
    vec2 uv = vec2(0.);
    vec3 col = surface (r) * (1.3 - max (abs (uv.x), abs (uv.y * 1.5)));
    return col;
}

#ifndef RIFTRAY
void main (void) {
	vec3 camera = vec3 (sin (iGlobalTime * 0.5), 2, -4.0);
	vec3 forward = vec3 (0,0,1);
	
	vec2 uv = (gl_FragCoord.xy * 2.0 - iResolution.xy) / iResolution.xx;
	ray r = getRay (camera, forward, vec3 (0,1,0), uv);
	
	gl_FragColor = vec4 (getSceneColor(r.p, r.d), 1.0);
}
#endif
