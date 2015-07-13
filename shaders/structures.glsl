// @var title Structures
// @var author rez
// @var url https://www.shadertoy.com/view/Ms2SDc

// @var eyePos 0.35 1.11 0.56

const float PI=3.14159265358979323846;
const float value=0.0;

float speed=iGlobalTime*0.5;
float ground_x=1.5*cos(PI*speed*0.125);
float ground_y=4.0-3.0*sin(PI*speed*0.125)+0.125*value;
float ground_z=-1.0-speed;

float rand(in vec2 p,in float t)
	{
	return fract(sin(dot(p+mod(t,1.0),vec2(12.9898,78.2333)))*43758.5453);
	}

vec2 rotate(vec2 k,float t)
	{
	return vec2(cos(t)*k.x-sin(t)*k.y,sin(t)*k.x+cos(t)*k.y);
	}

float scene1(vec3 p)
	{
	float ground=dot(p,vec3(0.0,1.0,0.0))+0.75;
	float t1=length(abs(mod(p.xyz,2.0)-1.0))-1.35+0.05*cos(PI*p.x*4.0)+0.05*sin(PI*p.z*4.0);	// structure
	float t3=length(max(abs(mod(p.xyz,2.0)-1.0).xz-1.0,0.5))-0.075+0.1*cos(p.y*36.0);			// structure slices
	float t5=length(abs(mod(p.xyz,0.5))-0.25)-0.975;
	float bubble_w=0.8+0.2*cos(PI*p.z)+0.2*cos(PI*p.x);
	float bubble=length(mod(p.xyz,0.125)-0.0625)-bubble_w;
	float hole_w=0.05;
	float hole=length(abs(mod(p.xz,1.0)-0.5))-hole_w;
	float tube_p=2.0-0.25*sin(PI*p.z*0.5);
	float tube_v=PI*8.0;
    float tube_b=tube_p*0.02;
	float tube_w=tube_b+tube_b*cos(p.x*tube_v)*sin(p.y*tube_v)*cos(p.z*tube_v)+tube_b*sin(PI*p.z+speed*4.0);
	float tube=length(abs(mod(p.xy,tube_p)-tube_p*0.5))-tube_w;
	return min(max(min(-t1,max(-hole-t5*0.375,ground+bubble)),t3+t5),tube);
	}

vec3 getSceneColor(in vec3 ray, in vec3 dir)
	{
	float t=0.0;
	const int ray_n=96;
	for(int i=0;i<ray_n;i++)
		{
		float k=scene1(ray+dir*t);
        if(abs(k)<0.005) break;
		t+=k*0.5;
		}
	vec3 hit=ray+dir*t;
	vec2 h=vec2(-0.02,0.01); // light
	vec3 n=normalize(vec3(scene1(hit+h.xyy),scene1(hit+h.yxx),scene1(hit+h.yyx)));
	float c=(n.x+n.y+n.z)*0.1;
	vec3 color=vec3(c,c,c)-t*0.0625;
    //color*=0.6+0.4*rand(vec2(t,t),iGlobalTime); // noise!
    return vec3(c+t*0.08,c+t*0.02,c*1.5-t*0.01)+color*color;
	}

#ifndef RIFTRAY
void main(void)
	{
	vec2 position=gl_FragCoord.xy/iResolution.xy;        
	vec2 p=-1.0+2.0*position;
	vec3 dir=normalize(vec3(p*vec2(1.625,1.0),0.75));	// screen ratio (x,y) fov (z)
	dir.yz=rotate(dir.yz,PI*0.25*sin(PI*speed*0.125)-value*0.25);	// rotation x
	dir.zx=rotate(dir.zx,PI*cos(-PI*speed*0.05));		// rotation y
	dir.xy=rotate(dir.xy,PI*0.125*cos(PI*speed*0.125));	// rotation z
	vec3 ray=vec3(ground_x,ground_y,ground_z);
    vec3 color = getSceneColor(ray,dir);
	gl_FragColor=vec4(color,1.0);
	}
#endif
