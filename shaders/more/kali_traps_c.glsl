/*	kali-traps by bergi in 2015
	
	License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License. 

	man, just showing ..
	try the other path as well ;)

	here's a quick qt app that helped with the parameter scan
	https://github.com/defgsus/kaliset
	(though it looks quite different with webgl float precision 
     compared to intel floats..)

	It's really difficult to make a good shadertoy selfrunner,
	der Spass liegt im Explorieren dieser Menge.
	
	----- Interact with all the stuff below --------------
*/

// @var title kali-traps c
// @var author bergi
// @var url https://www.shadertoy.com/view/lll3WM

// @var eyePos -0.048 -0.059 0.037
// @var headSize 0.01

// @var float foc 0.245 -1.0 5.0 0.003
uniform float foc;

// 1 or 2
#define PATH 					1

// animation time
#if PATH == 1
float ti = 						iGlobalTime * 1. + 76.;
#else
float ti = 						iGlobalTime * 2. + 500.;
#endif

// slight set modifier
float modf = 					.5 + .5 * sin(ti / 31.);

	  vec3 KALI_PARAM = 		vec3(0.99-0.02*modf, 1., 1.01+0.02*modf);
const int  KALI_ITERS = 		29;

// max traces
const int  NUM_ITERS = 			30;
// multisample
const int  NUM_RAYS =			1;
// just for camera
const float PI = 3.14159265;

/** kali set as usual. 
	returns last magnitude step and average */
vec4 average;
float kali(in vec3 p)
{
    average = vec4(0.);
	float mag;
    for (int i=0; i<KALI_ITERS; ++i)
    {
        mag = dot(p, p);
        p = abs(p) / mag;
        average += vec4(p, mag);
        p -= KALI_PARAM;
    }
	average /= 32.;
    return mag;
}

// steps from pos along dir and samples the cloud
// stp is 1e-5 - 1e+?? :)
vec3 ray_color(vec3 pos, vec3 dir, float stp)
{
    vec3 p, col = vec3(0.);
	float t = 0.;
	for (int i=0; i<NUM_ITERS; ++i)
	{
		p = pos + t * dir;
		float d = kali(p);

		// define a surface and get trapped
        // well it's all just numbers and signs really
        // and trial-and-error
        // the 'trap' is: making d small when close to what-looks-good
		d = d*1.4-.3;
            //-d*1.3+1.;
        	//1.4 - abs(1.1 - d);
		
		// always step within a certain range
		t += max(0.0001, min(0.0004+0.01*stp, d )) * (stp + 3. * t);

		// some color
        d = max(-0.5, d-0.5);
		col += average.rgb * (.7+.3*sin(average.rgb*vec3(3,5,7)*2.9)) 
		// by distance to surface
            / (1. + d * d * 400. + t * t / stp * 1000.);
	}
    
    return clamp(col / float(NUM_ITERS) * 7., 0., 1.);
}

// by D. Hoskins https://www.shadertoy.com/view/XlfGWN
float hash(in vec2 uv)
{
	vec3 p  = fract(vec3(uv,ti) / vec3(3.07965, 7.1235, 4.998784));
    p += dot(p.xy, p.yx+19.19);
    return fract(p.x * p.y);
}

vec2 uv;
//float foc = 0.083;
vec3 getSceneColor(in vec3 pos, in vec3 dir)
{
	uv = (gl_FragCoord.xy - iResolution.xy*.5) / iResolution.y * 2.;
    vec3 col = vec3(0.);
    for (float i=0.; i<float(NUM_RAYS); ++i)
    {
        vec3 p = pos + dir * hash(uv*1114.+ti+i*17.) * (0.000002 + 0.001 * foc);
	
    	col += ray_color(p, dir, foc * (1. + 0.03 * i));
	}
    col /= float(NUM_RAYS);
    return col;
}

#ifndef RIFTRAY
void main()
{
    // frequency of slowness
    float phasemf = 0.12;
    float phasem = sin(ti*phasemf);
    float phasem2 = sin(ti*phasemf+PI/2.);
    
    // ray stepsize - or focus scale 
    foc = 0.083 + 0.08*phasem2;

    // camera circle phase
    float rti = 0.05 * (ti + phasem / phasemf * 0.95);
    
    if (iMouse.z > .5) {
        foc = pow(iMouse.y / iResolution.y, 2.)/6.;
    }

	uv = (gl_FragCoord.xy - iResolution.xy*.5) / iResolution.y * 2.;

#if PATH == 1    
	vec3 pos = (vec3(0.3359+0.007*modf, 0.1139+0.0064*modf, -0.001-0.001*phasem2) 
                + (.0032 + 0.003 * phasem2) * vec3(uv, 0.)
                + (0.113 + 0.005*modf) * vec3(sin(rti),cos(rti),0.) );
    
    vec3 dir = normalize(vec3(uv, 2.5-length(uv)/1.3)).xyz;
#else
    
    foc = foc * 3.4 + 0.01;
    
    vec3 pos = vec3(0.25 + .4*sin(rti), 0.1 * cos(rti), 0.),
    	 posl = vec3(0.25 + .21*pow(sin(rti),3.), 0.02 * cos(rti) + 0.*sin(rti*2.3), -.1);
    
	vec3 look = normalize(posl - pos);
	vec3 up = normalize(cross(vec3(0., 0., -1.), look));
	vec3 right = normalize(cross(look, up));
	//look = normalize(cross(up, right));
	mat3 dirm = mat3(right, up, look);
    // screen / near-plane
    pos += dirm * (0.05 + 0.04 * phasem2) * vec3(uv, 0)
        ;
    vec3 dir = dirm * normalize(vec3(uv, 2.5-length(uv)/1.3)).xyz;
    
#endif    

    // "pass" uv in as global
    vec3 col = getSceneColor(pos, dir);
	gl_FragColor = vec4(pow(col,vec3(1./1.9)), 1.);	
}
#endif
