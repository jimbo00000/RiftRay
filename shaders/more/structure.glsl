/*by mu6k, Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

 Long time no shadertoy :D . . . 
 Well chrome had this huge rendering bug where it didn't render any webpage at all
 so I downloaded a different version of chrome. Whatever, I've been busy with life . . .

 So a few words about this before I leave:
 
 I wanted to make a better occlusion function than the one I've been using before.
 The new one I wrote has 3 parameters, position, light position and light size. It 
 magically returns how much of the lightsource is visible from a given position.

 The occlusion is done by finding the minimum distance to geometry from a given
 line. The implementation is a two pass algorithm. First I try to find the minimum
 by taking fixed steps. I sample n points from the line to find the minimum. Secondly
 I try to find a better position by trying nearby positions on the line. It works well.

 Currenly there are 2 lightsource and for each lightsource the occlusion is run twice.
 One from the camera to determine how much the lightsource is visible and one from the
 shading function to determine how much the surface is shadowed.

 The geometry is generated using 4 boxes. They use domain repetition and are reflected
 vertically to give the impression that there is a lot more going on.

 There is also a floor and a ceiling.

 Lighting is your standard ambient + diffuse + specular. I scale down the diffuse and 
 specular components depending on how much the lightsource is visible from the surface.

 Currently this barely compiles here. You can easily tweak framerate and quality below.
 I managed to compile it with 3 lightsources. If you remove flare occlusion you can get a 
 decent framerate boost.

 One day I'll make this run faster, clean up the code and add comments. 
 Suggestions are welcome :D

 23/07/2013:
 - published

 28/07/2013:
 - cleaned up the code a bit

 05/09/2013:
 - fixed a bug in the specular component

 muuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuusk!*/

// @var title Structure
// @var author mu6k
// @var url https://www.shadertoy.com/view/XdfGzS

#define occlusion_enabled
#define occlusion_pass1_quality 15
#define occlusion_pass2_quality 6

#define noise_use_smoothstep

#define background_color_0 vec3(.2,.4,.6)
#define background_color_1 vec3(.32,.2,.1)*4.0

#define object_color vec3(0.5,0.5,0.5)
#define object_count 3
#define object_speed_modifier 1.0

#define light_count_ 2 
#define light_speed_modifier 2.0
#define flare_occlusion

#define render_steps 30

const int light_count = light_count_;
vec3 light_pos[light_count];
vec3 light_color[light_count];

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
	//float d=1.0-abs(p.y);
	
	float t = iGlobalTime*object_speed_modifier+5.0;
	p.x+=t;	
	p.z+=t*.4;

		float s = length(p);

	p.y=abs(p.y);
	
	p.y-=5.0;
	
	float d = 0.5-p.y;
	
	for (int i=0; i<10; i++)
	{
		float fi = float(i);
		
		p+=vec3(1.25-fi,0.0,1.75+fi);
		vec3 pm;
		
		float rep = 10.0+sin(fi*2.0+1.0)*4.0;
		
		pm.xz = mod(p.xz+vec2(rep*.5),vec2(rep))-vec2(rep*.5);
		
	
		
		float width = 1.0+sin(fi)*.8;
		float height = 2.0+cos(fi)*1.1;
		float offset = -0.5+cos(fi)*1.8;

		vec3 df = abs(vec3(pm.x,p.y+1.0/width,pm.z))-vec3(width,height,width);
		float box = max(max(df.x,df.y),df.z);
		//float box=length(vec3(pm.x,p.y,pm.z))-1.0;
	
		d = min(d,box);
	}

	return d;
}

float amb_occ(vec3 p)
{
	float acc=0.0;

	acc+=dist(p+vec3(-0.5,-0.5,-0.5));
	acc+=dist(p+vec3(-0.5,-0.5,+0.5));
	acc+=dist(p+vec3(-0.5,+0.5,-0.5));
	acc+=dist(p+vec3(-0.5,+0.5,+0.5));
	acc+=dist(p+vec3(+0.5,-0.5,-0.5));
	acc+=dist(p+vec3(+0.5,-0.5,+0.5));
	acc+=dist(p+vec3(+0.5,+0.5,-0.5));
	acc+=dist(p+vec3(+0.5,+0.5,+0.5));
	return acc*.05+.5;
}

