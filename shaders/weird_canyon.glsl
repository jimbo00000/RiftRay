// Created by Stephane Cuillerdier - Aiekick/2015
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// Tuned via XShade (http://www.funparadigm.com/xshade/)

/* Weird Canyon
it was an attempt in my terrain experiment study to do an antelope canyon
but failed to sculpt it realistic, and failed to do a good lighting.
the sand is weird also ^^
I must deepen the learning of light technics:)
the screenshot is at 44.93s
the mouse axis y control the vertical angle of the cam
*/ 

// @var title Weird Canyon
// @var author aiekick
// @var url https://www.shadertoy.com/view/XtjSRm

// @var eyePos -8.81 1.76 3.39

#define mPi 3.14159
#define m2Pi 6.28318

float dstepf = 0.0;

const vec2 NoiseVar = vec2(950.,200.);
    
const vec2 RMPrec = vec2(.3, 0.0001); 
const vec2 DPrec = vec2(0.001, 40.); 

float random(float p) {return fract(sin(p)*NoiseVar.x);}
float mynoise(vec2 p) {return random(p.x + p.y*NoiseVar.y);}
vec2 sw(vec2 p) {return vec2( floor(p.x) , floor(p.y) );}
vec2 se(vec2 p) {return vec2( ceil(p.x)  , floor(p.y) );}
vec2 nw(vec2 p) {return vec2( floor(p.x) , ceil(p.y)  );}
vec2 ne(vec2 p) {return vec2( ceil(p.x)  , ceil(p.y)  );}
float snoise(vec2 p) {
  	vec2 inter = smoothstep(0., 1., fract(p));
  	float s = mix(mynoise(sw(p)), mynoise(se(p)), inter.x);
  	float n = mix(mynoise(nw(p)), mynoise(ne(p)), inter.x);
  	return mix(s, n, inter.y);
}

//https://www.shadertoy.com/view/llsXzB
//https://en.wikipedia.org/wiki/Oren%E2%80%93Nayar_reflectance_model
vec3 OrenNayarLightModel(vec3 rd, vec3 ld, vec3 n, float albedo)
{
	vec3 col = vec3(0.);
	float RDdotN = dot(-rd, n);
	float NdotLD = dot(n, ld);
    float aRDN = acos(RDdotN);
	float aNLD = acos(NdotLD);
	float mu = 5.; // roughness
	float A = 1.-.5*mu*mu/(mu*mu+0.57);
    float B = .45*mu*mu/(mu*mu+0.09);
	float alpha = max(aRDN, aNLD);
	float beta = min(aRDN, aNLD);
	float e0 = 4.8;
	col = vec3(albedo / mPi) * cos(aNLD) * (A + ( B * max(0.,cos(aRDN - aNLD)) * sin(alpha) * tan(beta)))*e0;
	return col;
}

//https://www.shadertoy.com/view/Xl23Rc
vec3 strate(vec2 uv)
{
    vec3 col1 = vec3(.94,.7,.25);
    vec3 col2 = vec3(.91,.67,.11);
    float y = uv.y+.85*sin(-uv.x);
    y/=.85;
    float r = sin(25.*y)+cos(16.*y)+cos(19.*y);
    vec3 col = mix(col1, col2, r);
    return col;
}

// shane code
float Voronesque( in vec3 p )
{
    vec3 i  = floor(p + dot(p, vec3(0.333333)) );  p -= i - dot(i, vec3(0.166666)) ;
    vec3 i1 = step(0., p-p.yzx), i2 = max(i1, 1.0-i1.zxy); i1 = min(i1, 1.0-i1.zxy);    
    vec3 p1 = p - i1 + 0.166666, p2 = p - i2 + 0.333333, p3 = p - 0.5;
    vec3 rnd = vec3(7, 157, 113); 
    vec4 v = max(0.5 - vec4(dot(p, p), dot(p1, p1), dot(p2, p2), dot(p3, p3)), 0.);
    vec4 d = vec4( dot(i, rnd), dot(i + i1, rnd), dot(i + i2, rnd), dot(i + 1., rnd) ); 
    d = fract(sin(d)*262144.)*v*2.; 
    v.x = max(d.x, d.y), v.y = max(d.z, d.w); 
    return max(v.x, v.y); 
}

// used to compute campath and plane deformation along z
float sinPath(vec3 p, float dec){return 6.4 * sin(p.z * .33 + dec);}

vec2 map(vec3 p)
{
    vec2 res = vec2(0.);

	dstepf += .0007;
	
	float pathLeft = sinPath(p, 0.);
	float pathRight = sinPath(p, 19.5);
	
	float voro = Voronesque(p/12.) * 7.6;
	float strateDisp = dot(strate(p.zy), vec3(.043));
	float strateNoise = snoise(p.zy/.5) *.2;
    float microNoise = snoise(p.zy/.01) *.02;
	
    float disp =  voro - strateDisp - strateNoise + microNoise;
    
	float bottom = p.y + 2. - snoise(p.xz) * .38- snoise(p.xz/.039) * .05;
	res = vec2(bottom, 1.);
	
	float left = p.x + pathLeft + 2. + disp;
	if (left < res.x)
		res = vec2(left, 2.);
	
	float right = 2. - p.x - pathRight + disp;
	if (right < res.x)
		res = vec2(right, 3.);
	
	return res;
}

