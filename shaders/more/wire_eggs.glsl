//Wire Eggs by eiffie (ft. polyfold by knighty)
//creative commons blahblahblah

// @var title Wire Eggs
// @var author eiffie
// @var url https://www.shadertoy.com/view/4lS3zw

// @var eyePos 0 1.6 -3

float rnd;
void randomize(in vec2 p){rnd=fract(sin(dot(p,vec2(13.3145,17.7391)))*317.7654321);}

#define Type 5
#define PI 3.14159
vec3 nc;

void Init(){//polyfold from knighty https://www.shadertoy.com/view/XlX3zB
	 float cospin=cos(PI/float(Type)), scospin=sqrt(0.75-cospin*cospin);
	 nc=vec3(-0.5,-cospin,scospin);
}
vec3 fold(vec3 pos) {
	 for(int i=0;i<Type;i++){
		pos=abs(pos);
		pos-=2.*min(0.,dot(pos,nc))*nc;
	 }
	 return pos;
}

//vec2 rotate(vec2 v, float angle) {return cos(angle)*v+sin(angle)*vec2(v.y,-v.x);}
vec3 fractal(vec3 p, vec2 c){
	vec2 s=sin(c);c=cos(c);
	mat2 rm1=mat2(c.x,s.x,-s.x,c.x),rm2=mat2(c.y,s.y,-s.y,c.y);
	for(int i=0;i<5;i++){
		p.xy=p.xy*rm1;
		p.xz=p.xz*rm2;
		p=abs(p)*2.0+vec3(-0.04,-0.12,-1.5);
	}
	return p*0.03125;
}

float DE(in vec3 p){
	float dF=p.y+1.5;
	vec2 c=floor(p.xz*0.333333);
	c=fract(sin(c)*31.567)*vec2(1.57,-0.25);
	p.xz=mod(p.xz,3.0)-1.5;
	p=fold(p);
	p=fractal(p,c);
	return min(dF,length(max(abs(p)-vec3(1.0,0.001,0.001),0.0)));
}

vec4 mcol;
float CE(in vec3 p){
	float d=DE(p);
	if(p.y>-1.45){
		vec2 c=floor(p.xz*0.333333)+p.xy*0.5;
		mcol+=vec4(vec3(cos(c.y)*sin(c.x),sin(c.y),cos(c.y)*cos(c.x))*0.33+0.66,0.2);
	}else mcol+=vec4(0.3,0.4,1.0,0.3);
	return d;
}

float ShadAO(in vec3 ro, in vec3 rd){
	float t=0.0,s=1.0,d,mn=0.01+0.04*rnd;
	for(int i=0;i<8;i++){
		d=max(DE(ro+rd*t)*1.5,mn);
		s=min(s,d/t+t*0.5);
		t+=d;
	}
	return s;
}
vec3 LightDir(in vec3 ro){
	float tim=iGlobalTime*0.1;
	vec3 LD=vec3(cos(tim),1.0,sin(tim))*0.707;
	return LD;
	//return normalize(LP-ro);
}
vec3 Backdrop(in vec3 rd){
	vec3 L=LightDir(rd);
	vec3 col=vec3(0.3,0.4,0.5)+rd*0.1+vec3(1.0,0.8,0.6)*(max(0.0,dot(rd,L))*0.2+pow(max(0.0,dot(rd,L)),40.0));
	col*=sqrt(0.5*(rd.y+1.0));
	return col;
}

vec3 scene(in vec3 ro, in vec3 rd){
	float d=DE(ro)*rnd*0.15,t=d,od=1.0,pxl=1.6/iResolution.y;
	vec4 dm=vec4(1000.0),tm=vec4(-1.0);
	for(int i=0;i<78;i++){
		d=DE(ro+rd*t);
		if(d<pxl*t && d<od && tm.w<0.0){dm=vec4(d,dm.xyz);tm=vec4(t,tm.xyz);}
		t+=min(d,0.1+t*t*0.04);
		od=d;
		if(t>20.0 || d<0.00001)break;
	}
	if(d<pxl*t && d<dm.x){dm.x=d;tm.x=t;}
	vec3 col=Backdrop(rd),fcol=col;
	for(int i=0;i<4;i++){
		if(tm.x<0.0)break;
		float px=pxl*tm.x;
		vec3 so=ro+rd*tm.x;
		mcol=vec4(0.0);
		vec3 ve=vec3(px,0.0,0.0);
		float d1=CE(so);
		vec3 dn=vec3(CE(so-ve.xyy),CE(so-ve.yxy),CE(so-ve.yyx));
		vec3 dp=vec3(CE(so+ve.xyy),CE(so+ve.yxy),CE(so+ve.yyx));
		vec3 N=(dp-dn)/(length(dp-vec3(d1))+length(vec3(d1)-dn));
		vec3 L=LightDir(so);
		vec3 scol=mcol.rgb*0.14;
		vec3 R=reflect(rd,N);
		float v=dot(-rd,N),l=dot(N,L);
		float shad=ShadAO(so+N*0.001,L);
		vec3 cc=vec3(0.6,0.8,1.0),lc=vec3(1.0,0.8,0.6);
		float cd=exp(-distance(ro,so));
		float spcl=pow(clamp(dot(R,L),0.0,1.0),10.0),spcc=pow(max(0.0,dot(R,-rd)),1.0+cd)*0.25;
		scol=scol*(cd*v*cc+shad*l*lc)+(cd*spcc*cc+shad*spcl*lc)*mcol.a;
		scol=clamp(scol,0.0,1.0);
		float fog=min(pow(tm.x,0.4)*0.3,1.0);
		scol=mix(scol,fcol,fog);
		col=mix(scol,col,clamp(dm.x/px,0.0,1.0));
		dm=dm.yzwx;tm=tm.yzwx;
	}
	if(col!=col)col=vec3(1.0,0.0,0.0);
	return clamp(col*2.0,0.0,1.0);
}
mat3 lookat(vec3 fw){
	fw=normalize(fw);vec3 rt=normalize(cross(fw,vec3(0.0,1.0,0.0)));return mat3(rt,cross(rt,fw),fw);
}

vec3 getSceneColor( in vec3 ro, in vec3 rd)
{
	Init();
    return scene(ro, rd);
}

void mainImage(inout vec4 fragColor, in vec2 fragCoord){
	Init();
	randomize(fragCoord);
	vec3 rd=normalize(vec3((2.0*fragCoord-iResolution.xy)/iResolution.y,1.0));
	float tim=iGlobalTime;
	vec3 ro=vec3(tim-1.5,1.2,tim);
	rd=lookat(vec3(1.0+sin(tim+sin(tim)),-0.5+sin(tim*0.3)*0.3,1.0))*rd;
	//ro=eye;rd=normalize(dir);
	fragColor=vec4(getSceneColor(ro,rd),1.0);
}
