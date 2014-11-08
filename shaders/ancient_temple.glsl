// "Ancient Temple" by Kali

// Made in "messy coder" mode, sorry! :D
// I will clean up and comment later...

// @var title Ancient Temple
// @var author Kali
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/XslGWf

// @var headSize 0.008
// @var eyePos -0.0011595160 1.5994246 -5.0102816

const int Iterations=14;
const float detail=.00002;
const float Scale=2.;

vec3 lightdir=normalize(vec3(0.,-0.3,-1.));


float ot=0.;
float det=0.;

float hitfloor=0.;

float de(vec3 pos) {
	hitfloor=0.;
	vec3 p=pos;
	p.xz=abs(.5-mod(pos.xz,1.))+.01;
	float DEfactor=1.;
	ot=1000.;
	for (int i=0; i<Iterations; i++) {
		p = abs(p)-vec3(0.,2.,0.);  
		float r2 = dot(p, p);
		ot = min(ot,abs(length(p)));
		float sc=Scale/clamp(r2,0.4,1.);
		p*=sc; 
		DEfactor*=sc;
		p = p - vec3(0.5,1.,0.5);
	}
	float fl=pos.y-3.013;
	float d=min(fl,length(p)/DEfactor-.0005);
	d=min(d,-pos.y+3.9);
	if (abs(d-fl)<.0001) hitfloor=1.;
	return d;
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
		float totalDist =2.0*det, sh=1.;
 		for (int steps=0; steps<30; steps++) {
			if (totalDist<1.) {
				vec3 p = pos - totalDist * sdir;
				float dist = de(p)*1.5;
				if (dist < detail)  sh=0.;
				totalDist += max(0.05,dist);
			}
		}
		return max(0.,sh);	
}

float calcAO( const vec3 pos, const vec3 nor ) {
	float aodet=detail*80.;
	float totao = 0.0;
    float sca = 10.0;
    for( int aoi=0; aoi<5; aoi++ ) {
        float hr = aodet + aodet*float(aoi*aoi);
        vec3 aopos =  nor * hr + pos;
        float dd = de( aopos );
        totao += -(dd-hr)*sca;
        sca *= 0.75;
    }
    return clamp( 1.0 - 5.0*totao, 0.0, 1.0 );
}



float kset(vec3 p) {
	p=abs(.5-fract(p*20.));
	float es, l=es=0.;
	for (int i=0;i<13;i++) {
		float pl=l;
		l=length(p);
		p=abs(p)/dot(p,p)-.5;
		es+=exp(-1./abs(l-pl));
	}
	return es;	
}

vec3 light(in vec3 p, in vec3 dir) {
	float hf=hitfloor;
	vec3 n=normal(p);
	float sh=min(1.,shadow(p, lightdir)+hf);
	//float sh=1.;
	float ao=calcAO(p,n);
	float diff=max(0.,dot(lightdir,-n))*sh*1.3;
	float amb=max(0.2,dot(dir,-n))*.4;
	vec3 r = reflect(lightdir,n);
	float spec=pow(max(0.,dot(dir,-r))*sh,10.)*(.5+ao*.5);
	float k=kset(p)*.18; 
	vec3 col=mix(vec3(k*1.1,k*k*1.3,k*k*k),vec3(k),.45)*2.;
	col=col*ao*(amb*vec3(.9,.85,1.)+diff*vec3(1.,.9,.9))+spec*vec3(1,.9,.5)*.7;	
	return col;
}


vec3 raymarch(in vec3 from, in vec3 dir) 
{
	float t=iGlobalTime;
	vec2 lig=vec2(sin(t*2.)*.6,cos(t)*.25-.25);
	float fog,glow,d=1., totdist=glow=fog=0.;
	vec3 p, col=vec3(0.);
	float ref=0.;
	float steps;
	for (int i=0; i<130; i++) {
		if (d>det && totdist<3.5) {
			p=from+totdist*dir;
			d=de(p);
			det=detail*(1.+totdist*55.);
			totdist+=d; 
			glow+=max(0.,.02-d)*exp(-totdist);
			steps++;
		}
	}
	//glow/=steps;
	float l=pow(max(0.,dot(normalize(-dir),normalize(lightdir))),10.);
	vec3 backg=vec3(.8,.85,1.)*.25*(2.-l)+vec3(1.,.9,.65)*l*.4;
	float hf=hitfloor;
	if (d<det) {
		col=light(p-det*dir*1.5, dir); 
		if (hf>0.5) col*=vec3(1.,.85,.8)*.6;
		col*=min(1.2,.5+totdist*totdist*1.5);
		col = mix(col, backg, 1.0-exp(-1.3*pow(totdist,1.3)));
	} else { 
		col=backg;
	}
	col+=glow*vec3(1.,.9,.8)*.34;
	col+=vec3(1,.8,.6)*pow(l,3.)*.5;
	return col; 
}

vec3 getSceneColor( in vec3 from, in vec3 dir )
{
	vec3 eyeoff = vec3(0.0, 3.21-1.78, 0.0);
	vec3 color=raymarch(from+eyeoff,dir);
	
	//col*=length(clamp((.6-pow(abs(uv2),vec2(3.))),vec2(0.),vec2(1.)));
	color*=vec3(1.,.94,.87);
	color=pow(color,vec3(1.2));
	color=mix(vec3(length(color)),color,.85)*.95;
	
	
	return color;
}

#ifndef RIFTRAY
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy*2.-1.;
	uv.y*=iResolution.y/iResolution.x;
	vec2 mouse=(iMouse.xy/iResolution.xy-.5);
	float t=iGlobalTime*.15;
	float y=(cos(iGlobalTime*.1)+1.);
	if (iMouse.z<1.) mouse=vec2(sin(t*2.),cos(t)+.3)*.15*(.5+y)*min(1.,iGlobalTime*.1);
	uv+=mouse*1.5;
	uv.y-=.1;
	//uv+=(texture2D(iChannel1,vec2(iGlobalTime*.15)).xy-.5)*max(0.,h)*7.;
	vec3 from=vec3(0.0,3.04+y*.1,-2.+iGlobalTime*.05);
	vec3 dir=normalize(vec3(uv*.85,1.));
	vec3 color=raymarch(from,dir); 
	//col*=length(clamp((.6-pow(abs(uv2),vec2(3.))),vec2(0.),vec2(1.)));
	color*=vec3(1.,.94,.87);
	color=pow(color,vec3(1.2));
	color=mix(vec3(length(color)),color,.85)*.95;
	color+=vec3(1,.85,.7)*pow(max(0.,.3-length(uv-vec2(0.,.03)))/.3,1.5)*.65;
	gl_FragColor = vec4(color,1.);
}
#endif
