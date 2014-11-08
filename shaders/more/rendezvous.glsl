// Rendezvous. By David Hoskins. Jan 2014.
// A Kleinian thingy, breathing, and with pumping arteries!

// @var title Rendezvous
// @var author David Hoskins
// @var url https://www.shadertoy.com/view/ldjGDw

// @var headSize 0.5
// @var eyePos -0.025 1.580 -3.910

// Enable the next line for pseudo anti-aliasing of colours,
// reduces pixel flicker on detailed areas. Might not be worth the GPU cost.
// #define AVERAGE_COLOURS 

// Add crude reflections..
// #define REFLECTIONS

// For red/cyan 3D. Red on the left.
// #define STEREO


#define CSize  vec3(.808, .8, 1.137)
#define FogColour vec3(.02, .015, .01)

vec3  lightPos;
float intensity;

//----------------------------------------------------------------------------------------
float Hash( float n )
{
    return fract(sin(n)*43758.5453123);
}

//----------------------------------------------------------------------------------------
float Noise( in float x )
{
    float p = floor(x);
    float f = fract(x);
    f = f*f*(3.0-2.0*f);
    return mix(Hash(p), Hash(p+1.0), f);
}

//----------------------------------------------------------------------------------------
float Map( vec3 p )
{
	float scale = 1.0;
	float add = sin(iGlobalTime)*.2+.1;

	for( int i=0; i < 7;i++ )
	{
		p = 2.0*clamp(p, -CSize, CSize) - p;
		float r2 = dot(p,p);
		float k = max((1.1)/(r2), 1.0);
		p     *= k;
		scale *= k;
	}
	float l = length(p.xy);
	float rxy = l - 4.0;
	float n = l * p.z;
	rxy = max(rxy, -(n) / (length(p))-.07+sin(iGlobalTime*2.0+p.x+p.y+23.5*p.z)*.02);
	return (rxy) / abs(scale);
}

//----------------------------------------------------------------------------------------
vec3 Colour( vec3 p)
{
	float col	= 0.0;
	float r2	= dot(p,p);
	float add = sin(iGlobalTime)*.2+.1;
	
	for( int i=0; i < 10;i++ )
	{
		vec3 p1= 2.0 * clamp(p, -CSize, CSize)-p;
		col += abs(p.z-p1.z);
		p = p1;
		r2 = dot(p,p);
		float k = max((1.1)/(r2), 1.0);
		p *= k;
	}
	return (0.5+0.5*sin(col*vec3(.647,-1.0,4.9)))*.75 + .15;
}

//----------------------------------------------------------------------------------------
vec3 ColourAve(vec3 pos, float t)
{
	vec3 eps = vec3(.001*t,0.0,0.0);
	return (Colour(pos) + 
			Colour(pos-eps.xyy) + Colour(pos+eps.xyy) +
			Colour(pos-eps.yxy) + Colour(pos+eps.yxy) +
			Colour(pos-eps.yyx) + Colour(pos+eps.yyx) ) / 7.0;	
}	

//----------------------------------------------------------------------------------------
float RayMarch( in vec3 ro, in vec3 rd )
{
	float precis = 0.0005;
    float h		 = precis*.2;
    float t		 = 0.1;
	float res	 = 200.0;
	bool hit	 = false;
	// If I rearrange the loop in a more logical way,
	// I get a black screen on Windows.
    for( int i = 0; i < 145; i++ )
    {
		if (!hit && t < 8.0)
		{
			h = Map(ro + rd * t);
			if (h < precis)
			{
				res = t;
				hit = true;
			}
			t += h * .83;
		}
    }
	
    return res;
}

//----------------------------------------------------------------------------------------
float Shadow(in vec3 ro, in vec3 rd, float dist)
{
	float res = 1.0;
    float t = 0.01;
	float h = 0.0;
    
	for (int i = 0; i < 20; i++)
	{
		// Don't run past the point light source...
		if(t < dist)
		{
			h = Map(ro + rd * t);
			res = min(h / t, res);
			t += 0.0045 + h*.25;
		}
	}
    return clamp(res, 0.0, 1.0);
}

//----------------------------------------------------------------------------------------
vec3 Normal(in vec3 pos, in float t)
{
	vec2  eps = vec2(t*t*.01,0.0);
	vec3 nor = vec3(Map(pos+eps.xyy) - Map(pos-eps.xyy),
					Map(pos+eps.yxy) - Map(pos-eps.yxy),
					Map(pos+eps.yyx) - Map(pos-eps.yyx));
	return normalize(nor);
}

