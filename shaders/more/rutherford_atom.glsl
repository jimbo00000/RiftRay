// @var title rutherford atom
// @var author mattz
// @var url https://www.shadertoy.com/view/lt2Gzh

// @var eyePos 0 0 -3

const float farval = 1e5;
const vec3 tgt = vec3(0);
vec3 cpos;
const int rayiter = 60;
const float dmax = 20.0;
vec3 L = normalize(vec3(-0.3, 1.0, -1.0));
mat3 Rview;

const float outline = 0.0225;

mat3 rotX(in float t) {
    float cx = cos(t), sx = sin(t);
    return mat3(1., 0, 0, 
                0, cx, sx,
                0, -sx, cx);
}

mat3 rotY(in float t) {
    float cy = cos(t), sy = sin(t);
	return mat3(cy, 0, -sy,
                0, 1., 0,
                sy, 0, cy);

}

mat3 rotZ(in float t) {
    float cz = cos(t), sz = sin(t);
	return mat3(cz, -sz, 0.,
                sz, cz, 0.,
                0., 0., 1.);

}

float sdRibbon( vec3 p, vec2 h ) {

	vec2 q = vec2(length(p.xy)-h.x, p.z);
    vec2 d = abs(q) - vec2(0.001, h.y);
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
    
}



vec2 opU(vec2 a, vec2 b) {
	return a.x < b.x ? a : b;
}


vec2 map(in vec3 pos) {	
    
    vec2 rval = vec2(farval, 0.0);
        
    const float k = 0.15;
    const float r = 0.25;
    const float m = 0.04;
	  
#if 0
   
    // Lithium?
    pos = pos.xzy;
    pos = rotX(0.2)*pos;
    vec3 p0 = rotY(-0.1*iGlobalTime)*rotZ(-0.2*iGlobalTime)*pos;
    rval = opU(rval, vec2(length(p0-vec3(k))-r, 0.0));
    rval = opU(rval, vec2(length(p0-vec3(k,-k,-k))-r, 0.01));
    rval = opU(rval, vec2(length(p0-vec3(-k,k,-k))-r, 0.5));
    rval = opU(rval, vec2(length(p0-vec3(-k,-k,k))-r, 0.51));
            
    vec3 p1 = rotX(0.1)*rotZ(-2.9*iGlobalTime)*pos;
    rval = opU(rval, vec2(length(p1-vec3(1.8,0,0))-r, 0.14));
    rval = opU(rval, vec2(sdRibbon(p1, vec2(1.8, m)), 0.7));

    vec3 p2 = rotX(-0.07)*rotZ(1.7*iGlobalTime)*pos;
    rval = opU(rval, vec2(length(p2-vec3(1.2,0,0))-r, 0.14));
    rval = opU(rval, vec2(sdRibbon(p2, vec2(1.2, m)), 0.7));   

#else
        
    // Beryllium?
    pos = pos.xzy;
    vec3 p0 = rotY(-0.3*iGlobalTime)*rotZ(-0.2*iGlobalTime)*pos;

    float c = sign(p0.x)*sign(p0.y)*sign(p0.z)*0.25 + 0.25;
    rval = opU(rval, vec2(length(abs(p0)-vec3(k))-r, c));
    
    float rx = 0.2;
    float rz = 1.05;
    float l = 1.0;
    float s = 1.0;
    
    for (int i=0; i<4; ++i) {
        vec3 pi = rotX(0.04)*rotZ(s*rz*iGlobalTime)*rotY(-s*rx)*rotX(s*rx*0.5+0.35)*pos;
        rval = opU(rval, vec2(length(pi-vec3(l,0,0))-r, 0.14));
        rval = opU(rval, vec2(sdRibbon(pi, vec2(l, m)), 0.7));
        l += 0.4;
        rx += 0.05;
        rz += 0.2;
        s *= -1.0;
    }
    
#endif    
   
	return rval;
    
}

vec3 hue(float h) {
	
	vec3 c = mod(h*6.0 + vec3(2, 0, 4), 6.0);
	return h >= 1.0 ? vec3(h-1.0) : clamp(min(c, -c+4.0), 0.0, 1.0);
}



vec3 calcNormal( in vec3 pos )
{
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    map(pos+eps.xyy).x - map(pos-eps.xyy).x,
	    map(pos+eps.yxy).x - map(pos-eps.yxy).x,
	    map(pos+eps.yyx).x - map(pos-eps.yyx).x );
	return normalize(nor);
}


vec4 castRay( in vec3 ro, in vec3 rd, in float maxd )
{
	float precis = 0.002;
    
    float h=outline;
    
    float t = 0.0;
    float m = -1.0;

    float tclose = farval;
    float hclose = 2.0*outline;
    
    for( int i=0; i<rayiter; i++ )
    {
        if( abs(h)<precis||t>maxd ) continue;//break;
        t += h;
	    vec2 res = map( ro+rd*t );
        
        if (h < outline && res.x > h && t < tclose) {
            hclose = h;
            tclose = t;
            //t = maxd+1.;
        } 
        
        h = res.x;
	    m = res.y;        
    }    
    
    if (t > maxd) {
        m = -1.0;
    }

    return vec4(t, m, tclose, hclose);
    
}

vec3 shade( in vec3 ro, in vec3 rd ){
    
    vec4 tm = castRay(ro, rd, dmax);        
    
    vec3 c;
    
    if (tm.y == -1.0) {
        
        c = vec3(step(farval, tm.z));
        //c = vec3(smoothstep(0.5*outline, outline, tm.w)); // Bad anti-aliasing?
        
    } else {        
        
        vec3 pos = ro + tm.x*rd;
        vec3 n = calcNormal(pos);        
        vec2 res = map( pos + 0.5*outline*n );        
        
        if (res.y != tm.y) {
            
            c = vec3(0);
            
        } else {

            vec3 color = hue(tm.y) * 0.4 + 0.6;
            vec3 diffamb = (0.5*clamp(dot(n,L), 0.0, 1.0)+0.5) * color;
            vec3 R = 2.0*n*dot(n,L)-L;
            float spec = 0.5*pow(clamp(-dot(R, rd), 0.0, 1.0), 20.0);
            c = diffamb + spec;
            float u = clamp((tm.x-tm.z)*2.0, 0.0, 1.0);
            c *= step(outline*u, tm.w);

        }

    }

    
    return c;
    
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    return shade(ro, rd);
}

#ifndef RIFTRAY
void mainImage( out vec4 fragColor, in vec2 fragCoord ) {

    cpos = vec3(0,0,3.5);
	vec2 uv = (fragCoord.xy - .5*iResolution.xy) * 1.25 / (iResolution.y);
	
	vec3 rz = normalize(tgt - cpos),
        rx = normalize(cross(rz,vec3(0,1.,0))),
        ry = cross(rx,rz);
	
	float thetax = 0.0, thetay = 0.0;
	
	if (max(iMouse.x, iMouse.y) > 20.0) { 
		thetax = (iMouse.y - .5*iResolution.y) * 3.14/iResolution.y; 
		thetay = (iMouse.x - .5*iResolution.x) * -6.28/iResolution.x; 
	}
	
    Rview = mat3(rx,ry,rz)*rotX(thetax)*rotY(thetay);
    L = Rview*L;

	vec3 rd = Rview*normalize(vec3(uv, 1.)),
        ro = tgt + Rview*vec3(0,0,-length(cpos-tgt));
	
	fragColor.xyz = getSceneColor(ro, rd);	
}
#endif
