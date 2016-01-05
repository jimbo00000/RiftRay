// @var title Cave Pillars
// @var author Xor
// @var url https://www.shadertoy.com/view/Xsd3z7

// @var tex0 tex19.png
// @var tex1 tex20.jpg
// @var tex2 tex09.jpg

float MAX = 120.0;
float PRE = 0.01;
vec3 L = normalize(vec3(0.1,-0.3,-0.8));
//Tri-linear Texturing Function
vec3 textureTL(sampler2D tex, vec3 p)
{
 	return  (texture(tex,p.xy).rgb
            +texture(tex,p.zy).rgb
            +texture(tex,p.xz).rgb)/3.0;
}
//Main Distance Field Function
float model(vec3 p)
{
    vec3 n = p * vec3(1.0,1.0,0.5);
    float V = (textureTL(iChannel0,n/64.0).r-0.4)*4.0;
          V += (textureTL(iChannel0,n/24.0).r-0.5)/2.0;
    return V;
}
// Grey scale by Shane
float getGrey(vec3 p)
{ 
    return dot(p, vec3(.299, .587, .114)); 
}
// Texture bump mapping by Shane also. Four tri-planar lookups, or 12 texture lookups in total.
vec3 doBumpMap( sampler2D tex, vec3 p, vec3 n, float bumpfactor)
{
 	const float eps = 0.001; // Change to suit needs.
	vec3 grad = vec3( getGrey(textureTL(tex, vec3(p.x-eps, p.y, p.z))),
 	getGrey(textureTL(tex, vec3(p.x, p.y-eps, p.z))),
 	getGrey(textureTL(tex, vec3(p.x, p.y, p.z-eps))))/eps;

 	grad -= getGrey(textureTL(tex, p))/eps; 

 	grad -= n*dot(n, grad); 

 	return normalize( n + grad*bumpfactor );

}

//Normal Function
vec3 normal(vec3 p)
{
 	vec3 N = vec3(-8.0, 8.0, 0.0) * PRE; // Change to suit needs.

 	N = normalize(model(p+N.xyy)*N.xyy+model(p+N.yxy)*N.yxy+model(p+N.yyx)*N.yyx+model(p+N.xxx)*N.xxx);

 	return doBumpMap( iChannel2, p/16., N, 0.008); // Change to suit needs.
}
//Color/Material Function
vec3 color(vec3 p, vec3 d)
{
    vec3 C = (textureTL(iChannel1,p/32.0)
        	  +textureTL(iChannel1,p/4.0))/2.0;
    vec3 N = normal(p);
 	float TL = (dot(N,L)*0.5+0.5);
    TL *= clamp(1.0-model(p-L),0.1,1.0);
    return C*TL+pow(abs((dot(reflect(N,L),-d)*0.5+0.5)),16.0);
}
//Simple Raymarcher
vec4 raymarch(vec3 p, vec3 d)
{
    float S = 0.0;
    float T = S;
    vec3 D = normalize(d);
    vec3 P = p+D*S;
    for(int i = 0;i<240;i++)
    {
        S = model(P);
        T += S;
        P += D*S;
        if ((T>MAX) || (S<PRE)) {break;}
    }
    return vec4(P,min(T/MAX,1.0));
}

vec3 getSceneColor( in vec3 P, in vec3 D )
{
    P.y *= -1.;
    P = P.zxy;
    D.y *= -1.;
    D = D.zxy;
    vec4 M = raymarch(P,D);
    float fog = clamp(max(M.w,M.z/8.0),0.0,1.0);
	vec3 c = mix(color(M.xyz,D)*max(1.0-M.w*2.0,0.0),vec3(0.42,0.46,0.5),fog);
    return c;
}

#ifndef RIFTRAY
//Camera Variables
vec3 P = vec3(iGlobalTime*2.0,0.0,0.0);
vec2 A = (0.5-iMouse.xy/iResolution.xy)*vec2(6.2831,3.1416)*0.5;
vec3 D = mix(vec3(1.0,0.0,0.0),vec3(cos(A.x)*cos(A.y),sin(A.x)*cos(A.y),sin(A.y)),
             float(iMouse.x>1.0 || iMouse.y>1.0));

vec3 X = normalize(D);
vec3 Y = normalize(cross(X,vec3(0.0,0.0,1.0)));
vec3 Z = cross(X,Y);

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 UV = (fragCoord.xy-iResolution.xy*0.5)/iResolution.y;
    
    D = normalize(mat3(X,Y,Z) * vec3(1.0,UV));
    
    vec4 M = raymarch(P,D);
    float fog = clamp(max(M.w,M.z/8.0),0.0,1.0);
    vec3 col = getSceneColor(P,D);
	//fragColor = vec4(mix(color(M.xyz,D)*max(1.0-M.w*2.0,0.0),vec3(0.42,0.46,0.5),fog),1.0);
	fragColor = vec4(col,1.0);
}
#endif
