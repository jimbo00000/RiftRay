// tribute to Escher's "Depth"
// @simesgreen
// v1.1 - fewer interations, fixed lighting

// @var title Depth
// @var author simesgreen
// @var url https://www.shadertoy.com/view/Mdl3zn

// @var eyePos 1.0 1.0 0.0

// CSG operations
float _union(float a, float b)
{
    return min(a, b);
}

float _union(float a, float b, inout float m, float nm)
{
	bool closer = (a < b);
	m = closer ? m : nm;
	return closer ? a : b;
}

float intersect(float a, float b)
{
    return max(a, b);
}

float difference(float a, float b)
{
    return max(a, -b);
}

// primitive functions
// these all return the distance to the surface from a given point

float box(vec3 p, vec3 b)
{
	vec3 d = abs(p) - b;
	return min(max(d.x,max(d.y,d.z)),0.0) +
    	   length(max(d,0.0));
}

float sphere(vec3 p, float r)
{
    return length(p) - r;
}


// transforms
vec3 rotateX(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    vec3 r;
    r.x = p.x;
    r.y = ca*p.y - sa*p.z;
    r.z = sa*p.y + ca*p.z;
    return r;
}

vec3 rotateY(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    vec3 r;
    r.x = ca*p.x + sa*p.z;
    r.y = p.y;
    r.z = -sa*p.x + ca*p.z;
    return r;
}

// distance to scene
float scene(vec3 p, inout float m)
{
   float d;
   m = 0.0; // material

   p.z -= iGlobalTime;
	
   // repeat
   p += vec3(1.5);
   p = mod(p, 3.0);
   p -= vec3(1.5);
	
   p.x = abs(p.x);	// mirror in x
	
   // body
//   d = sphere(p, 1.0);
   //d = sphere(p*vec3(4.0, 4.0, 1.0), 1.0)*0.25;
	float s = 4.0 + smoothstep(0.5, -1.0, p.z)*2.0;	// taper
	d = sphere(p*vec3(s, s, 1.0), 1.0) / 6.0;

   // mouth
   d = difference(d, box(p - vec3(0.0, 0.0, 1.0), vec3(0.2, 0.01, 0.2)));
   //d = difference(d, sphere(p*vec3(1.0, 10.0, 1.0) - vec3(0.0, 0.0, 0.95), 0.15)*0.1);

   // fins
   float f;
   f = box(p, vec3(1.2, 0.02, 0.2));
   f = _union(f, box(p, vec3(0.02, 1.2, 0.2)));
   f = intersect(f, sphere(p - vec3(0.0, 0.0, -1.8), 2.0));
   f = difference(f, sphere(p - vec3(0, 0, -0.5), 0.5));
   f = difference(f, sphere(p - vec3(1.2-0.4, 0, -0.2-0.7), 0.8));
   //f = difference(f, sphere(p - vec3(-1.2+0.4, 0, -0.2-0.7), 0.8));
	
   //d = _union(f, d);
   d = _union(d, f, m, 3.0);

   // tail
   f = sphere(p*vec3(1.0, 1.0, 0.5) + vec3(0, 0, 0.5), 0.25);
   f = intersect(box(p + vec3(0.0, 0.0, 1.0), vec3(0.02, 0.5, 0.5)), f);
   f = difference(f, sphere(p*vec3(1.0, 1.0, 0.5) + vec3(0, 0, 0.75), 0.2));
   //d = _union(d, f);
   d = _union(d, f, m, 3.0);
	
   //d = _union(d, box(p + vec3(0.0, 0.0, 1.0), vec3(0.02, 0.25, 0.25)));
   //d = difference(d, sphere(p + vec3(0.0, 0.0, 1.25), 0.25));

   // eyes
   d = _union(d, sphere(p-vec3(0.08, 0.08, 0.85), 0.06), m, 1.0);
   d = _union(d, sphere(p-vec3(0.1, 0.1, 0.88), 0.03), m, 2.0);

   //d = _union(d, sphere(p-vec3(-0.08, 0.08, 0.85), 0.06), m, 1.0);
   //d = _union(d, sphere(p-vec3(-0.1, 0.1, 0.88), 0.03), m, 2.0);
		
   return d;
}

// calculate scene normal
vec3 sceneNormal( in vec3 pos )
{
    float eps = 0.001;
    vec3 n;
	float m;
	float d = scene(pos, m);
    n.x = scene( vec3(pos.x+eps, pos.y, pos.z), m ) - d;
    n.y = scene( vec3(pos.x, pos.y+eps, pos.z), m ) - d;
    n.z = scene( vec3(pos.x, pos.y, pos.z+eps), m ) - d;
    return normalize(n);
}

// ambient occlusion approximation
float ambientOcclusion(vec3 p, vec3 n)
{
    const int steps = 3;
    const float delta = 0.5;

    float a = 0.0;
    float weight = 1.0;
	float m;
    for(int i=1; i<=steps; i++) {
        float d = (float(i) / float(steps)) * delta;
        a += weight*(d - scene(p + n*d, m));
        weight *= 0.5;
    }
    return clamp(1.0 - a, 0.0, 1.0);
}

