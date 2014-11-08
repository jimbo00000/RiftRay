// rainbow spaghetti by mattz
//
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// Some code from iq's raymarching primer: https://www.shadertoy.com/view/Xds3zN

// @var title rainbow spaghetti
// @var author mattz
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/lsjGRV

// @var headSize 0.3
// @var eyePos 0.81623608 1.0209554 -2.3850200

const float i3 = 0.5773502691896258;
const float r = 0.40824829046386302;

const float i = 0.3333333333333333;
const float j = 0.6666666666666666;

const float lrad = 0.015;
const float trad = 0.06;
const float fogv = 0.025;

const float dmax = 20.0;
const int rayiter = 60;

const float wrap = 64.0;

vec3 L = normalize(vec3(0.1, 1.0, 0.5));

const vec3 axis = vec3(1.0, 1.0, 0.0);//vec3(1.0, 1.0, 1.0);
const vec3 tgt = vec3(1.0, 1.7, 1.1);//vec3(-0.0, 0.3, -0.15);
const vec3 cpos = tgt + axis;

const vec3 vel = 0.2*axis;

const float KEY_G = 71.5/256.0;

float hash(in vec3 x) {
	return fract(87.3*dot(x, vec3(0.1, 0.9, 0.7)));
}

float line(in vec3 p0, in vec3 p1, in vec3 p) {
	
	vec3 dp0 = p-p0;
	vec3 d10 = p1-p0;
	
	float u = clamp(dot(dp0, d10)/dot(d10, d10), -5.0, 5.0);
	return distance(mix(p0, p1, u), p)-0.5*lrad;

}

vec2 opU(vec2 a, vec2 b) {
	return a.x < b.x ? a : b;
}

float hueOf(vec3 pos) {
	return cos( 2.0*dot(2.0*pos, vec3(0.3, 0.7, 0.4)) ) * 0.49 + 0.5;
}

vec3 round2(in vec3 x, in vec3 a) {
	return 2.0 * floor( 0.5 * (x + 1.0 - a) ) + a;
}

vec4 pdist(vec3 p, vec3 q) {
	vec3 pq = p-q;
	return vec4(q, dot(pq,pq));
}

vec4 pselect(vec4 a, vec4 b) {
	return a.w < b.w ? a : b;
}

float torus(in vec3 a, in vec3 b, in vec3 pos) {
	pos -= 0.5*(a+b);
	vec3 n = normalize(b-a);
	return distance(pos, r*normalize(pos - n*dot(n, pos))) - trad;
}

mat4 permute(vec3 e, vec3 f, vec3 g, vec3 h, float p) {
	return (p < i ? mat4(vec4(e,1.0), vec4(f,1.0), vec4(g, 1.0), vec4(h, 1.0)) :
			(p < j ? mat4(vec4(e,1.0), vec4(g,1.0), vec4(f, 1.0), vec4(h, 1.0)) :
			 mat4(vec4(e,1.0), vec4(h,1.0), vec4(f, 1.0), vec4(g, 1.0))));
}

vec3 randomBasis(float p) {
	return (p < i ? vec3(1.0, 0.0, 0.0) :
			p < j ? vec3(0.0, 1.0, 0.0) :
			vec3(0.0, 0.0, 1.0));
}

vec3 randomPerp(vec3 v, float p) {
	return (v.x>0.0 ? (p < 0.5 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0)) :
			v.y>0.0 ? (p < 0.5 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 1.0)) :
			(p < 0.5 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0)));
}


vec2 map(in vec3 pos) {
		
	vec3 orig = pos;
	
	/// Slow pan animation
	//pos = mod(pos + mod(iGlobalTime*vel, wrap), wrap);
		
	// a, b, c, d are octahedron centers
	// d, e, f, g are tetrahedron vertices
	vec3 a = round2(pos, vec3(1.0));
	vec3 h = round2(pos, vec3(0.0));
	
	vec3 b = vec3(a.x, h.y, h.z);
	vec3 c = vec3(h.x, a.y, h.z);
	vec3 d = vec3(h.x, h.y, a.z);
	
	vec3 e = vec3(h.x, a.y, a.z);
	vec3 f = vec3(a.x, h.y, a.z);
	vec3 g = vec3(a.x, a.y, h.z);

	// o is the closest octahedron center
	vec3 o = pselect(pselect(pdist(pos, a), pdist(pos, b)),
					 pselect(pdist(pos, c), pdist(pos, d))).xyz;
	
	// t is the closest tetrahedron center
	vec3 t = floor(pos) + 0.5;

	// normal points towards o
	// so bd is positive inside octahedron, negative inside tetrahedron
	float bd = dot(pos - o.xyz, (o.xyz-t.xyz)*2.0*i3) + i3;	

	mat4 m = permute(e,f,g,h,hash(mod(t, wrap)));
	
	float t1 = torus(m[0].xyz, m[1].xyz, pos);
	float t2 = torus(m[2].xyz, m[3].xyz, pos);
	
	float p = hash(mod(o, wrap));
	vec3 b1 = randomBasis(fract(85.17*p));
	vec3 b2 = randomPerp(b1, fract(63.61*p+4.2));
	vec3 b3 = randomPerp(b1, fract(43.79*p+8.3));

	vec3 po = pos-o;
	
	float o1 = torus( b1,  b2, po);
	float o2 = torus( b1, -b2, po);
	float o3 = torus(-b1,  b3, po);
	float o4 = torus(-b1, -b3, po);
						 
	vec2 noodle = vec2(min(max(bd, min(t1,t2)),
						   max(-bd, min(min(o1, o2), min(o3, o4)))),
					   hueOf(orig+0.5*vel*iGlobalTime));
						   	
	if (false) { //texture2D(iChannel0, vec2(KEY_G, 0.75)).x > 0.0) {
				
		float dline = line(e, f, pos);
		dline = min(dline, line(e, g, pos));
		dline = min(dline, line(e, h, pos));
		dline = min(dline, line(f, g, pos));
		dline = min(dline, line(f, h, pos));
		dline = min(dline, line(g, h, pos));
		
		vec2 grid = vec2(dline, 2.0);
		
		noodle.x += 0.1*trad;
		noodle.y = hash(mod(bd < 0.0 ? t : o, wrap));
		return opU(grid, noodle);
		
	} else {
		
		return noodle;
		
	}
	
}

