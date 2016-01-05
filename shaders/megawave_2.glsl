// Created by Stephane Cuillerdier - Aiekick/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// @var title MegaWave 2
// @var author aiekick
// @var url https://www.shadertoy.com/view/ltjXWR
// @var license CC BY-NC-SA 3.0

const vec3 ligthDir = vec3(0.,1., 0.5);
const float mPi = 3.14159;
const float m2Pi = 6.28318;

float t = iGlobalTime;

float dstepf = 0.;

vec2 df(vec3 p)
{
	dstepf += 0.001;

	vec2 res = vec2(1000.);
	
	vec3 q;
	
	// mat 2
	q.x = cos(p.x);
	q.y = p.y * 5. - 25. + 10. * cos(p.x / 7. + t) + 10. * sin(p.z / 7. + t);
	q.z = cos(p.z);
	float sphere = length(q) - 1.;
	if (sphere < res.x)
		res = vec2(sphere, 2.);
	
	// mat 3
	q.x = cos(p.x);
	q.y = p.y * 5. + 25. + 10. * cos(p.x / 7. + t + mPi) + 10. * sin(p.z / 7. + t + mPi);
	q.z = cos(p.z);
	sphere = length(q) - 1.;
	if (sphere < res.x)
		res = vec2(sphere, 3.);
		
	return res;
}

vec3 nor( vec3 p, float prec )
{
    vec2 e = vec2( prec, 0. );
    vec3 n = vec3(
		df(p+e.xyy).x - df(p-e.xyy).x,
		df(p+e.yxy).x - df(p-e.yxy).x,
		df(p+e.yyx).x - df(p-e.yyx).x );
    return normalize(n);
}

// from iq code
float softshadow( in vec3 ro, in vec3 rd, in float mint, in float tmax )
{
	float res = 1.0;
    float t = mint;
    for( int i=0; i<200; i++ )
    {
		float h = df( ro + rd*t ).x;
        res = min( res, 8.0*h/t );
        t += clamp( h, 0.02, 0.10 );
        if( h<0.001 || t>tmax ) break;
    }
    return clamp( res, 0.0, 1.0 );
}

// from iq code
float calcAO( in vec3 pos, in vec3 nor )
{
	float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<10; i++ )
    {
        float hr = 0.01 + 0.12*float(i)/4.0;
        vec3 aopos =  nor * hr + pos;
        float dd = df( aopos ).x;
        occ += -(dd-hr)*sca;
        sca *= 0.95;
    }
    return clamp( 1.0 - 3.0*occ, 0.0, 1.0 );    
}


//--------------------------------------------------------------------------
// Grab all sky information for a given ray from camera
// from Dave Hoskins // https://www.shadertoy.com/view/Xsf3zX
vec3 GetSky(in vec3 rd, in vec3 sunDir, in vec3 sunCol)
{
	float sunAmount = max( dot( rd, sunDir), 0.0 );
	float v = pow(1.0-max(rd.y,0.0),6.);
	vec3  sky = mix(vec3(.1, .2, .3), vec3(.32, .32, .32), v);
	sky = sky + sunCol * sunAmount * sunAmount * .25;
	sky = sky + sunCol * min(pow(sunAmount, 800.0)*1.5, .3);
	return clamp(sky, 0.0, 1.0);
}

vec3 getSceneColor( in vec3 rayOrg, in vec3 rayDir )
{
    vec4 f = vec4(0.);
	vec2 s = vec2(0.01);
	float d = 0.;
	vec3 p = rayOrg + rayDir * d;
	float dMax = 80.;
	float sMin = 0.0001;
	
	float h = .8;
	
	for (float i=0.; i<250.; i++)
	{
		if (s.x<sMin || d>dMax) break;
		s = df(p);
		d += s.x * (s.x>0.001?0.25:0.001);
		p = rayOrg + rayDir * d;	
	}
	
	vec3 sky = GetSky(rayDir, ligthDir, vec3(h*5.));
	
	if (d<dMax)
	{
		vec3 n = nor(p, 0.001);
		
		// 	iq primitive shader : https://www.shadertoy.com/view/Xds3zN
		float r = mod( floor(5.0*p.z) + floor(5.0*p.x), 2.0);
		f.rgb = 0.4 + 0.1*r*vec3(1.0);

		// from iq
		float occ = calcAO( p, n );
		float amb = clamp( 0.5+0.5*n.y, 0.0, 1.0 );
		float dif = clamp( dot( n, ligthDir ), 0.0, 1.0 );
		float spe = pow(clamp( dot( rayDir, ligthDir ), 0.0, 1.0 ),16.0);
		dif *= softshadow( p, ligthDir, 0.02, 50.);
		vec3 brdf = vec3(0.0);
		brdf += 1.20*dif*vec3(1.00,0.90,0.60);
		brdf += 1.20*spe*vec3(1.00,0.90,0.60)*dif;
		brdf += 0.30*amb*vec3(0.50,0.70,1.00)*occ;
		f.rgb *= brdf + dstepf;
		f.rgb = mix( f.rgb, sky, 1.0-exp( -0.001*d*d ) );
	}
	else
	{
		f.rgb = sky;
	}
    return f.rgb;
}

#ifndef RIFTRAY
void mainImage( out vec4 f, in vec2 g )
{	
	vec2 si = iResolution.xy;
    
	vec2 uv = (2.*g-si)/min(si.x, si.y);
	
	t = iGlobalTime;
	
	vec3 rayOrg = vec3(t,0.,t)  * 5.;
	vec3 camUp = vec3(0,1,0);
	vec3 camOrg = rayOrg + vec3(1,0,1);
	
	float fov = 0.5;
	vec3 axisZ = normalize(camOrg - rayOrg);
	vec3 axisX = normalize(cross(camUp, axisZ));
	vec3 axisY = normalize(cross(axisZ, axisX));
	vec3 rayDir = normalize(axisZ + fov * uv.x * axisX + fov * uv.y * axisY);
	vec3 sc = getSceneColor(rayOrg, rayDir);
    f.rgb = sc;
}
#endif
