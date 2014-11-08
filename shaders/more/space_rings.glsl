/*by mu6k, Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

 Rotating rings in space. Use mouse to rotate the camera.

 The distance function creates rings by subtracting a smaller radius cylinder with
 a larger radius cylinder. The result is then intersected with a plane.
 This is inside a for loop so that you get more rings. The space is rotated at
 every iteration.

 Sometimes the rings line up and form a plane. This happens when the rotation for
 each axis is 2*k*pi where k is an integer. This will cause 0 rotation on every iteration.

 For the flare, a ray is traced from the center of the camera to determine how much the
 light source is occluded. This is used to scale and amplify the flare.

 Most of the code was taken from my older shader. https://www.shadertoy.com/view/Mss3WN

 21/06/2013:
 - published

 muuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuusk!*/

// @var title Space Rings
// @var author mu6k
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/4dXGDM

// @var headSize 2.65
// @var eyePos -0.094 2.922 -9.537

#define occlusion_enabled
#define occlusion_quality 12
//#define occlusion_preview

#define noise_use_smoothstep

#define light_color vec3(1.0,1.1,1.8)

#define background_color_0 vec3(.2,.4,.6)
#define background_color_1 vec3(.32,.2,.1)*4.0

#define object_color vec3(0.5,0.5,0.5)
#define object_count 10
#define object_speed_modifier 1.0

#define render_steps 50

//use sphere instead of cylinder, only 1 sqrt is needed per distance function
#define optimisation_0

//remove ambient light, it's barely visible anyways...
//#define optimisation_1

//only 2 axis rotation instead of 3
//#define optimisation_2

float hash(float x)
{
	return fract(sin(x*.0127863)*17143.321); //decent hash for noise generation
}

float hash(vec2 x)
{
	return fract(cos(dot(x.xy,vec2(2.31,53.21))*124.123)*412.0); 
}

float hashmix(float x0, float x1, float interp)
{
	x0 = hash(x0);
	x1 = hash(x1);
	#ifdef noise_use_smoothstep
	interp = smoothstep(0.0,1.0,interp);
	#endif
	return mix(x0,x1,interp);
}

float hashmix(vec2 p0, vec2 p1, vec2 interp)
{
	float v0 = hashmix(p0[0]+p0[1]*128.0,p1[0]+p0[1]*128.0,interp[0]);
	float v1 = hashmix(p0[0]+p1[1]*128.0,p1[0]+p1[1]*128.0,interp[0]);
	#ifdef noise_use_smoothstep
	interp = smoothstep(vec2(0.0),vec2(1.0),interp);
	#endif
	return mix(v0,v1,interp[1]);
}

float hashmix(vec3 p0, vec3 p1, vec3 interp)
{
	float v0 = hashmix(p0.xy+vec2(p0.z*143.0,0.0),p1.xy+vec2(p0.z*143.0,0.0),interp.xy);
	float v1 = hashmix(p0.xy+vec2(p1.z*143.0,0.0),p1.xy+vec2(p1.z*143.0,0.0),interp.xy);
	#ifdef noise_use_smoothstep
	interp = smoothstep(vec3(0.0),vec3(1.0),interp);
	#endif
	return mix(v0,v1,interp[2]);
}

float noise(vec3 p) // 3D noise
{
	vec3 pm = mod(p,1.0);
	vec3 pd = p-pm;
	return hashmix(pd,(pd+vec3(1.0,1.0,1.0)), pm);
}

vec3 cc(vec3 color, float factor,float factor2) // color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color,vec3(w)*factor,w*factor2);
}


vec3 rotate_z(vec3 v, float angle)
{
	float ca = cos(angle); float sa = sin(angle);
	return v*mat3(
		+ca, -sa, +.0,
		+sa, +ca, +.0,
		+.0, +.0,+1.0);
}

vec3 rotate_y(vec3 v, float angle)
{
	float ca = cos(angle); float sa = sin(angle);
	return v*mat3(
		+ca, +.0, -sa,
		+.0,+1.0, +.0,
		+sa, +.0, +ca);
}

vec3 rotate_x(vec3 v, float angle)
{
	float ca = cos(angle); float sa = sin(angle);
	return v*mat3(
		+1.0, +.0, +.0,
		+.0, +ca, -sa,
		+.0, +sa, +ca);
}

float dist(vec3 p)//distance function
{
	float d=1024.0;
	float t = iGlobalTime*object_speed_modifier+5.0;
	
	#ifdef optimisation_0
		float s = length(p);
	#endif
	
	for (int i=0; i<object_count; i++)
	{
		#ifndef optimisation_2
		p = rotate_z(p,t*.05);
		#endif
		p = rotate_y(p,t*.1);
		p = rotate_x(p,t*.075);
		
		float fi = float(i);
		
		#ifdef optimisation_0
			float c1 = s-(fi+1.0); //use sphere radius
			float c2 = s-(fi+1.6);
		#else
			float c1 = length(p.xz)-(fi+1.0); //2 cylinders
			float c2 = length(p.xz)-(fi+1.6);
		#endif
		
		float plane = length(abs(p.y))-0.2;
		//intersect(plane(subtract(cylinder2,cylinder1)))
		float shape = max(plane,max(c2,-c1)); 
		d = min(d,shape);
	}

	return d;
}

