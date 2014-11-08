// Desert Morning
// Uploaded by ESpitz in 2013-Jul-8
// A second attempt at procedural terrain generation - based on
// techniques developed by Inigo Quilez.

// @var title Desert Morning
// @var author ESpitz
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/lsf3zB

// @var headSize 0.7

#ifdef GL_ES
precision mediump float;
#endif

const float dMax = 28.0;

// Simple 2d noise algorithm contributed by Trisomie21 (Thanks!)
float snoise( vec2 p ) {
	vec2 f = fract(p);
	p = floor(p);
	float v = p.x+p.y*1000.0;
	vec4 r = vec4(v, v+1.0, v+1000.0, v+1001.0);
	r = fract(100000.0*sin(r*.001));
	f = f*f*(3.0-2.0*f);
	return 2.0*(mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y))-1.0;
}

float terrain( vec2 p, int octaves ) {	
	float h = 0.0; // height
	float w = 0.5; // octave weight
	float m = 0.4; // octave multiplier
	for (int i=0; i<12; i++) {
		if (i<octaves) {
			h += w * snoise((p * m));
		}
		else break;
		w *= 0.5;
		m *= 2.0;
	}
	return h;
}

vec2 map( vec3 p, int octaves ) {
	
	float dMin = dMax; // nearest intersection
	float d; // depth
	float mID = -1.0; // material ID
	
	// mountains
	float h = terrain(p.xz, octaves);
	h += smoothstep(-0.6, 1.2, h); // exaggerate the higher terrain
	d = p.y - h;
	if (d<dMin) { 
		dMin = d;
		mID = 0.0;
	}
	// sand dunes
	if (h<0.5) {
		float s = 0.5 * terrain(p.xz*0.9, 3); // minimal detail needed for sand
		d = p.y - s;	
		if (d<dMin) { 
			dMin = d;
			mID = 1.0;
		}
	}
	return vec2(dMin, mID);
}

vec2 castRay( vec3 ro, vec3 rd, int octaves) {
	const float p = 0.001; // precision
	float t = 0.0; // distance
	float h = p * 2.0; // step
	float m = -1.0;
	for (int i=0; i<36; i++) {
		if (abs(h)>p || t<dMax ) {
			t += h; // next step
			vec2 res = map(ro + rd*t, octaves); // next intersection
			h = res.x; // distance
			m = res.y; // material
		} 
		else break;
	}
	if (t>dMax) m = -1.0; // if no intersection, material ID is -1.0;
	return vec2(t, m);
}

vec3 calcNormal( vec3 p, int octaves) {
	const vec3 eps = vec3(0.002, 0.0, 0.0);
	return normalize( vec3(map(p+eps.xyy, octaves).x - map(p-eps.xyy, octaves).x,
			       map(p+eps.yxy, octaves).x - map(p-eps.yxy, octaves).x,
			       map(p+eps.yyx, octaves).x - map(p-eps.yyx, octaves).x) );
}

float shadows( vec3 ro, vec3 rd, float tMax, float k, int octaves ) {
    float res = 1.0;
	float t = 0.1;
	for(int i=0; i<22; i++) {
        if (t<tMax) {
			float h = map(ro + rd*t, octaves).x;
        	res = min( res, k*h/t );
        	t += h;
		}
		else break;
    }
    return clamp(res, 0.2, 1.0);
}

