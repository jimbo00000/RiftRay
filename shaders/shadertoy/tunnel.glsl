// Tunnel #1
// @var author WAHa_06x36
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/4dfGDr

// @var headSize 1.75
// @var eyePos 0.0 0.0 0.0

float time=iGlobalTime;

vec3 palette(float i)
{
	if(i<4.0)
	{
		if(i<2.0)
		{
			if(i<1.0) return vec3(0.0,0.0,0.0);
			else return vec3(1.0,3.0,31.0);
		}
		else
		{
			if(i<3.0) return vec3(1.0,3.0,53.0);
			else return vec3(28.0,2.0,78.0);
		}
	}
	else if(i<8.0)
	{
		if(i<6.0)
		{
			if(i<5.0) return vec3(80.0,2.0,110.0);
			else return vec3(143.0,3.0,133.0);
		}
		else
		{
			if(i<7.0) return vec3(181.0,3.0,103.0);
			else return vec3(229.0,3.0,46.0);
		}
	}
	else
	{
		if(i<10.0)
		{
			if(i<9.0) return vec3(252.0,73.0,31.0);
			else return vec3(253.0,173.0,81.0);
		}
		else if(i<12.0)
		{
			if(i<11.0) return vec3(254.0,244.0,139.0);
			else return vec3(239.0,254.0,203.0);
		}
		else
		{
			return vec3(242.0,255.0,236.0);
		}
	}
}


vec4 colour(float c)
{
	c*=12.0;
	vec3 col1=palette(c)/256.0;
	vec3 col2=palette(c+1.0)/256.0;
	return vec4(mix(col1,col2,c-floor(c)),1.0);
}

float periodic(float x,float period,float dutycycle)
{
	x/=period;
	x=abs(x-floor(x)-0.5)-dutycycle*0.5;
	return x*period;
}

float pcount(float x,float period)
{
	return floor(x/period);
}

float distfunc(vec3 pos)
{
	vec3 gridpos=pos-floor(pos)-0.5;
	float r=length(pos.xy);
	float a=atan(pos.y,pos.x);
	a+=time*0.3*sin(pcount(r,3.0)+1.0)*sin(pcount(pos.z,1.0)*13.73);
	return min(max(max(
	periodic(r,3.0,0.2),
	periodic(pos.z,1.0,0.7+0.3*cos(time/3.0))),
	periodic(a*r,3.141592*2.0/6.0*r,0.7+0.3*cos(time/3.0))),
	0.25);
	//return max(length(gridpos)-0.5,
	//	  abs(r-floor(r)-0.5)-0.1);
}

vec3 getSceneColor( in vec3 ray_pos, in vec3 ray_dir )
{
	float i=192.0;
	for(int j=0;j<192;j++)
	{
		float dist=distfunc(ray_pos);
		ray_pos+=dist*ray_dir;

		if(abs(dist)<0.001) { i=float(j); break; }
	}

	float c=i/192.0;
	return colour(c).xyz;
}

#ifndef RIFTRAY
void main()
{
	vec2 coords=(2.0*gl_FragCoord.xy-iResolution.xy)/max(iResolution.x,iResolution.y);

	vec3 ray_dir=normalize(vec3(coords,1.0+0.0*sqrt(coords.x*coords.x+coords.y*coords.y)));
	vec3 ray_pos=vec3(0.0,-1.0,time*1.0);

	float a=cos(time)*0.0*0.4;
	ray_dir=ray_dir*mat3(
		cos(a),0.0,sin(a),
		0.0,1.0,0.0,
		-sin(a),0.0,cos(a)
	);

	vec3 col = getSceneColor( ray_pos, ray_dir );
	gl_FragColor=vec4(col,1.);
}
#endif
