// @var title City Maker
// @var author phi16
// @var url https://www.shadertoy.com/view/lscGz2

// @var tex0 tex16.png

// @var eyePos 0.0 6.0 0.0

#define M(x,y) res = mel(res,C(x,y))

struct C{
    float d;
    int t;
};
    
C mel(C c,C d){
    if(c.t != -1 && c.d < d.d)return c;
    else return d;
}
    
C sub(C c,C d){
    return C(max(c.d,-d.d),c.t);
}

float random(vec2 p){
    return texture2D(iChannel0,mod((p+vec2(0.5,0.5))/256.,1.)).x;
}

float hash( float n ) { return fract(sin(n)*753.5453123); } // https://www.shadertoy.com/view/4sfGzS
float noise1( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
	
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}

const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );
float noise(vec3 p){
    float f = 0.0;
    vec3 q = 8.0*p;
    f  = 0.5000*noise1( q ); q = m*q*2.01;
    f += 0.2500*noise1( q ); q = m*q*2.02;
    f += 0.1250*noise1( q ); q = m*q*2.03;
    f += 0.0625*noise1( q ); q = m*q*2.01;
    return f;
}

float box(vec3 p,vec3 b){
    vec3 d = abs(p)-b;
    return min(max(d.x,max(d.y,d.z)),0.)+length(max(d,0.));
}

C dist(vec3 p){
    vec3 q = p;
    q.xz = mod(q.xz,1.)-0.5;
    C res = C(0.,-1);
    
    M(p.y,1);
    vec3 pp = p;
    pp.z = mod(pp.z+10.,20.)-10.;
    M(box(pp-vec3(iGlobalTime*5.-10000.,4.,0),vec3(10000.,0.02,0.02)),5);
    M(box(pp-vec3(iGlobalTime*5.-10000.+2.,5.,2.),vec3(10000.,0.02,0.02)),7);
    M(box(pp-vec3(iGlobalTime*5.-10000.-3.,6.,5.),vec3(10000.,0.02,0.02)),2);
    M(box(pp-vec3(iGlobalTime*5.-10000.-2.,4.5,-3.),vec3(10000.,0.02,0.02)),3);
    M(box(pp-vec3(iGlobalTime*5.-10000.+1.,7.,-4.),vec3(10000.,0.02,0.02)),6);
    float hei = random(floor(p.xz))*1.6+0.5;
    hei *= smoothstep(-10.,10.,iGlobalTime*5.-floor(p.x)+random(vec2(floor(p.z),0.5))*12.-3.+pow(abs(floor(p.z)),0.8));
    if(hei>0.)M(box(q-vec3(0,hei,0),vec3(0.3,hei,0.3)),0);
    res = sub(res, C(box(q-vec3(0,hei*2.,0),vec3(0.25,0.05*smoothstep(-10.,2.,iGlobalTime*5.-floor(p.x)),0.25)),0));
    hei = 2.1 * smoothstep(-10.,10.,iGlobalTime*5.-floor(p.x-8.)+random(vec2(floor(p.z),0.5))*12.-3.+pow(abs(floor(p.z)),0.8));
    if(hei>0.)M(box(q-vec3(1,0,0)-vec3(0,hei,0),vec3(0.3,hei,0.3)),0);
    if(hei>0.)M(box(q-vec3(0,0,1)-vec3(0,hei,0),vec3(0.3,hei,0.3)),0);
    if(hei>0.)M(box(q-vec3(-1,0,0)-vec3(0,hei,0),vec3(0.3,hei,0.3)),0);
    if(hei>0.)M(box(q-vec3(0,0,-1)-vec3(0,hei,0),vec3(0.3,hei,0.3)),0);
    return res;
}

vec3 normal(vec3 p){
    vec2 e=vec2(0.001,0);
    return normalize(vec3(
        dist(p+e.xyy).d-dist(p-e.xyy).d,
        dist(p+e.yxy).d-dist(p-e.yxy).d,
        dist(p+e.yyx).d-dist(p-e.yyx).d));
}

float ao(vec3 p,vec3 n){
    float a = 0.;
    float d = 1./6.;
    for(float i=1.;i<6.;i++){
    	a += max(0.,dist(p+n*i*d).d) / (i*d) * pow(2.,-i);
    }
    return a * 0.5 + 0.5;
}