vec3 render( vec3 ro, vec3 rd ) {
	const int geoLOD = 4;
	
	vec3 color = vec3(0.3,0.4,0.6); // base color is sky color
	vec2 res = castRay(ro, rd, geoLOD);
	
	vec3 lPos = normalize( vec3(1.0, 0.5, 0.0) ); // light position
	vec3 lCol = vec3(1.0, 0.9, 0.7); // yellowish light
	
	vec3 pos = ro + rd*res.x; // world position
	
	// mat -1 = background/sky
	if (res.y < -0.5) {
		float sun = clamp(dot(rd,lPos),0.0,1.0); // sun orb
		color += 0.8 * lCol * sun*sun;
		// cheap clouds (hopefully, no one will notice)
		float c = snoise(pos.xz * 0.05); 
		color = mix( color, vec3(1.0, 1.0, 0.8), smoothstep(0.0, 1.5, c) );
		return color;
	}
	
	int norLOD = int(max(2.0, 12.0-11.0*res.x/dMax)); // normal LOD decreases with distance
	vec3 nor = calcNormal(pos, norLOD); // terrain normals
	
	// mat 0 = terrain
	if (res.y>-0.5 && res.y<0.5) {
		
		// base rock formation colors
		color = mix( vec3(0.4, 0.1, 0.0), vec3(0.7, 0.6, 0.3), step(0.9, nor.y) );
		
		// layer noise (to produdce lighter color bands of rock)
		float n = 0.5*(snoise(pos.xy*vec2(2.0, 15.0))+1.0);
		// rock layers should show through where nomals are NOT straight up
		color = mix( vec3(0.6, 0.5, 0.4), color, n*smoothstep(0.0, 0.7, nor.y) ); 
		
		// add in lighting and shadows
		float lAmb = clamp( 0.5 + 0.5 * nor.y, 0.0, 1.0); // ambient
		float lDif = clamp( dot( nor, lPos ), 0.0, 2.0); // diffuse
		
		// shadow octaves should match geometry octaves used in initial ray cast
		if (lDif>0.05) lDif *= shadows(pos, lPos, 8.0, 12.0, geoLOD);
		
		color += (0.4*lAmb) * lCol;
		color *= (1.8*lDif) * lCol;	
	}
	// mat 1 = sand
	if (res.y>0.5) {
		// base sand and rock color
		color = mix( vec3(0.3, 0.2, 0.0), vec3(0.7, 0.6, 0.3), nor.y );
			
		// add in lighting and shadows
		float lAmb = clamp( 0.5 + 0.5 * nor.y, 0.0, 1.0); // ambient
		float lDif = clamp( dot( nor, lPos ), 0.0, 2.0); // diffuse
		
		// shadow octaves should match geometry octaves used in initial ray cast
		if (lDif>0.05) lDif *= shadows(pos, lPos, 8.0, 12.0, geoLOD);
		
		color += (0.4*lAmb) * lCol;
		color *= (1.8*lDif) * lCol;
	}
	
	// fog
	float fog = exp(-0.005 * res.x*res.x); // exponentioal fog equation
	color = mix(vec3(0.3,0.3,0.35), color, fog); // add fog in distance
		
	return color;
}

vec3 getSceneColor( in vec3 cPos, in vec3 rd )
{
	
	// render
	return render(cPos, rd);
}

#ifndef RIFTRAY

void main( void ) {

	vec2 pos = 2.0 * ( gl_FragCoord.xy / iResolution.xy ) - 1.0; // bound screen coords to [0, 1]
	pos.x *= iResolution.x / iResolution.y; // correct for aspect ratio

	// camera
	float x = 0.0 + (0.5*iGlobalTime);
	float y = 0.0;
	float z = 0.0 + sin(0.1*iGlobalTime);
	vec3 cPos = vec3(x, y, z); // position
	cPos.y = terrain(cPos.xz, 1) + 1.5;
	
	const vec3 cUp = vec3(0., 1., 0.); // up 
	vec3 cLook = vec3(cPos.x + 1.0, cPos.y*0.7, 0.0); // lookAt
	
	// camera matrix
	vec3 ww = normalize( cLook-cPos );
	vec3 uu = normalize( cross(ww, cUp) );
	vec3 vv = normalize( cross(uu, ww) );
	
	vec3 rd = normalize( pos.x*uu + pos.y*vv + 2.0*ww );
	
	// render
	vec3 color = getSceneColor(cPos, rd);
	
	gl_FragColor = vec4( color, 1.0 );
}
#endif