//!!modified
float occlusion(vec3 p, vec3 d, float e, float spd)//returns how much a point is visible from a given direction
{
	float occ = 1.0;
	p=p+d;
	for (int i=0; i<occlusion_quality; i++)
	{
		float lp = length(p);
		if(lp<1.0) //lightsource
		{
			break;
		}
		float dd = dist(p);
		p+=d*dd*spd;
		occ = min(occ,dd*e);
	}
	return max(.0,occ);
}

vec3 normal(vec3 p,float e) //returns the normal, uses the distance function
{
	float d=dist(p);
	return normalize(vec3(dist(p+vec3(e,0,0))-d,dist(p+vec3(0,e,0))-d,dist(p+vec3(0,0,e))-d));
}

vec3 light; //global variable that holds light direction

vec3 background(vec3 p,vec3 d)//render background
{
	float i = d.y*.5+.5;
	
	float dust = noise(d*3.0); 
	float stars = noise(d*100.0);
	stars = pow(stars,80.0);
	stars*=2.0;
	
	float w = dust*.5+.5;
	vec3 color = mix(background_color_0,background_color_1,i);
	
	color*=w;
	color += vec3(stars);

	return color*.6;
}

vec3 object_material(vec3 p, vec3 d) //computes the material for the object
{
	vec3 color = object_color;
	vec3 n = normal(p,0.12);
	vec3 r = reflect(d,n);	
	
	float reflectance = dot(d,r)*.5+.5;reflectance=pow(reflectance,2.0);
	float diffuse = dot(light,n)*.5+.5; diffuse = max(.0,diffuse);
	
	
	float post_light_occlusion = occlusion(p-light*.5,light,2.0,.3);
	float post_light = pow(dot(r,normalize(-p))*.50+.5,40.0)*post_light_occlusion;
	
	#ifdef occlusion_enabled
		#ifndef optimisation_1
		float oa = occlusion(p,n,0.4,1.0)*.5+.5;
		#else
		float oa=.0;
		#endif
		float od = post_light_occlusion*.5+.5;
		float os = occlusion(p,r,2.0,1.0)*.95+.05;
	#else
		float oa=1.0;
		float od=1.0;
		float os=1.0;
	#endif
	
	#ifndef occlusion_preview
		color = 
		color*oa*.1 + //ambient
		color*diffuse*od*.7 + //diffuse
		light_color*post_light+background(p,r)*os*reflectance*.5; //reflection
	#else
		color=vec3(oa*od*os);
	#endif
	
	//return vec3(post_light_occlusion);
	
	return color;
}

#define offset1 4.9
#define offset2 3.6


vec3 getSceneColor( in vec3 p, in vec3 d )
{
	vec3 sp = p;
	
	
	//and action!
	float dd;
	vec3 color;
	for (int i=0; i<render_steps; i++) //raymarch
	{
		dd = dist(p);
		p+=d*dd*.7;
		if (dd<.001 || dd>10.0) break;
	}
	
	//setup light source for shading
	light = normalize(-p);
	
	if (dd<0.1) //close enough
		color = object_material(p,d);
	else
		color = background(sp,d);
	
	//prepare ray to check lightsource occlusion
	vec3 md = vec3(vec2(.0,.0),1.0);
	md.z -= length(md)*.7; //lens distort
	md = normalize(md);
	//md = rotate_x(md,mouse.y*9.0+offset1);
	//md = rotate_y(md,mouse.x*9.0+offset2);
	
	float post_light_occlusion = occlusion(sp-md,md,4.0,1.0);
	
	float post_light = pow(dot(d,normalize(-sp))*.50+.5,256.0/(post_light_occlusion*1.0));
	//float post_light_scifi = pow((dot(d,normalize(-sp))*.2+1.0/(1.0+abs(uv.y)))*.6,4.0/(post_light_occlusion*1.0));
	
	color+=light_color*(post_light);
	//color+=light_color*(post_light+post_light_scifi);
	
	//post procesing
	color *=.85;
	color = mix(color,color*color,0.3);
	//color -= hash(color.xy+uv.xy)*.015;
	//color -= length(uv)*.1;
	color =cc(color,.5,.6);
	return color;
}

#ifndef RIFTRAY
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 mouse = vec3(iMouse.xy/iResolution.xy - 0.5,iMouse.z-.5);
	
	float t = iGlobalTime*.5*object_speed_modifier + 2.0;
	
	//setup the camera
	vec3 p = vec3(.0,0.0,-4.9);
	p = rotate_x(p,mouse.y*9.0+offset1);
	p = rotate_y(p,mouse.x*9.0+offset2);
	vec3 d = vec3(uv,1.0);
	d.z -= length(d)*.7; //lens distort
	d = normalize(d);
	d = rotate_x(d,mouse.y*9.0+offset1);
	d = rotate_y(d,mouse.x*9.0+offset2);
	
	vec3 color = getSceneColor( p, d );
	gl_FragColor = vec4(color,1.0);
}
#endif
