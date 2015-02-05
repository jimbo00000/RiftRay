// Space Jewels. December 2014
// https://www.shadertoy.com/view/llX3zr

// @var title Space Jewels
// @var author Dave_Hoskins
// @var url https://www.shadertoy.com/view/llX3zr

// @var tex0 tex09.jpg

// @var float maxNum 1.1 0.1 4.0 0.0005
uniform float maxNum;
// @var float maxArg .03 0.0 1.0 0.00005
uniform float maxArg;

//--------------------------------------------------------------------------
#define SUN_COLOUR vec3(1., .9, .85)
#define FOG_COLOUR vec3(0.07, 0.05, 0.05)

vec3 CSize;
vec4 aStack[2];
vec4 dStack[2];

//----------------------------------------------------------------------------------------
float Hash(vec2 p)
{
	p  = fract(p * vec2(5.3983, 5.4427));
    p += dot(p.yx, p.xy + vec2(21.5351, 14.3137));
	return fract(p.x * p.y * 95.4337);
}

//----------------------------------------------------------------------------------------
vec3 Colour( vec3 p)
{
	float col	= 0.0;
    float r2	= dot(p,p);
		
	for( int i=0; i < 12;i++ )
	{
		vec3 p1= 2.0 * clamp(p, -CSize, CSize)-p;
		col += abs(p.z-p1.z);
		p = p1;
		r2 = dot(p,p);
		float k = max((maxNum)/(r2), maxArg);
		p *= k;
	}
	return (0.5+0.5*sin(col*vec3(1.647,-1.0,4.9)));
}

//--------------------------------------------------------------------------

float Map( vec3 p )
{
	float scale = 1.0;
	
	for( int i=0; i < 12;i++ )
	{
		p = 2.0*clamp(p, -CSize, CSize) - p;
		float r2 = dot(p,p);
		float k = max((maxNum)/(r2), maxArg);
		p     *= k;
		scale *= k;
	}
	float l = length(p.xy);
	float rxy = l - 4.0;
	float n = l * p.z;
	rxy = max(rxy, -(n) / (length(p))-.1);
	return (rxy) / abs(scale);
}



//--------------------------------------------------------------------------
float Shadow( in vec3 ro, in vec3 rd)
{
	float res = 1.0;
    float t = 0.05;
	float h;
	
    for (int i = 0; i < 6; i++)
	{
		h = Map( ro + rd*t );
		res = min(7.0*h / t, res);
		t += h+.01;
	}
    return max(res, 0.0);
}

//--------------------------------------------------------------------------
vec3 DoLighting(in vec3 mat, in vec3 pos, in vec3 normal, in vec3 eyeDir, in float d)
{
    vec3 sunLight  = normalize( vec3(  0.5, 0.2,  0.3 ) );
	float sh = Shadow(pos,  sunLight);
    // Light surface with 'sun'...
	vec3 col = mat * SUN_COLOUR*(max(dot(sunLight,normal), 0.0)) *sh;
    //col += mat * vec3(0.1, .0, .0)*(max(dot(-sunLight,normal), 0.0));
    
    normal = reflect(eyeDir, normal); // Specular...
    col += pow(max(dot(sunLight, normal), 0.0), 25.0)  * SUN_COLOUR * 1.5 *sh;
    // Abmient..
    col += mat * .2 * max(normal.z, 0.0);
    col = mix(FOG_COLOUR,col, min(exp(-d*d*.05), 1.0));
    
	return col;
}


//--------------------------------------------------------------------------
vec3 GetNormal(vec3 p, float sphereR)
{
	vec2 eps = vec2(sphereR*.5, 0.0);
	return normalize( vec3(
           Map(p+eps.xyy) - Map(p-eps.xyy),
           Map(p+eps.yxy) - Map(p-eps.yxy),
           Map(p+eps.yyx) - Map(p-eps.yyx) ) );
}

//--------------------------------------------------------------------------
float SphereRadius(in float t)
{
	if (t< 1.4) t= (1.4-t) * 4.5;
	t = t*0.04;
	return max(t*t, 16.0/iResolution.x);
}

//--------------------------------------------------------------------------
float Scene(in vec3 rO, in vec3 rD)
{
    //float t = 0.0;
	float t = .1 * Hash(gl_FragCoord.xy*fract(iGlobalTime));
	float  alphaAcc = 0.0;
	vec3 p = vec3(0.0);
    int hits = 0;

	for( int j=0; j < 80; j++ )
	{
		if (hits == 8 || alphaAcc >= 1.0 || t > 10.0) break;
		p = rO + t*rD;
		float sphereR = SphereRadius(t);
		float h = Map(p);
        // Is it within the sphere?...
		if( h < sphereR)
		{
			// Accumulate the alphas with the scoop of geometry from the sphere...
            // Think of it as an expanding ice-cream scoop flying out of the camera! 
			float alpha = (1.0 - alphaAcc) * min(((sphereR-h) / sphereR), 1.0);
			// put it on the 2 stacks, alpha and distance...
			aStack[1].yzw = aStack[1].xyz; aStack[1].x = aStack[0].w;
			aStack[0].yzw = aStack[0].xyz; aStack[0].x = alpha;
			dStack[1].yzw = dStack[1].xyz; dStack[1].x = dStack[0].w;
			dStack[0].yzw = dStack[0].xyz; dStack[0].x = t;
			alphaAcc += alpha;
			hits++;
		}
		t +=  h*.85+t*.001;
        
	}
	
	return clamp(alphaAcc, 0.0, 1.0);
}


