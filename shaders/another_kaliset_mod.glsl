//Another Kaliset Mod (inspired by https://www.shadertoy.com/view/XtlGDH from bergi)

// @var title Another Kaliset Mod
// @var author eiffie
// @var url https://www.shadertoy.com/view/Xls3z2

#define time iGlobalTime
#define size iResolution

vec3 mcol,ro;
float dL=100.0,mxscl=max(0.5,abs(sin(time*0.1))*2.5),ltpos=-1.5+sin(time*10.0);
float DE(vec3 z0)
{
	vec4 z = vec4(z0,1.0);
	float d=100.0;
	for (int n = 0; n < 7; n++) {//kaliset mod
		z.xyz=abs(z.yzx+vec3(-0.25,-0.75,-1.5)+ro*0.1);
		z/=min(dot(z.xyz,z.xyz),mxscl);
		d=min(d,(length(z.xy)+abs(z.z)*0.01)/z.w);
		if(n==2)dL=min(dL,(length(z.xy)+abs(z.z+ltpos)*0.1)/z.w);
		if(n==3)mcol=vec3(0.7,0.6,0.5)+sin(z.xyz)*0.1;
	}
	return d;
}



float rndStart(vec2 co){return 0.1+0.9*fract(sin(dot(co,vec2(123.42,117.853)))*412.453);}

mat3 lookat(vec3 fw,vec3 up){
	fw=normalize(fw);vec3 rt=normalize(cross(fw,up));return mat3(rt,cross(rt,fw),fw);
}

vec3 getSceneColor(in vec3 ro, in vec3 rd)
{
	float pxl=2.0/size.y;//find the pixel size
	vec3 LDir=normalize(vec3(0.4,0.75,0.4));//direction to light
	vec3 bcol=vec3(0.0);
	//march

    float t=DE(ro)*rndStart(gl_FragCoord.xy),d,od=1.0;
	vec4 col=vec4(0.0);//color accumulator
	for(int i=0;i<99;i++){
		d=DE(ro+rd*t);
		float px=pxl*(1.0+t);
		if(d<px){
			vec3 scol=mcol;
			float d2=DE(ro+rd*t+LDir*px);
			float shad=abs(d2/d),shad2=max(0.0,1.0-d/od);
			scol=scol*shad+vec3(0.2,0.0,-0.2)*(shad-0.5)+vec3(0.1,0.15,0.2)*shad2;
			scol*=3.0*max(0.2,shad2);
			scol/=(1.0+t)*(0.2+10.0*dL*dL);
			
			float alpha=(1.0-col.w)*clamp(1.0-d/(px),0.0,1.0);
			col+=vec4(clamp(scol,0.0,1.0),1.0)*alpha;
			if(col.w>0.9)break;
		}
		col.rgb+=vec3(0.01,0.02,0.03)/(1.0+1000.0*dL*dL)*(1.0-col.w);
		od=d;
		t+=d*0.5;
		if(t>20.0)break;
	}
	col.rgb+=bcol*(1.0-clamp(col.w,0.0,1.0));
    return col.rgb;
}

#ifndef RIFTRAY
void main(){
	float pxl=2.0/size.y;//find the pixel size
	float tim=time*0.3;
	
	//position camera
	ro=vec3(cos(tim*1.1),-1.11,sin(tim*0.7))*(2.0+0.7*cos(tim*1.3))+vec3(1.0);
	vec3 rd=normalize(vec3((2.0*gl_FragCoord.xy-size.xy)/size.y,2.0));
	rd=lookat(vec3(1.0,0.0,0.0)-ro,vec3(0.0,1.0,0.0))*rd;
	//ro=eye;rd=normalize(dir);
    	
	vec3 col = getSceneColor(ro, rd);
	gl_FragColor=vec4(col.rgb,1.0);
}
#endif
