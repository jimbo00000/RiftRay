// http://www.fractalforums.com/movies-showcase-%28rate-my-movie%29/very-rare-deep-sea-fractal-creature/

// @var title Underwater Life
// @var author Kali
// @var url https://www.shadertoy.com/view/Mtf3Rr
// @var eyePos -2.0 -0.2 -17.0

const int Iterations=20;
const float Scale=1.25;
const vec3 Julia=vec3(-3.,-1.5,-0.5);
const vec3 RotVector=vec3(0.5,-0.05,-1.);
const float RotAngle=40.;
const float Speed=1.3;
const float Amplitude=0.6;
const float detail=.025;
const vec3 lightdir=-vec3(0.,1.,0.);

mat2 rot;

float de(vec3 p); 
vec3 color(vec3 p);

vec3 normal(vec3 p) {
	vec3 e = vec3(0.0,detail,0.0);
	
	return normalize(vec3(
			de(p+e.yxx)-de(p-e.yxx),
			de(p+e.xyx)-de(p-e.xyx),
			de(p+e.xxy)-de(p-e.xxy)
			)
		);	
}

float calcAO( const vec3 pos, const vec3 nor ) {
	float aodet=detail*3.;
	float totao = 0.0;
    float sca = 1.0;
    for( int aoi=0; aoi<5; aoi++ ) {
        float hr = aodet*float(aoi*aoi);
        vec3 aopos =  nor * hr + pos;
        float dd = de( aopos );
        totao += -(dd-hr)*sca;
        sca *= 0.7;
    }
    return clamp( 1.0 - 5.0*totao, 0., 1.0 );
}


float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<48; i++ )
    {
        float h = de(ro + rd*t);
		h = max( h, 0.0 );
        res = min( res, k*h/t );
        t += clamp( h, 0.01, 0.5 );
    }
    return clamp(res,0.0,1.0);
}




vec3 light(in vec3 p, in vec3 dir) {
	vec3 ldir=normalize(lightdir);
	vec3 n=normal(p);
	float sh=softshadow(p,-ldir,1.,20.);
	float diff=max(0.,dot(ldir,-n));
	vec3 r = reflect(ldir,n);
	float spec=max(0.,dot(dir,-r));
	vec3 colo=color(p);
    return (diff*sh+.15*max(0.,dot(normalize(dir),-n))*calcAO(p,n))*colo+pow(spec,30.)*.5*sh;	
		}


float kaliset(vec3 p) {
    p.y-=iGlobalTime;
	p.y=abs(2.-mod(p.y,4.));
    for (int i=0;i<18;i++) p=abs(p)/dot(p,p)-.8;
    return length(p);
}


vec3 raymarch(in vec3 from, in vec3 dir)
{
	vec3 col=vec3(0.);
    float st,d=1.0,totdist=st=0.;
	vec3 p;
	float k;
    for (int i=0; i<70; i++) {
	if (d>detail && totdist<50.)
	{
		k+=kaliset(p)*exp(-.002*totdist*totdist)*max(0.,totdist-3.)*(1.-step(0.,.2-d));
        p=from+totdist*dir;
		d=de(p);
		totdist+=d;
	}
	}
	vec3 backg=vec3(.4,.5,.55)*(1.+gl_FragCoord.y/iResolution.y*1.5);
	if (d<detail) {
		col=light(p-detail*dir, dir); 
	} else { 
		col=vec3(backg);
	}
	col = mix(col, vec3(backg), 1.0-exp(-.002*totdist*totdist));
    return col+k*.002;
}

vec3 getSceneColor( vec3 from, vec3 dir )
{
	vec3 col=raymarch(from,dir); 
    return col;
}

#ifndef RIFTRAY
void main(void)
{
	float t=iGlobalTime*.3;
	vec2 uv = gl_FragCoord.xy / iResolution.xy*2.-1.;
	uv.y*=iResolution.y/iResolution.x;
	vec3 from=vec3(0.,-.7,-25.+sin(t)*8.);
	vec3 dir=normalize(vec3(uv,1.));
	rot=mat2(cos(-.5),sin(-.5),-sin(-.5),cos(-.5));
	dir.yz=dir.yz*rot;
	from.yz=from.yz*rot;

	vec3 col=raymarch(from,dir); 
	col*=max(0.,.1-length(uv*.05))/.1;
    gl_FragColor = vec4(col,1.);
}
#endif

mat3  rotationMatrix3(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}


float de(vec3 p) {
	p=p.zxy;
	float a=1.5+sin(iGlobalTime*.3578)*.5;
	p.xy=p.xy*mat2(cos(a),sin(a),-sin(a),cos(a));
	p.x*=.75;
	float time=iGlobalTime*Speed;
	vec3 ani;
	ani=vec3(sin(time),sin(time),cos(time))*Amplitude;
	p+=sin(p*3.+time*6.)*.04;
	mat3 rot = rotationMatrix3(normalize(RotVector+ani), RotAngle+sin(time)*10.);
	vec3 pp=p;
	float l;
	for (int i=0; i<Iterations; i++) {
		p.xy=abs(p.xy);
		p=p*Scale+Julia;
		p*=rot;
		l=length(p);
	}
	return l*pow(Scale, -float(Iterations))-.1;
}



vec3 color(vec3 p) {
	p=p.zxy;
	float a=1.5+sin(iGlobalTime*.3578)*.5;
	p.xy=p.xy*mat2(cos(a),sin(a),-sin(a),cos(a));
	p.x*=.75;
	float time=iGlobalTime*Speed;
	vec3 ani;
	ani=vec3(sin(time),sin(time),cos(time))*Amplitude;
	p+=sin(p*3.+time*6.)*.04;
	mat3 rot = rotationMatrix3(normalize(RotVector+ani), RotAngle+sin(time)*10.);
	vec3 pp=p;
	float l;
	float ot=9999.;
    for (int i=0; i<15; i++) {
		p.xy=abs(p.xy);
		p=p*Scale+Julia;
		p*=rot;
		l=length(p.x);
        ot=min(l,ot);
	}
	return mix(vec3(1.,0.2,0.1),vec3(0.6,0.5,0.6),max(0.,2.-ot)/2.);
}
