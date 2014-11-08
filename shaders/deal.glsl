// Distance Estimated Area Lights by eiffie, via ben, via Brian Karis
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// 
// This script is based on _ben's example found here: https://www.shadertoy.com/view/ldfGWs
// It has been extended to use distance estimated lights. I tried to keep
// as much of the original code as possible. Thanks bj for the example and link!
// Distance Estimated Light is versatile but estimating light (the most important part of a scene)
// is tricky at best, a total hack at worst - use with caution.

// Here is the original header by ben:
//	
//	area lights based on Brian Karis's Siggraph 2013 presentation
//	http://blog.selfshadow.com/publications/s2013-shading-course/
//	
//	kind of pointless for a ray marched scene, but they work with rasterized stuff too.
//	I still need to do attenuation and some noodling. comments too, though everything's
//	in the course notes at the site above for anyone interested.
//	
//	raymarching code and anything else that looks well written courtesy of iq.
//	
//	~bj.2013

// Everything that looks scrunched and hard to read is what I added - eiffie

// @var title D.E.A.L.
// @var author eiffie
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/4ss3Ws

// @var headSize 0.4
// @var eyePos -0.028 0.291 -0.858
// @var tex0 tex05.jpg
// @var tex1 tex16.png

//#define DISABLE_ALBEDO
//#define DISABLE_NORMALS
//#define DISABLE_ROUGHNESS
//#define DISABLE_BLOCK

const float SpecFalloff=33.5, BloomFalloff=50000.0, AttenFalloff=10.0;//season to taste (higher # falls quicker)

vec3 tubeStart;
vec3 tubeEnd;

float specTrowbridgeReitz( float HoN, float a, float aP )
{
	float a2 = a * a;
	float aP2 = aP * aP;
	return ( a2 * aP2 ) / pow( HoN * HoN * ( a2 - 1.0 ) + 1.0, 2.0 );
}

float visSchlickSmithMod( float NoL, float NoV, float r )
{
	float k = pow( r * 0.5 + 0.5, 2.0 ) * 0.5;
	float l = NoL * ( 1.0 - k ) + k;
	float v = NoV * ( 1.0 - k ) + k;
	return 1.0 / ( 4.0 * l * v );
}

float fresSchlickSmith( float HoV, float f0 )
{
	return f0 + ( 1.0 - f0 ) * pow( 1.0 - HoV, 5.0 );
}

//here is the new stuff

float tim=mod(iGlobalTime+15.0,60.0)*0.1;//timing to flip between shapes
//some handy DE shapes
float Tube(vec3 pa, vec3 ba, float r){return length( pa - ba * clamp( dot(pa,ba) / dot(ba,ba), 0.0, 1.0 ) ) - r;}
float RRect(in vec3 z, vec4 r){return length(max(abs(z.xyz)-r.xyz,0.0))-r.w;}
float RSTube(in vec3 z, vec4 r){z.xz=abs(z.xz)-r.xy;return length(vec2(length(max(z.xz,0.0))-r.z,z.y))-r.w;}
float ROCyl(in vec3 z, vec3 r){return length(vec2(max(abs(z.y)-r.y,0.0),length(z.xz)-r.x))-r.z;}

float DEL(vec3 pos){// distance estimated light
	pos-=tubeStart;//just switching shapes (could be anything but don't use sharp edges!)
	if(tim<1.0)return length(pos)-0.075;//sphere
	else if(tim<2.0)return RRect(pos,vec4(0.2,0.0,0.1,0.05));//rounded rectangular box
	else if(tim<3.0)return length(vec2(length(pos.xy)-0.075,pos.z))-0.005;//torus
	else if(tim<4.0)return RSTube(pos,vec4(0.2,0.0,0.1,0.025));//rounded square tube
	else if(tim<5.0)return Tube(pos,tubeEnd-tubeStart,0.025);//segment
	else return ROCyl(pos,vec3(0.2,0.05,0.01));//rounded open cylinder
}