// smooth pulse
float pulse(float a, float b, float w, float x)
{
    return smoothstep(a, a + w, x) - smoothstep(b - w, b, x);
}

// lighting
vec3 shade(vec3 pos, vec3 n, vec3 eyePos, float m)
{
    const vec3 l = vec3(0.577, 0.577, 0.577);
    const float shininess = 100.0;

    //vec3 l = normalize(lightPos - pos);
    vec3 v = normalize(eyePos - pos);
    vec3 h = normalize(v + l);
    float ndotl = dot(n, l);
    float spec = max(0.0, pow(max(0.0, dot(n, h)), shininess)) * float(ndotl > 0.0);
    //float diff = max(0.0, ndotl);
    float diff = 0.5+0.5*ndotl;

    float fresnel = pow(1.0 - dot(n, v), 5.0);
    //float ao = ambientOcclusion(pos, n);
	
	//float edge = scene(pos+n*0.05) - scene(pos);
	//edge *= 50.0;
	//edge = smoothstep(0.0, 0.02, edge);

    vec3 color = vec3(0.9, 0.5, 0.1);
#if 1
	// lines
	pos.z -= iGlobalTime;
	
   	pos += vec3(1.5);
   	pos = mod(pos, 3.0);
   	pos -= vec3(1.5);

  	// stripes
    //float sx = pulse(0.0, 0.5, 0.1, fract(pos.t*15.0));
	float sx = pulse(0.0, 0.5, 0.1, fract(atan(pos.y, pos.x)*2.0));

    //float w = 0.5;
    float w = smoothstep(0.4, -0.5, pos.y)*1.0;
    float sz = 1.0 - pulse(0.0, w, 0.1, fract(pos.z*15.0)) * ((w > 0.1) ? 1.0 : 0.0);

    //vec3 color = mix(vec3(1.0), vec3(0.9, 0.5, 0.1), sx) * sz;
	if (m==0.0) {
		// body
		color = mix(vec3(1.0), color, sx) * sz;
		//color *= sz;	
	} else if (m==3.0) {
		// fins
		color *= pulse(0.0, 1.0, 0.1, fract(pos.x*10.0));
		color *= pulse(0.0, 1.0, 0.1, fract(pos.y*10.0));
		pos.z += pos.x*pos.x*0.3 + pos.y*pos.y*0.3;
		color *= pulse(0.0, 1.0, 0.1, fract(pos.z*10.0));
	}
#else

#endif
	// eyes
	if (m==1.0) {
		color = vec3(1.0, 1.0, 1.0);
	} else if (m==2.0) {
		color = vec3(0.0, 0.0, 0.0);
	}
	
    //return vec3(diff*ao) * color + vec3(spec + fresnel*0.5);
//    return vec3(diff*ao) * color + vec3(spec);
    return vec3(diff)*color + vec3(spec + fresnel*0.5);
//	return n*0.5+0.5;
//	return vec3(edge);
//  return vec3(diff);
//  return vec3(ao);
//  return vec3(fresnel);
}

// trace ray using sphere tracing
vec3 trace(vec3 ro, vec3 rd, out bool hit, inout float m)
{
    const int maxSteps = 64;
    const float hitThreshold = 0.01;
	const float minStep = 0.0001;
    hit = false;
    vec3 pos = ro;
    for(int i=0; i<maxSteps; i++)
    {
		if (!hit) {
			float d = scene(pos, m);
			//d = max(d, minStep);
			if (d < hitThreshold) {
				hit = true;
				//return pos;
			}
			pos += d*rd;
		}
    }
    return pos;
}




vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    // trace ray
    bool hit;
	float m;
    vec3 pos = trace(ro, rd, hit, m);

	const vec3 fogColor = vec3(1.0, 1.0, 0.8);
    vec3 rgb = fogColor;
	if(hit) {
        // calc normal
        vec3 n = sceneNormal(pos);
        // shade
        rgb = shade(pos, n, ro, m);
    }

 	// fog
   	float d = length(pos)*0.07;
   	float f = exp(-d*d);

   	// vignetting
   	//rgb *= 0.5+0.5*smoothstep(2.0, 0.5, dot(pixel, pixel));
	
   	return mix(fogColor, rgb, f);	
}

#ifndef RIFTRAY
void main(void)
{
    vec2 pixel = (gl_FragCoord.xy / iResolution.xy)*2.0-1.0;

    // compute ray origin and direction
    float asp = iResolution.x / iResolution.y;
    vec3 rd = normalize(vec3(asp*pixel.x, pixel.y, -3.0));
    vec3 ro = vec3(0.0, 0.0, 2.5);

	float rx = -0.5 + (iMouse.y / iResolution.y)*3.0;	
	float ry = 0.3 -(iMouse.x / iResolution.x)*6.0;
	ry += iGlobalTime*0.1;
	
    ro = rotateX(ro, rx);
    ro = rotateY(ro, ry);
    rd = rotateX(rd, rx);
    rd = rotateY(rd, ry);

    vec3 col = getSceneColor( ro, rd );
   	gl_FragColor=vec4(col, 1.0);
}
#endif
