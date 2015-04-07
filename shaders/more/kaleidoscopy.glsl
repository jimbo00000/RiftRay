// Ben Quantock 2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// @var title Kaleidoscopy
// @var author TekF
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/lssSWM

// @var eyePos -0.488 0.73 -3.6



float DistanceField( vec3 pos )
{
	//pos = abs(pos);
	//return max( pos.x, max( pos.y, pos.z ) )-1.0;
	
//	return (length(pos-vec3(0,-1,0))-2.0 + sin(sqrt(pow(atan(length(pos.xz),pos.y),2.0)+1.0)*20.0-iGlobalTime)/20.0)*.707;
	
	//return (pos.y + sin(pos.x*1.0)*sin(pos.z*1.0)/1.0)*.7;
	
	// fractalise space, rotate then mirror on axis
	const float tau = 6.2831853;
	const float phi = 1.61803398875;
	
	vec3 p = pos;
	float T = .3*iGlobalTime;
	float a0 = .2*phi*(T+sin(T));//.5*tau/phi;
	float a1 = .05*phi*(T-sin(T))*phi;//tau/phi;
	float c0 = cos(a0);
	vec2 s0 = vec2(1,-1)*sin(a0);
	float c1 = cos(a1);
	vec2 s1 = vec2(1,-1)*sin(a1);
	
	const int n = 32;
	float o = 1.0;

	// centre on the first mirror
	p.x -= o;
	
	// and the second
	p.y += o*s0.x;
	p.z -= o*s1.x;
	
	for ( int i=0; i < n; i++ )
	{
		p.x = abs(p.x+o)-o;
		p.xy = p.xy*c0 + p.yx*s0;
		p.xz = p.xz*c1 + p.zx*s1;
		//o = o/sqrt(2.0);
		//o = o*.8;
		//o = max( o*.8 - .02, o*.7 );
		//o = o-1.0/float(n);
		//o = o*(float(n-i-1)/float(n-i)); // same as^
		//o = o*.8*(float(n-i-1)/float(n-i));
		o = o*.9*(float(n-i-1)/float(n-i));
		//o = o/phi;
	}


	// spheres	
//	return length(p)-.15;

	// cubes
//	return max(abs(p.x),max(abs(p.y),abs(p.z)))-.14;

	// octahedra	
//	return (abs(p.x)+abs(p.y)+abs(p.z))/sqrt(3.0)-.1;
	
	// spikes! Precision issues, but wow!
//	return (abs(p.z)*.05+length(p.xy)*sqrt(1.0-.05*.05))-.1;

	// stretched octahedra
//	vec3 s = vec3(1,1,.3);	p = abs(p)*s/length(s);	return dot(p,vec3(1))-.2;
	
	// tetrahedra
	return max( max( p.x+p.y+p.z, -p.x-p.y+p.z), max( p.x-p.y-p.z, -p.x+p.y-p.z ) )/sqrt(3.0) -.1;

	// offset tetrahedra, for more variety
//	p -= .1;
//	return max( max( p.x+p.y+p.z, -p.x-p.y+p.z), max( p.x-p.y-p.z, -p.x+p.y-p.z ) )/sqrt(3.0) - .1;
}


vec3 Sky( vec3 ray )
{
	return mix( vec3(.8), vec3(0), exp2(-(1.0/max(ray.y,.01))*vec3(.4,.6,1.0)) );
}


vec3 Shade( vec3 pos, vec3 ray, vec3 normal, vec3 lightDir, vec3 lightCol, float distance )
{
	vec3 ambient = mix( vec3(.03,.05,.08), vec3(.1), (-normal.y+1.0) ); // ambient
	// ambient occlusion, based on my DF Lighting: https://www.shadertoy.com/view/XdBGW3
	float aoRange = distance/20.0;
	float occlusion = max( 0.0, 1.0 - DistanceField( pos + normal*aoRange )/aoRange ); // can be > 1.0
	occlusion = exp2( -3.0*pow(occlusion,2.0) ); // tweak the curve
	ambient *= occlusion;

	vec3 light = lightCol*max(.0,dot(normal,lightDir));
	light += ambient;
	
	vec3 h = normalize(lightDir-ray);
	vec3 specular = lightCol*pow(max(.0,dot(normal,h)), 100.0)*100.0/32.0;
	
	vec3 rray = reflect(ray,normal);
	vec3 reflection = Sky( rray );
	// prevent sparkles in heavily occluded areas
	reflection *= occlusion;
	// specular occlusion, adjust the divisor for the gradient we expect
	occlusion = max( 0.0, 1.0 - DistanceField( pos + rray*aoRange )/(aoRange*dot(rray,normal)) ); // can be > 1.0
	occlusion = exp2( -2.0*pow(occlusion,2.0) ); // tweak the curve
	reflection *= occlusion; // could fire an additional ray for more accurate results
	
	float fresnel = pow( 1.0+dot(normal,ray), 5.0 );
	fresnel = mix( .0, .5, fresnel );
	
	vec3 albedo = vec3(1);//.6,.3,.15);//.8,.02,0);
	
	return mix( light*albedo, reflection, fresnel ) + specular;
}




// Isosurface Renderer

float traceStart = .1; // set these for tighter bounds for more accuracy
float traceEnd = 25.0;
float Trace( vec3 pos, vec3 ray )
{
	float t = traceStart;
	float h;
	for( int i=0; i < 60; i++ )
	{
		h = DistanceField( pos+t*ray );
		if ( h < .001 || t > traceEnd )
			break;
		t = t+h;
	}
	
	if ( t > traceEnd )//|| h > .001 )
		return 0.0;
	
	return t;
}

