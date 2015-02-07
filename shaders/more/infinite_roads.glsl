// "Infinite Fractal Roads" by Kali

// @var title Infinite Fractal Roads
// @var author Kali
// @var url https://www.shadertoy.com/view/XlXGzj

// @var tex0 tex06.jpg

// @var eyePos -4.975 1.256 -3.0
// @var headSize 0.01

#define ang radians(-37.)

const int Iterations=13;
const float width=.22;
const float detail=.00003;
const float Scale=2.30;

vec3 lightdir=normalize(vec3(0.,-0.15,-1.));
vec3 skydir=normalize(vec3(0.,-1.,0.));

mat2 rota=mat2(cos(ang),sin(ang),-sin(ang),cos(ang));
mat2 rota2=mat2(cos(-ang),sin(-ang),-sin(-ang),cos(-ang));


float ot=0.;
float det=0.;

float de(vec3 pos) {
	float DEfactor=Scale;
	vec3 p = pos;
	ot=1000.;
	for (int i=0; i<Iterations; i++) {
		p.xyz = vec3(2.)-abs(abs(p.xyz+vec3(1.,1.,0.))-vec3(2.))-vec3(1.,1.,0.);  
		float r2 = dot(p.xyz, p.xyz);
		ot = clamp(ot,0.,r2);
		p*=Scale/clamp(r2,0.,1.); 
		DEfactor*=Scale/clamp(r2,0.,1.);
		p = p + pos.yxz;
	}
	return length(p.xyz)/DEfactor;
}



vec3 normal(vec3 p) {
	vec3 e = vec3(0.0,det,0.0);
	
	return normalize(vec3(
			de(p+e.yxx)-de(p-e.yxx),
			de(p+e.xyx)-de(p-e.xyx),
			de(p+e.xxy)-de(p-e.xxy)
			)
		);	
}

float shadow(vec3 pos, vec3 sdir) {
		float totalDist =5.0*detail, sh=1.;
 		for (int steps=0; steps<20; steps++) {
			if (totalDist<.3) {
				vec3 p = pos - totalDist * sdir;
				float dist = de(p);
				if (dist < det)  sh=0.2;
				totalDist += dist;
			}
		}
		return sh;	
}

float AO( in vec3 pos, in vec3 nor ) //by iq... but I had to unroll it! (Firefox fault)
{
    float ao = 1.0;
    float aoi = 0.0;
    float totao = 0.0;
    float sca = 20.0;
	float hr;
	vec3 aopos;
	float dd;
    hr = .015 + .01*float(aoi*aoi);
    aopos =  nor * hr + pos;
    dd = de( aopos );
    totao += -(dd-hr)*sca;
    sca *= 0.4;
	aoi++;
    hr = .015 + .01*float(aoi*aoi);
    aopos =  nor * hr + pos;
    dd = de( aopos );
    totao += -(dd-hr)*sca;
    sca *= 0.4;
	aoi++;
    hr = .015 + .01*float(aoi*aoi);
    aopos =  nor * hr + pos;
    dd = de( aopos );
    totao += -(dd-hr)*sca;
    sca *= 0.4;
	aoi++;
    hr = .015 + .01*float(aoi*aoi);
    aopos =  nor * hr + pos;
    dd = de( aopos );
    totao += -(dd-hr)*sca;
    sca *= 0.4;
	aoi++;
    hr = .015 + .01*float(aoi*aoi);
    aopos =  nor * hr + pos;
    dd = de( aopos );
    totao += -(dd-hr)*sca;
    sca *= 0.4;
	aoi++;
	return 1.0 - clamp( totao, 0.0, 1.0 );
}

vec3 light(in vec3 p, in vec3 dir) {
	vec3 n=normal(p);
	float sh=shadow(p, lightdir);
	float ao=AO(p, n);
	float diff=max(0.,dot(lightdir,-n));
	float amb=(.2+.6*max(0.,dot(normalize(skydir),-n)))*ao;
	vec3 r = reflect(lightdir,n);
	float spec=max(0.,dot(dir,-r));
	return (diff*1.5+pow(spec,20.)*.4)*sh*vec3(1,.9,.65)+amb*vec3(.8,.9,1.);	
		}

vec3 raymarch(in vec3 from, in vec3 dir) 
{
	float st,d=1., totdist=st=0.;
	vec3 p, col;
	for (int i=0; i<140; i++) {
		if (d>det && totdist<.5) {
			p=from+totdist*dir;
			d=de(p)*.8;
			det=detail*(1.+totdist*50.);
			totdist+=d; 
			st+=max(0.,.032-d);
		}
	}
	float l=pow(max(0.,dot(normalize(-dir),normalize(lightdir))),4.);
	vec3 backg=vec3(.6,.8,1.)*.4*(.4+clamp(l,0.,.6))+vec3(1.,.9,.7)*l*.5;
	float y=(p.xy*rota2).y;
	if (d<det) {
		ot=clamp(pow(ot,2.)*4.,0.,1.);
		col=.5+vec3(ot*ot,ot,ot*ot*ot*1.1)*.5;
		col*=light(p-det*dir, dir); 
		col = mix(col, backg, 1.0-exp(-9.*totdist*totdist));
	} else { 
		col=backg*(1.+texture2D(iChannel0,dir.xy*rota2*.5-vec2(0.,iGlobalTime*.004)).z*.25);
	}
	return col+st*vec3(1.,.95,.8)*.15;
}

vec3 getSceneColor(in vec3 from, in vec3 dir)
{
	vec3 col=raymarch(from,dir); 
	col=col*vec3(1.1,1.03,1.)+vec3(.05,.02,.0);
    return col;
}

#ifndef RIFTRAY
void main(void)
{
	float t=radians(45.-iGlobalTime*10.);
	vec2 uv = gl_FragCoord.xy / iResolution.xy*2.-1.;
	uv.y*=iResolution.y/iResolution.x;
	vec2 uv2=uv;
	vec2 mouse=(iMouse.xy/iResolution.xy-.5);
	if (mouse.x<-.499) mouse=vec2(0.);
	uv+=mouse*1.5;
	uv.y-=.1;
	uv*=rota;
	float h=sin(iGlobalTime*.2)*.0031;
	uv+=(texture2D(iChannel1,vec2(iGlobalTime*.15)).xy-.5)*max(0.,h)*7.;
	vec3 from=vec3(-4.975+h,1.256-h,-3.+iGlobalTime*.005);
	vec3 dir=normalize(vec3(uv*1.3,1.));
    
	vec3 col=getSceneColor(from,dir);
    /*
	vec3 flare=vec3(1,.95,.93)*pow(max(0.,.8-length(uv2-mouse*1.5))/.8,1.5)*.3;
	col+=flare*dot(uv2,uv2);
	col*=length(clamp((.6-pow(abs(uv2),vec2(3.))),vec2(0.),vec2(1.)));
	col=col*vec3(1.1,1.03,1.)+vec3(.05,.02,.0);
	col+=vec3(1,.85,.7)*pow(max(0.,.3-length(uv))/.3,2.)*.5;*/
	gl_FragColor = vec4(col,1.);
}
#endif