vec3 color(vec3 p, vec3 v){
    float d = 0.001;
    int maxIter = 100;
    C c=C(0.,-1);
    float cubeNearest = -1.;
    vec3 pos;
    for(int i=0;i<80;i++){
        pos = p+d*v;
        C ci=dist(pos);
        float rd = ci.d;
        if(2 <= ci.t && ci.t <= 10){
        	if(cubeNearest<0. || cubeNearest > rd)cubeNearest = rd;
        }
        if(abs(rd) < 0.001*max(1.,d/10.)){
            maxIter=i;
            c=ci;
            break;
        }
        d += rd;
    }
    vec3 n = normal(pos);
    if(c.t == 1 || c.t==-1 && v.y < 0.)c = C(-p.y/v.y,1), d = c.d, pos = p+c.d*v, n = vec3(0,1,0);
    vec3 col = vec3(0);
    if(c.t==-1)col = v*0.5+0.5;
    else if(c.t == 0){
        vec3 light = normalize(vec3(1,1,1));
        float hei = random(floor(pos.xz))*1.6+0.5;
        hei *= smoothstep(-10.,10.,iGlobalTime*5.-floor(pos.x)+random(vec2(floor(pos.z),0.5))*12.-3.+pow(abs(floor(pos.z)),0.8));
        vec3 hdif = vec3(0,-hei*2.,0);
        
        vec3 col1 = vec3(1,0.95,0.8);
        vec3 grad = vec3(noise(pos+hdif),noise(pos+hdif+1.5),noise(pos+hdif+2.2))*2.-1.;
        grad = normalize(n + grad*2.);
        col1 *= 0.9 + 0.1 * dot(light,grad);
        vec3 bc = vec3(mod(pos.xz,1.)-0.5,pos.y).xzy-vec3(0,hei,0), bs = vec3(0.3,hei,0.3);
        vec3 ap = bs-abs(bc);
        vec3 bp = vec3(0.015);
        if((ap.x<bp.x && ap.y<bp.y) || (ap.z<bp.z && ap.y<bp.y) || (ap.x<bp.x && ap.z<bp.z))col1 *= 0.1;
        
        vec3 col2 = vec3(0);
        float rColor = random(floor(pos.xz)*2.);
        if(rColor < 0.3)col2 = mix(vec3(0.7,0.3,0.2),vec3(0.8,0.7,0.6),rColor*10./3.);
        else col2 = mix(vec3(0.3,0.3,0.3),vec3(0.8,0.8,0.8),(rColor-0.3)*10./7.);
        col2 += max(0.,dot(light,n))*0.4;
        
        vec3 col3 = col2;
        vec3 posi = abs(mod(pos+hdif+0.05,vec3(0.1,0.1,0.1))-0.05)*20.;
        vec3 ri = mod(pos+hdif,1.);
        if(pos.y < hei*2.-0.05 && (ri.x > 0.25 && ri.x < 0.75 || ri.z > 0.25 && ri.z < 0.75)){
            float ep = max(posi.x,max(posi.y,posi.z));
            if(ep < 0.65){
                col3 = vec3(0.);//textureCube(iChannel1,reflect(v,n)).xyz * 0.5 + 0.5;
                col3 *= 2.0 - exp(ep-0.65);
            }else if(ep < 0.7){
                col3 *= max(0.,dot(normalize(posi),light))*0.4;
            }
        }else if(ri.x > 0.25 && ri.x < 0.75 && ri.z > 0.25 && ri.z < 0.75){
            col3 *= 0.3;
        }
        if(posi.y > 0.7)col3 *= 1. + (posi.y-1.)*3. * max(smoothstep(0.35,0.1,abs(ri.x-0.5)),smoothstep(0.35,0.1,abs(ri.z-0.5)));
        
        float level = smoothstep(-5.,9.,iGlobalTime*5.-floor(pos.x)+random(vec2(floor(pos.z),0.5))*6.-3.);
        float level2 = smoothstep(-1.,9.,iGlobalTime*5.-floor(pos.x)+random(vec2(floor(pos.z),0.5))*6.-3.);
        col2 = mix(col2,col3,level2);
        col = mix(col2,col1,smoothstep(level-0.3,level+0.7,noise(pos+hdif)));
        col *= ao(pos,n);    }else if(c.t == 1){
        col = vec3(0.8,0.8,0.8);
        if(max(abs(mod(pos.x,1.)),abs(mod(pos.z,1.))) > 0.9)col *= 0.8;
        col *= min(1.,ao(pos,n));
    }else if(c.t >= 2 && c.t <= 10){
        float d = (iGlobalTime*5.+float(c.t-5)-0.04) - pos.x;
        if(d < 0.)return vec3(1,1,1);
        return vec3(0.8,0.8,5)/(d+1.);
    }
    if(cubeNearest > 0. && cubeNearest < 0.2)col += (1.-pow(cubeNearest*5.,0.5))*0.5;
    col += pow(max(0.,(dot(v,vec3(1,0,0))-0.8)*10./2.),9.);
    return col;
}

vec3 getSceneColor( in vec3 p, in vec3 v )
{
    p.x += iGlobalTime * 5.;
    return color(p,v);
}

#ifndef RIFTRAY
vec4 sampleImage(vec2 coord){
	vec2 R = iResolution.xy, uv = (2.*coord - R)/R.y;
    vec3 p=vec3(iGlobalTime*5.-sin(iGlobalTime/4.)*1.,4.2,0);
    vec3 rv=vec3(1,0,0);
    vec2 bi = vec2(random(vec2(iGlobalTime*256.,0.)),random(vec2(-iGlobalTime*256.,1.)));
    uv += bi/60.*800./iResolution.x;
    vec3 v=vec3(1,uv.y-0.1,uv.x);
    v = normalize(v);
    float spd = 2.;
    float rot = (iGlobalTime*spd + sin(iGlobalTime*spd/2.))/8.+0.1;
    v.xz *= mat2(cos(rot),-sin(rot),sin(rot),cos(rot));
    rv.xz *= mat2(cos(rot),-sin(rot),sin(rot),cos(rot));
    p -= rv;
	return vec4(color(p,v),1.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor = vec4(0);
    const float p = 1.;//sampling
    for(float i=0.;i<1.;i+=1./p){
        fragColor += sampleImage(fragCoord+vec2(cos(i*2.*3.1415),sin(i*2.*3.1415))/2.)/p;
    }
}
#endif