//----------------------------------------------------------------------------------------
float LightGlow(vec3 light, vec3 ray, float t)
{
	float ret = 0.0;
	if (length(light) < t)
	{
		light = normalize(light);
		ret = pow(max(dot(light, ray), 0.0), 2000.0)*.5;
		float a = atan(light.x - ray.x, light.z - ray.z);
		ret = (1.0+(sin(a*10.0-iGlobalTime*4.3)+sin(a*13.141+iGlobalTime*3.141)))*(sqrt(ret))*.05+ret;
		ret *= 3.0;
	}
		
	return ret;
}

//----------------------------------------------------------------------------------------
vec3 RenderPosition(vec3 pos, vec3 ray, vec3 nor, float t)
{
	vec3 col = vec3(0.0);				
	vec3 lPos  = lightPos-pos;
	float lightDist = length(lPos);
	vec3 lightDir  = normalize(lPos);

	float bri = max( dot( lightDir, nor ), 0.0) * intensity;
	float spe = max(dot(reflect(ray, nor), lightDir), 0.0);
	float amb = max(-nor.z*.015, 0.0);
	float sha = clamp(Shadow(pos+nor*0.005, lightDir, lightDist) / max(lightDist-2.0, 0.1), 0.0, 1.0);
	bri = amb + bri * sha;

	#ifdef AVERAGE_COLOURS
	col =  ColourAve(pos, t);
	#else
	col =  Colour(pos);
	#endif
	col = col * bri + pow(spe, 18.0) * sha * .3;
	
	return col;
}

vec3 getSceneColor( in vec3 origin, in vec3 ray )
{
	float time = sin(1.6+iGlobalTime*.05)*12.5;
    // camera
	float height = (smoothstep(9.4, 11.5, abs(time))*.5);
	origin += vec3( 1.2, time+1.0, 2.5+height).xzy;
	vec3 target = vec3(.0+sin(time), 0.0, 2.5-height*4.0);
	lightPos = origin+vec3(-0.56-cos(time*2.0+2.8)*.3, -1.4, .24+cos(time*2.0+1.5)*.3);
	intensity = .8+.3*Noise(iGlobalTime*5.0);

	// Invert vertical axis
	origin *= vec3(1,1,-1);
	ray *= vec3(1,1,-1);
	lightPos *= vec3(1,1,-1);

	// Switch the y and z directions.
	origin = origin.xzy;
	ray = ray.xzy;
	lightPos = lightPos.xzy;


	vec3 col = vec3(0.0);
	float t = 0.0;
	t = RayMarch(origin, ray);

	if(t < 199.0)
	{
		vec3 pos = origin + t * ray;
		vec3 nor = Normal(pos, t);
		col = RenderPosition(pos, ray, nor, t);
		
		#ifdef REFLECTIONS
		vec3 ray2    = reflect(ray, nor);
		vec3 origin2 = pos + nor*.01;
		float d = RayMarch(origin2, ray2);
		if(d < 199.0)
		{
			pos = origin2 + d * ray2;
			nor = Normal(pos, d);
			col += RenderPosition(pos, ray, nor, d) * .2;
		}
		#endif
	}
	
	// Effects...
	col = mix(FogColour, col, exp(-.6*max(t-3.0, 0.0)));
	col = pow(col, vec3(.45));
	//col *= pow(50.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.25);
	col += LightGlow(lightPos-origin, ray, t) * intensity;

	#ifdef STEREO	
	col *= vec3( isRed, 1.0-isRed, 1.0-isRed );	
	#endif	
	

	return col;	
}

#ifndef RIFTRAY
//----------------------------------------------------------------------------------------
void main(void)
{
	vec2 q = gl_FragCoord.xy/iResolution.xy;
    vec2 p = -1.0+2.0*q;
	p.x *= iResolution.x/iResolution.y;
	
	#ifdef STEREO
	float isRed = mod(gl_FragCoord.x + mod(gl_FragCoord.y,2.0),2.0);
	#endif

	float time = sin(1.6+iGlobalTime*.05 + iMouse.x*.005)*12.5;
    // camera
	float height = (smoothstep(9.4, 11.5, abs(time))*.5);
	vec3 origin = vec3( 1.2, time+1.0, 2.5+height);
	vec3 target = vec3(.0+sin(time), 0.0, 2.5-height*4.0);
	lightPos = origin+vec3(-0.56-cos(time*2.0+2.8)*.3, -1.4, .24+cos(time*2.0+1.5)*.3);
	intensity = .8+.3*Noise(iGlobalTime*5.0);
	
	vec3 cw = normalize( target-origin);
	vec3 cp = normalize(vec3(0.0, 0.0, 1.));
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = cross(cu,cw);
	vec3 ray = normalize( p.x*cu + p.y*cv + 2.6*cw );	
	#ifdef STEREO
	origin += .008*cu*isRed; // move camera to the right - the rd vector is still good
	#endif	

	vec3 col = getSceneColor( origin, ray );
	gl_FragColor=vec4(clamp(col, 0.0, 1.0),1.0);
}
#endif