//--------------------------------------------------------------------------
vec3 PostEffects(vec3 rgb, vec2 xy)
{
	// Gamma first...
	rgb = pow(rgb, vec3(0.45));

	// Then...
	#define CONTRAST 1.3
	#define SATURATION 1.3
	#define BRIGHTNESS 1.2
	rgb = mix(vec3(.5), mix(vec3(dot(vec3(.2125, .7154, .0721), rgb*BRIGHTNESS)), rgb*BRIGHTNESS, SATURATION), CONTRAST);

	// Vignette...
	rgb *= .5+0.5*pow(180.0*xy.x*xy.y*(1.0-xy.x)*(1.0-xy.y), 0.3 );	

	return clamp(rgb, 0.0, 1.0);
}

//--------------------------------------------------------------------------
vec3 TexCube( sampler2D sam, in vec3 p, in vec3 n )
{
	vec3 x = texture2D( sam, p.yz ).xzy;
	vec3 y = texture2D( sam, p.zx ).xyz;
	vec3 z = texture2D( sam, p.xy ).yzx;
	return (x*abs(n.x) + y*abs(n.y) + z*abs(n.z))/(abs(n.x)+abs(n.y)+abs(n.z));
}

//--------------------------------------------------------------------------
vec3 Albedo(vec3 pos, vec3 nor)
{
    vec3 col = TexCube(iChannel0, pos*.1, nor).zxy;
    col *= Colour(pos);
    return col;
}


//--------------------------------------------------------------------------
vec3 CameraPath( float t )
{
    vec3 p = vec3(-13.0 +3.4 * sin(t),-0.+4.5 * cos(t),-1.1+.3 * sin(2.3*t+2.0) );
	return p;
} 
    
vec3 getSceneColor(in vec3 cameraPos, in vec3 dir)
{
	cameraPos = cameraPos.xzy;
	dir = dir.xzy;
	float m = 0.;
	float gTime = ((iGlobalTime+26.)*.2+m);
    CSize = vec3(.808, .99-sin((gTime+35.0)*.5)*.3, 1.151-sin((gTime+16.0)*.78)*.3);
    cameraPos += CameraPath(gTime + 0.0);
	vec3 col = vec3(.0);
	
    for (int i = 0; i <2; i++)
    {
		dStack[i] = vec4(-20.0);
    }
	float alpha = Scene(cameraPos, dir);
	
    
    // Render both stacks...
    for (int s = 0; s < 2; s++)
    {
        for (int i = 0; i < 4; i++)
        {
            float d = dStack[s][i];
            if (d < 0.0) continue;
            float sphereR = SphereRadius(d);
            vec3 pos = cameraPos + dir * d;
            vec3 normal = GetNormal(pos, sphereR);
            vec3 alb = Albedo(pos, normal);
            col += DoLighting(alb, pos, normal, dir, d)* aStack[s][i];
        }
    }
    // Fill in the rest with fog...
    col += FOG_COLOUR *  (1.0-alpha);

	//col = PostEffects(col, xy) * smoothstep(.0, 2.0, iGlobalTime);
	col = PostEffects(col, vec2(0.25)) * smoothstep(.0, 2.0, iGlobalTime);
    return col;
}

//--------------------------------------------------------------------------
#ifndef RIFTRAY
void main(void)
{
	float m = (iMouse.x/iResolution.x)*20.0;
	float gTime = ((iGlobalTime+26.)*.2+m);
    vec2 xy = gl_FragCoord.xy / iResolution.xy;
	vec2 uv = (-1.0 + 2.0 * xy) * vec2(iResolution.x/iResolution.y,1.0);

    // Animate...
    CSize = vec3(.808, .99-sin((gTime+35.0)*.5)*.3, 1.151-sin((gTime+16.0)*.78)*.3);
	
	vec3 cameraPos 	= CameraPath(gTime + 0.0);
	vec3 camTarget 	= vec3 (-12., .0, -2.0);

	vec3 cw = normalize(camTarget-cameraPos);
	vec3 cp = vec3(0.0, 0.0,1.0);
	vec3 cu = normalize(cross(cw,cp));
	vec3 cv = cross(cu,cw);
	vec3 dir = normalize(uv.x*cu + uv.y*cv + 1.1*cw);
	vec3 col = getSceneColor(cameraPos, dir);
	gl_FragColor=vec4(col,1.0);
}
#endif
//--------------------------------------------------------------------------