float TraceMin( vec3 pos, vec3 ray )
{
	float Min = traceEnd;
	float t = traceStart;
	float h;
	for( int i=0; i < 60; i++ )
	{
		h = DistanceField( pos+t*ray );
		Min = min(h,Min);
		if ( /*h < .001 ||*/ t > traceEnd )
			break;
		t = t+max(h,.1);
	}
	
	return Min;
}

vec3 Normal( vec3 pos, vec3 ray, float distance )
{
	// in theory we should be able to get a good gradient using just 4 points
//	vec2 d = vec2(-1,1) * .01;
	vec2 d = vec2(-1,1) * .5 * distance / iResolution.x;
	vec3 p0 = pos+d.xxx; // tetrahedral offsets
	vec3 p1 = pos+d.xyy;
	vec3 p2 = pos+d.yxy;
	vec3 p3 = pos+d.yyx;
	
	float f0 = DistanceField(p0);
	float f1 = DistanceField(p1);
	float f2 = DistanceField(p2);
	float f3 = DistanceField(p3);
	
	vec3 grad = p0*f0+p1*f1+p2*f2+p3*f3 - pos*(f0+f1+f2+f3);
	
	// prevent normals pointing away from camera (caused by precision errors)
	float gdr = dot ( grad, ray );
	grad -= max(.0,gdr)*ray;
	
	return normalize(grad);
}


// Camera

vec3 Ray( float zoom )
{
	return vec3( gl_FragCoord.xy-iResolution.xy*.5, iResolution.x*zoom );
}

vec3 Rotate( inout vec3 v, vec2 a )
{
	vec4 cs = vec4( cos(a.x), sin(a.x), cos(a.y), sin(a.y) );
	
	v.yz = v.yz*cs.x+v.zy*cs.y*vec2(-1,1);
	v.xz = v.xz*cs.z+v.zx*cs.w*vec2(1,-1);
	
	vec3 p;
	p.xz = vec2( -cs.w, -cs.z )*cs.x;
	p.y = cs.y;
	
	return p;
}


// Camera Effects

void BarrelDistortion( inout vec3 ray, float degree )
{
	// would love to get some disperson on this, but that means more rays
	ray.z /= degree;
	ray.z = ( ray.z*ray.z - dot(ray.xy,ray.xy) ); // fisheye
	ray.z = degree*sqrt(ray.z);
}

vec3 LensFlare( vec3 ray, vec3 light, float lightVisible )
{
	vec2 dirtuv = gl_FragCoord.xy/iResolution.x;
	
	float dirt = 1.0-texture2D( iChannel1, dirtuv ).r;
	
	float l = (dot(light,ray)*.5+.5)*lightVisible;
	
	return ((pow(l,40.0)+.04)*dirt*.1 + 1.0*pow(l,200.0))*vec3(1.05,1,.95);
}

vec3 getSceneColor( in vec3 pos, in vec3 ray )
{
	vec3 ray1 = Ray(1.0);
	BarrelDistortion( ray1, .5 );
	ray1 = normalize(ray1);
	vec3 localRay = ray1;
    
	vec3 col;

	vec3 lightDir = normalize(vec3(3,2,-1));
	
	float t = Trace( pos, ray );
	if ( t > .0 )
	{
		vec3 p = pos + ray*t;
		
		// shadow test
		float s = Trace( p, lightDir );
		
		vec3 n = Normal(p, ray, t);
		col = Shade( p, ray, n, lightDir, (s>.0)?vec3(0):vec3(1.1,1,.9), t );
		
		// fog
		float f = 40.0;
		col = mix( vec3(.8), col, exp2(-(t-3.0)*vec3(.4,.6,1.0)/f) );
	}
	else
	{
		col = Sky( ray );
	}
	
	// lens flare
	float sun = TraceMin( pos, lightDir );
	col += LensFlare( ray, lightDir, smoothstep(-.5,.0,sun) );

	// vignetting:
	//col *= smoothstep( .5, .0, dot(localRay.xy,localRay.xy) );

	// compress bright colours, ( because bloom vanishes in vignette )
	vec3 c = (col-1.0);
	c = sqrt(c*c+.01); // soft abs
	col = mix(col,1.0-c,.48); // .5 = never saturate, .0 = linear
	
    return col;
}

#ifndef RIFTRAY
void main()
{
	vec3 ray = Ray(1.0);
	BarrelDistortion( ray, .5 );
	ray = normalize(ray);
	vec3 localRay = ray;

	vec2 mouse = iMouse.yx/iResolution.yx - .5;
	vec3 pos = 8.0*Rotate( ray, vec2(.2,-3.0+iGlobalTime*.1)+vec2(1.6,-6.3)*mouse );
	
    vec3 col = getSceneColor( pos, ray );
    
	// grain
	vec2 grainuv = gl_FragCoord.xy + floor(iGlobalTime*60.0)*vec2(37,41);
	vec2 filmNoise = texture2D( iChannel0, .5*grainuv/iChannelResolution[0].xy ).rb;
	//col *= mix( vec3(1), mix(vec3(1,.5,0),vec3(0,.5,1),filmNoise.x), .1*filmNoise.y );

	
	gl_FragColor = vec4(pow(col,vec3(1.0/2.2)),1);
}
#endif