vec3 nor( vec3 pos, float prec )
{
    vec2 e = vec2( prec, 0. );
    vec3 n = vec3(
		map(pos+e.xyy).x - map(pos-e.xyy).x,
		map(pos+e.yxy).x - map(pos-e.yxy).x,
		map(pos+e.yyx).x - map(pos-e.yyx).x );
    return normalize(n);
}

vec3 cam(vec2 uv, vec3 ro, vec3 cu, vec3 cv)
{
	vec3 rov = normalize(cv-ro);
    vec3 u =  normalize(cross(cu, rov));
    vec3 v =  normalize(cross(rov, u));
    vec3 rd = normalize(rov + u*uv.x + v*uv.y);
    return rd;
}

// from iq code
float softshadow( in vec3 ro, in vec3 rd, in float mint, in float tmax )
{
	float res = 1.0;
    float t = mint;
    for( int i=0; i<16; i++ )
    {
		float h = map( ro + rd*t ).x;
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
    for( int i=0; i<5; i++ )
    {
        float hr = 0.01 + 0.12*float(i)/4.0;
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos ).x;
        occ += -(dd-hr)*sca;
        sca *= 0.95;
    }
    return clamp( 1.0 - 3.0*occ, 0.0, 1.0 );    
}

vec3 lighting(vec3 col, vec3 p, vec3 n, vec3 rd, vec3 ref, float t) // lighting    
{
	// from iq code
	float occ = calcAO( p, n );
	vec3  lig = normalize( vec3(0., 1., 0.) );
	float amb = clamp( 0.5+0.5*n.y, 0.0, 1.0 );
	float dif = clamp( dot( n, lig ), 0.0, 1.0 );
	float bac = clamp( dot( n, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-p.y,0.0,1.0);
	float dom = smoothstep( -0.1, 0.1, ref.y );
	float fre = pow( clamp(1.0+dot(n,rd),0.0,1.0), 2.0 );
	float spe = pow(clamp( dot( ref, lig ), 0.0, 1.0 ),16.0);
        
	dif *= softshadow( p, lig, 0.02, 2.5 );
	dom *= softshadow( p, ref, 0.02, 2.5 );

	vec3 brdf = vec3(0.0);
	brdf += 1.20*dif*vec3(1.00,0.90,0.60);
	brdf += 1.20*spe*vec3(1.00,0.90,0.60)*dif;
	brdf += 0.30*amb*vec3(0.50,0.70,1.00)*occ;
	brdf += 0.40*dom*vec3(0.50,0.70,1.00)*occ;
	brdf += 0.30*bac*vec3(0.25,0.25,0.25)*occ;
	brdf += 0.40*fre*vec3(1.00,1.00,1.00)*occ;
	brdf += 0.02;
	col = col*brdf;

	col = mix( col, vec3(0.8,0.9,1.0), 1.0-exp( -0.0005*t*t ) );
	
	return col;
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    vec4 f;
    vec3 d = vec3(0.);
    vec3 p = ro+rd*d.x;
    vec2 s = vec2(DPrec.y,0.);
	
    for(int i=0;i<200;i++)
	{      
		if(s.x<DPrec.x||s.x>DPrec.y) break;
        s = map(p);
		s.x *= (s.x>DPrec.x?RMPrec.x:RMPrec.y);;
		d.x += s.x;
        p = ro+rd*d.x;
   	}
	
	if (d.x<DPrec.y)
	{
		vec3 n = nor(p, .05);
        if(s.y < 3.5)
		{
			f.rgb = OrenNayarLightModel(rd, rd, n, 0.3);	
			f.rgb += vec3(.94, .71, .53);		
		}
		if (s.y < 1.5) // bottom
        {
			f.rgb = OrenNayarLightModel(reflect(rd,n), rd, n, .75);	
			f.rgb += vec3(.98,.76,.24);
		}
		
		
		f.rgb = lighting(f.rgb, p, n, rd, rd, d.x); // lighting    
   	}
    else
    {
        //https://www.shadertoy.com/view/4ssXW2
        vec3 sun = vec3(1.0, .88, .75);;
        float v = pow(1.0-max(rd.y,0.0),10.);
		f.rgb = vec3(v*sun.x*0.42+.04, v*sun.y*0.4+0.09, v*sun.z*0.4+.17);
    }

	// vigneting  https://www.shadertoy.com/view/MsXGWr
    //f.rgb *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.25 );	 

    return f.rgb;
}

#ifndef RIFTRAY
void mainImage( out vec4 f, in vec2 g )
{
    vec2 si = iResolution.xy;
	vec2 uv = (g+g-si)/min(si.x, si.y);
	vec2 q = g/si;
    
    float t = iGlobalTime*3.;
	
	vec4 gp = vec4(0.); //uGamePad360;
	
    vec3 cu = vec3(0,1,0);
    vec3 ro = vec3(0., gp.y + 1., t);
	vec3 cv = vec3(gp.zw,.08); 
    if (iMouse.z>0.) 
    {
        cv.y = iMouse.y/iResolution.y*.8 - .5;
        cv.x = iMouse.x/iResolution.x*.8 - .5;
    }
    float cx = (sinPath(ro + cv, 0.) + sinPath(ro + cv, 19.5))/2.;
	ro.x -= cx;
	vec3 rd = cam(uv, ro, cu, ro + cv);
	vec3 col = getSceneColor(ro, rd);
    f = vec4(col.rgb,1.0);
}
#endif
