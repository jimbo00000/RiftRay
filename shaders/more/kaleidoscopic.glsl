// Kaleidoscopic Journey
//
// Mikael Hvidtfeldt Christensen
// @SyntopiaDK
//
// License:
// Creative Commons Attribution
// http://creativecommons.org/licenses/by/3.0/

// @var title Kaleidoscopic Journey
// @var author Syntopia
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/XsX3z7

// @var eyePos 0.0 1.2 0.0

// Decrease this for better performance
#define Iterations 12
#define MaxSteps 30
#define MinimumDistance 0.001
#define normalDistance     0.0002
#define ColorIterations 9
#define PI 3.141592
#define Scale 2.0
#define FieldOfView 1.0
#define Jitter 0.05
#define FudgeFactor 1.0

#define Ambient 0.28452
#define Diffuse 0.57378
#define Specular 0.07272
#define LightDir vec3(1.0,1.0,-0.65048)
#define LightColor vec3(1.0,0.666667,0.0)
#define LightDir2 vec3(1.0,-0.62886,1.0)
#define LightColor2 vec3(0.596078,0.635294,1.0)

float time = iGlobalTime + 38.0;

vec2 rotate(vec2 v, float a) {
	return vec2(cos(a)*v.x + sin(a)*v.y, -sin(a)*v.x + cos(a)*v.y);
}

// Two light source + env light
vec3 getLight(in vec3 color, in vec3 normal, in vec3 dir) {
	vec3 lightDir = normalize(LightDir);
	float specular = pow(max(0.0,dot(lightDir,-reflect(lightDir, normal))),20.0); // Phong
	float diffuse = max(0.0,dot(-normal, lightDir)); // Lambertian
	
	vec3 lightDir2 = normalize(LightDir2);
	float specular2 = pow(max(0.0,dot(lightDir2,-reflect(lightDir2, normal))),20.0); // Phong
	float diffuse2 = max(0.0,dot(-normal, lightDir2)); // Lambertian
	
	return
		 //textureCube(iChannel0, reflect(dir, normal)).xyz*Specular+
		(Specular*specular)*LightColor+(diffuse*Diffuse)*(LightColor*color) +
		(Specular*specular2)*LightColor2+(diffuse2*Diffuse)*(LightColor2*color);
}

// Geometric orbit trap. Creates the 'cube' look.
float trap(vec3 p){
	return  length(p.x-0.5-0.5*sin(time/10.0)); // <- cube forms 
	//return  length(p.x-1.0); 
	//return length(p.xz-vec2(1.0,1.0))-0.05; // <- tube forms
	//return length(p); // <- no trap
}

vec3 offset = vec3(1.0+0.2*(cos(time/5.7)),0.3+0.1*(cos(time/1.7)),1.).xzy;

// DE: Infinitely tiled Kaleidoscopic IFS. 
//
// For more info on KIFS, see: 
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/
float DE(in vec3 z)
{	
	// Folding 'tiling' of 3D space;
	z  = abs(1.0-mod(z,2.0));
	
	float d = 1000.0;
	float r;
	for (int n = 0; n < Iterations; n++) {
		z.xz = rotate(z.xz, time/18.0);
		
		// This is octahedral symmetry,
		// with some 'abs' functions thrown in for good measure.
		if (z.x+z.y<0.0) z.xy = -z.yx;
		z = abs(z);
		if (z.x+z.z<0.0) z.xz = -z.zx;
		z = abs(z);
		if (z.x-z.y<0.0) z.xy = z.yx;
		z = abs(z);
		if (z.x-z.z<0.0) z.xz = z.zx;
		z = z*Scale - offset*(Scale-1.0);
		z.yz = rotate(z.yz, -time/18.0);
		
		d = min(d, trap(z) * pow(Scale, -float(n+1)));
	}
	return d;
}

// Finite difference normal
vec3 getNormal(in vec3 pos) {
	vec3 e = vec3(0.0,normalDistance,0.0);

	return normalize(vec3(
			DE(pos+e.yxx)-DE(pos-e.yxx),
			DE(pos+e.xyx)-DE(pos-e.xyx),
			DE(pos+e.xxy)-DE(pos-e.xxy)));
}

// Solid color with a little bit of normal :-)
vec3 getColor(vec3 normal, vec3 pos) {
	return mix(vec3(1.0),abs(normal),0.3); 
}

// Filmic tone mapping:
// http://filmicgames.com/archives/75
vec3 toneMap(in vec3 c) {
	c=pow(c,vec3(2.0));
	vec3 x = max(vec3(0.),c-vec3(0.004));
	c = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
	return c;
}

// Pseudo-random number
// From: lumina.sourceforge.net/Tutorials/Noise.html
float rand(vec2 co){
	return fract(cos(dot(co,vec2(4.898,7.23))) * 23421.631);
}

vec4 rayMarch(in vec3 from, in vec3 dir) {
	// Add some noise to prevent banding
	float totalDistance = Jitter*rand(gl_FragCoord.xy+vec2(time));
	
	float distance;
	int steps = 0;
	vec3 pos;
	for (int i=0; i < MaxSteps; i++) {
		pos = from + totalDistance * dir;
		distance = DE(pos)*FudgeFactor;
		totalDistance += distance;
		if (distance < MinimumDistance) break;
		steps = i;
	}
	
	// 'AO' is based on number of steps.
	// Try to smooth the count, to combat banding.
	float smoothStep = float(steps) + distance/MinimumDistance;
	float ao = 1.0-smoothStep/float(MaxSteps);

	// Since our distance field is not signed,
    // backstep when calc'ing normal
	vec3 normal = getNormal(pos-dir*normalDistance*3.0);	

	vec3 color = getColor(normal, pos);	
	vec3 light = getLight(color, normal, dir);
	return vec4(toneMap((color*Ambient+light)*ao),1.0);
}

vec3 getSceneColor( in vec3 camPos, in vec3 rayDir )
{
	return rayMarch(camPos, rayDir).xyz;
}

#ifndef RIFTRAY
void main(void)
{
	float angle = time/5.0;
	
	// Camera position (eye), and camera target
	vec3 camPos = 0.5*time*vec3(1.0,0.0,0.0);
	vec3 target = camPos + vec3(1.0,0.5*cos(time),0.5*sin(0.4*time));
	vec3 camUp  = vec3(0.0,cos(angle),sin(angle));
	
	// Calculate orthonormal camera reference system
	vec3 camDir   = normalize(target-camPos); // direction for center ray
	camUp = normalize(camUp-dot(camDir,camUp)*camDir); // orthogonalize
	vec3 camRight = normalize(cross(camDir,camUp));
	
	vec2 coord =-1.0+2.0*gl_FragCoord.xy/iResolution.xy;
	coord.x *= iResolution.x/iResolution.y;
	
	// Get direction for this pixel
	vec3 rayDir = normalize(camDir + (coord.x*camRight + coord.y*camUp)*FieldOfView);
	
	vec3 col = getSceneColor( camPos, rayDir );
	gl_FragColor = vec4(col, 1.0);
}
#endif