float occ(vec3 start, vec3 dest, float size)
{
	vec3 dir = dest-start;
	float total_dist = length(dir);
	dir = dir/total_dist;
	
	float travel = .1;
	float o = 1.0;
	vec3 p=start;
	
	float search_travel=.0;
	float search_o=1.0;
	
	float e = .5*total_dist/float(occlusion_pass1_quality);
	
	//pass 1 fixed step search
	
	for (int i=0; i<occlusion_pass1_quality;i++)
	{
		travel = (float(i)+0.5)*total_dist/float(occlusion_pass1_quality);
		float cd = dist(start+travel*dir);
		float co = cd/travel*total_dist*size;
		if (co<search_o)
		{
			search_o=co;
			search_travel=travel;
			if (co<.0)
			{
				break;
			}
		}
		
	}
	
	//pass 2 tries to find a better match in close proximity to the result from the 
	//previous pass
		
	for (int i=0; i<occlusion_pass2_quality;i++)
	{
		float tr = search_travel+e;
		float oc = dist(start+tr*dir)/tr*total_dist*size;
		if (tr<.0||tr>total_dist)
		{
			break;
		}
		if (oc<search_o)
		{
			search_o = oc;
			search_travel = tr;
		}
		e=e*-.75;
	}
	
	o=max(search_o,.0);

	return o;
}

vec3 normal(vec3 p,float e) //returns the normal, uses the distance function
{
	float d=dist(p);
	return normalize(vec3(dist(p+vec3(e,0,0))-d,dist(p+vec3(0,e,0))-d,dist(p+vec3(0,0,e))-d));
}

vec3 light; //global variable that holds light direction

vec3 background(vec3 p,vec3 d)//render background
{
	return vec3(0.0);
}

vec3 object_material(vec3 p, vec3 d) //computes the material for the object
{
	vec3 n = normal(p,.01); //normal vector
	vec3 r = reflect(d,n); //reflect vector
	float ao = amb_occ(p); //fake ambient occlusion
	vec3 color = vec3(.0,.0,.0); //variable to hold the color
	
	for(int i=0;i<light_count; i++) //for each light source
	{
		vec3 light_dir = light_pos[i]-p; //light direction vector

		float d = dot(n,light_dir); //standard diffuse shading
		float o = .0;
		float s = .0;
		float ldist = length(light_dir);
		
		if (d<.0)
		{
			d = .0; //backface, always 100% occluded
		}
		else
		{
			o = occ(p,light_pos[i],1.0); //call the occlusion function
			float temp = 1.0/ldist;
			d = d*temp*o; //proper diffuse with occlusion
			s = pow(dot(r,light_dir*temp)*.5+.5,20.0)*o; //specular with occlusion
		}
		float attenuation = 1.0/pow(ldist*.1+1.0,2.0);
		
		color +=  
			(.7*ao + //ambient
			d + //diffuse
			.4*s) * //specular
			light_color[i] * attenuation;//color
			
	}
	
	return color;
	
}

#define offset1 4.5
#define offset2 1.8

vec3 getSceneColor( in vec3 p, in vec3 d )
{
	vec3 sp = p;
	vec3 color;
	float dd;
	
	//raymarcing 
	for (int i=0; i<render_steps; i++)
	{
		dd = dist(p);
		p+=d*dd;
		if (dd<.001) break;
	}
	
	float t = iGlobalTime*.5*object_speed_modifier + 10.0;
	t*=light_speed_modifier;
	
	
	//setup light position and color
	for(int i=0; i<light_count; i++)
	{
		float offs = float(i)*(3.14159*1.5/float(light_count));
		light_pos[i] = 2.0*vec3(
			sin(t*.7+offs)*2.0,
			sin(t*.5+offs)*.3,
			sin(t*.3+offs)*2.0);
		light_color[i] = .5 + .5*vec3(
			cos(t*.5+offs),
			sin(t*.3+offs),
			sin(t*.7+offs));
		light_color[i] = normalize(light_color[i]);
	}

	//of ray is close enough to geometry, call the surface shading function
	if (dd<0.1) //close enough
		color = object_material(p,d);
	else
		color = background(sp,d);
	
	//render the flares
	for(int i=0; i<light_count; i++)
	{
		float q = dot(d,normalize(light_pos[i]-sp))*.5+.5;
		float o = occ(sp,light_pos[i],1.0);
		color+=pow(q,500.0/o)*light_color[i]*3.0;
	}
	return color;
}

#ifndef RIFTRAY
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 mouse = vec3(iMouse.xy/iResolution.xy - 0.5,iMouse.z-.5);
	
	float t = iGlobalTime*.5*object_speed_modifier + 10.0;
	
	//setup the camera
	vec3 p = vec3(.0,0.0,-4.0);
	p = rotate_x(p,mouse.y*9.0+offset1);
	p = rotate_y(p,mouse.x*9.0+offset2);
	p.y*.2;
	vec3 d = vec3(uv,1.0);
	d.z -= length(d)*.7; //lens distort
	d = normalize(d);
	d = rotate_x(d,mouse.y*9.0+offset1);
	d = rotate_y(d,mouse.x*9.0+offset2);
	
	vec3 color = getSceneColor( p, d );
	
	//post procesing
	color *=.85;
	color = mix(color,color*color,0.3);
	color -= hash(color.xy+uv.xy)*.015;
	color -= length(uv)*.06;
	color =cc(color,.7,.7);
	gl_FragColor = vec4(color,1.0);
}
#endif