vec4 cp; //closest point to light along ray (well kinda, very inaccurate if ray does not come close)
vec2 map( in vec3 pos )
{//same as iq's but adding a light trap
	float bump = 0.0;
	#ifndef DISABLE_NORMALS
		if( pos.y < 0.005) bump = texture2D( iChannel0, pos.xz * 6.0 ).x * 0.002;
	#endif
	vec2 res	= vec2( pos.y + bump, 1.0 );
	#ifndef DISABLE_BLOCK
	res.x=min(res.x,RRect(pos,vec4(0.05,0.05,0.05,0.001)));
	#endif
	float dL=DEL(pos);
	if(dL<cp.w)cp=vec4(pos,dL);//catch the position nearest the light as we march by (accurate only when close)
	if(dL<res.x)res=vec2(dL,-2.0);
	return res;
}

vec2 castRay( in vec3 ro, in vec3 rd, in float maxd )
{//same as iq's but adding a light trap
	cp=vec4(1000.0);//reset the light trap
	const float precis = 0.001;
	vec2 res=vec2(100.0);
	float t = precis*10.0;
	for( int i=0; i<32; i++ )
	{
		if( res.x<precis||t>maxd ) continue;//break;
		res = map( ro+rd*t );
		t += res.x;
	}
	if( t>maxd ) res.y=-1.0;
	return vec2( t, res.y );
}

vec3 findLightDir(vec3 p, float d){ //find the light direction given a point and estimated distance
	vec2 v=vec2(d,0.0);
	return -normalize(vec3(-DEL(p-v.xyy)+DEL(p+v.xyy),-DEL(p-v.yxy)+DEL(p+v.yxy),-DEL(p-v.yyx)+DEL(p+v.yyx)));
}

float findSpecLight( vec3 pos, vec3 N, vec3 V, float f0, float roughness )
{//calculates the specular portion of light by finding the point on the light closest to the reflected ray
 //and using that as the light direction
	castRay(pos, reflect( -V, N ), 3.0);//find the closest point to the light along the reflected ray
	float distLight=max(DEL(pos),length(cp.xyz-pos));//find the distance to that point
	vec3 NL=findLightDir(cp.xyz,cp.w); //the direction to light from closest point on reflected ray
	vec3 closestPoint=cp.xyz+NL*cp.w; //an estimate of the light surface point nearest the reflected ray
	vec3 l=normalize(closestPoint-pos); //the direction to this point

	//pretty much the same as original from here
	float NoV		= clamp( dot( N, V ), 0.0, 1.0 );	
	vec3 h			= normalize( V + l );
	float HoN		= clamp( dot( h, N ), 0.0, 1.0 );
	float HoV		= dot( h, V );
	float NoL		= clamp( dot( N, l ), 0.0, 1.0 );
	
	float alpha		= roughness * roughness;
	//float alphaPrime	= clamp( tubeRad / ( distLight * 2.0 ) + alpha, 0.0, 1.0 );
	float alphaPrime	= clamp( 1.0/(1.0+distLight*SpecFalloff) + alpha, 0.0, 1.0 );//not sure at all about this??
	
	float specD		= specTrowbridgeReitz( HoN, alpha, alphaPrime );
	float specF		= fresSchlickSmith( HoV, f0 );
	float specV		= visSchlickSmithMod( NoL, NoV, roughness );
	return  specD * specF * specV * NoL;
}