vec3 hue(float h) {
	
	vec3 c = mod(h*6.0 + vec3(2, 0, 4), 6.0);
	return h > 1.0 ? vec3(0.5) : clamp(min(c, -c+4.0), 0.0, 1.0);
}

vec2 castRay( in vec3 ro, in vec3 rd, in float maxd )
{
	float precis = 0.0001;
    float h=precis*2.0;
    float t = 0.0;
    float m = -1.0;
    for( int i=0; i<rayiter; i++ )
    {
        if( abs(h)<precis||t>maxd ) continue;//break;
        t += h;
	    vec2 res = map( ro+rd*t );
        h = res.x;
	    m = res.y;
    }

    return vec2( t, m );
}

vec3 calcNormal( in vec3 pos )
{
	vec3 eps = vec3( 0.0001, 0.0, 0.0 );
	vec3 nor = vec3(
	    map(pos+eps.xyy).x - map(pos-eps.xyy).x,
	    map(pos+eps.yxy).x - map(pos-eps.yxy).x,
	    map(pos+eps.yyx).x - map(pos-eps.yyx).x );
	return normalize(nor);
}

vec3 shade( in vec3 ro, in vec3 rd ) {
	vec2 tm = castRay(ro, rd, dmax);
	if (tm.y >= 0.0) {
		vec3 n = calcNormal(ro + tm.x * rd);
		float fog = exp(-tm.x*tm.x*fogv);
		vec3 color = hue(tm.y) * 0.55 + 0.45;
		vec3 diffamb = (0.5*dot(n,L)+0.5) * color;
		vec3 R = 2.0*n*dot(n,L)-L;
		float spec = 0.2*pow(clamp(-dot(R, rd), 0.0, 1.0), 6.0);
		return fog * (diffamb + spec);
	} else {
		return vec3(1.0);
	}
}


vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
	return shade( ro, rd );
}

#ifndef RIFTRAY
void main(void) {
	
	const float yscl = 720.0;
	const float f = 900.0;
	
	vec2 uv = (gl_FragCoord.xy - 0.5*iResolution.xy) * yscl / iResolution.y;
	
	vec3 up = vec3(0.0, 1.0, 0.0);
	
	vec3 rz = normalize(tgt - cpos);
	vec3 rx = normalize(cross(rz,up));
	vec3 ry = cross(rx,rz);
	
	float thetax = 0.0;
	float thetay = 0.0;
	
	if (max(iMouse.x, iMouse.y) > 20.0) { 
		thetax = (iMouse.y - 0.5*iResolution.y) * 3.14/iResolution.y; 
		thetay = (iMouse.x - 0.5*iResolution.x) * -6.28/iResolution.x; 
	}

	float cx = cos(thetax);
	float sx = sin(thetax);
	float cy = cos(thetay);
	float sy = sin(thetay);
	
	mat3 Rx = mat3(1.0, 0.0, 0.0, 
				   0.0, cx, sx,
				   0.0, -sx, cx);
	
	mat3 Ry = mat3(cy, 0.0, -sy,
				   0.0, 1.0, 0.0,
				   sy, 0.0, cy);

	mat3 R = mat3(rx,ry,rz);
	mat3 Rt = mat3(rx.x, ry.x, rz.x,
				   rx.y, ry.y, rz.y,
				   rx.z, ry.z, rz.z);

	vec3 rd = R*Rx*Ry*normalize(vec3(uv, f));
	
	vec3 ro = tgt + R*Rx*Ry*Rt*(cpos-tgt);

	gl_FragColor.xyz = getSceneColor(ro, rd);
	
}
#endif