vec3 areaLights( vec3 pos, vec3 nor, vec3 rd )
{
	float noise	=  texture2D( iChannel1, pos.xz ).x * 0.5;
	noise			+= texture2D( iChannel1, pos.xz * 0.5 ).y;
	noise			+= texture2D( iChannel1, pos.xz * 0.25 ).z * 2.0;
	noise			+= texture2D( iChannel1, pos.xz * 0.125 ).w * 4.0;
	
	vec3 albedo		= pow( texture2D( iChannel0, pos.xz ).xyz, vec3( 2.2 ) );
	albedo			= mix( albedo, albedo * 1.3, noise * 0.35 - 1.0 );
	float roughness = 0.7 - clamp( 0.5 - dot( albedo, albedo ), 0.05, 0.95 );
	float f0		= 0.3;
	
	#ifdef DISABLE_ALBEDO
	albedo			= vec3(0.1);
	#endif
	
	#ifdef DISABLE_ROUGHNESS
	roughness		= 0.05;
	#endif
	
	float dist	= DEL(pos);
	vec3 L		= findLightDir(pos,dist); //the direction to light
	float NdotL	= max(0.0,dot(nor,L));
	vec2 vShad	= castRay(pos, L, 0.5);
	float shad	= ((vShad.y>-0.5)?1.0-NdotL:1.0);
	float spec	= findSpecLight( pos, nor, -rd, f0, roughness );
	
	float atten	= 1.0/(1.0+dist*dist*AttenFalloff);
	vec3 color	= albedo * 0.63 * NdotL * atten * shad + spec;
	return pow( color, vec3( 1.0 / 2.2 ) );
}

vec3 rotYaw( vec3 v, float a )
{
	float c = cos(a);
	float s = sin(a);
	return v * mat3( c, 0,-s,	0, 1, 0,	s, 0, c );
}

vec3 rotPitch( vec3 v, float a )
{
	float c = cos(a);
	float s = sin(a);
	return v * mat3( 1, 0, 0,	0, c,-s,	0, s, c );
}

void updateLights()
{
	vec3 pos		= vec3( sin( iGlobalTime * 0.25 )*0.5, abs( cos( iGlobalTime ) * 0.25 ) + 0.1 , 0.0 );
	
	vec3 tubeVec	=	rotPitch( 
						rotYaw( 
						normalize( vec3(0,0,1) ) * 0.2, 
							iGlobalTime * -1.5 ), 
							cos( iGlobalTime * 0.5 ) * 0.3 );
	
	tubeStart		= pos + tubeVec;
	tubeEnd			= pos - tubeVec + vec3(0.0,0.05,0.0);
}

//--------------------------------------------------------------------------
//	everything below here is based on iq's Raymarching - Primitives shader	
//	https://www.shadertoy.com/view/Xds3zN
//	
//--------------------------------------------------------------------------

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// A list of usefull distance function to simple primitives, and an example on how to 
// do some interesting boolean operations, repetition and displacement.
//
// More info here: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

vec3 calcNormal( in vec3 pos )
{
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    map(pos+eps.xyy).x - map(pos-eps.xyy).x,
	    map(pos+eps.yxy).x - map(pos-eps.yxy).x,
	    map(pos+eps.yyx).x - map(pos-eps.yyx).x );
	return normalize(nor);
}

vec3 render( in vec3 ro, in vec3 rd )
{ 
   
	vec2 res = castRay(ro,rd,3.0);
	vec3 col = vec3(1.1)/(1.0+cp.w*cp.w*BloomFalloff);
	if( res.y>-0.5 )
	{
		vec3 pos = ro + res.x * rd;
		vec3 nor = calcNormal( pos );
		col = max( col, areaLights( pos, nor, rd ) );
	}//else if(res.y<-1.5)col=vec3(1.0);
	return clamp(col,0.0,1.0);
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
	updateLights();
    vec3 col = render( ro, rd );

	return col;
}

#ifndef RIFTRAY
void main( void )
{
	vec2 q = gl_FragCoord.xy/iResolution.xy;
    vec2 p = -1.0+2.0*q;
	p.x *= iResolution.x/iResolution.y;

	float time = 15.0 + iGlobalTime*0.1;
	vec3 ro = vec3(1.2*cos(time),0.5+0.4*cos(time*4.0),1.2*sin(time));
	vec3 ta = vec3(0,0,0);

	vec3 cw = normalize( ta-ro );
	vec3 cp = vec3( 0.0, 1.0, 0.0 );
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
	vec3 rd = normalize( p.x*cu + p.y*cv + 2.5*cw );
	
    vec3 col = getSceneColor( ro, rd );

	gl_FragColor=vec4( col, 1.0 );
}
#endif
