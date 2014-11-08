//"Glass Field" by Kali

// @var title Glass Field
// @var author Kali
// @var url https://www.shadertoy.com/view/4ssGWr

#define lightcol1 vec3(1.,.95,.85)
#define lightcol2 vec3(.85,.95,1.)




//Rotation matrix by Syntopia
mat3 rotmat(vec3 v, float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}

//Smooth min by IQ
float smin( float a, float b )
{
    float k = 0.5;
	float h = clamp( 0.5 + 0.5*(b-a)/k, 0.0, 1.0 );
	return mix( b, a, h ) - k*h*(1.0-h);
}


//Distance Field
float de(vec3 pos) {
	vec3 A=vec3(5.);
	vec3 p = abs(A-mod(pos,2.0*A)); //tiling fold by Syntopia
	float sph=length(p)-2.5;
	float cyl=length(p.xy)-.4;
	cyl=min(cyl,length(p.xz))-.4;
	cyl=min(cyl,length(p.yz))-.4;
    return smin(cyl,sph);
}

// finite difference normal
vec3 normal(vec3 pos) {
	vec3 e = vec3(0.0,0.001,0.0);
	
	return normalize(vec3(
			de(pos+e.yxx)-de(pos-e.yxx),
			de(pos+e.xyx)-de(pos-e.xyx),
			de(pos+e.xxy)-de(pos-e.xxy)
			)
		);	
}

vec3 getSceneColor( in vec3 from, in vec3 dir )
{
	float time = 100.0;
	//raymarch
	float totdist=0.;
	float distfade=1.;
	float glassfade=1.;
	float intens=1.;
	float maxdist=80.;
	float vol=0.;
	vec3 spec=vec3(0.);
	float ref=0.;
	vec3 light1=normalize(vec3(cos(time),sin(time*3.)*.5,sin(time)));
	vec3 light2=normalize(vec3(cos(time),sin(time*3.)*.5,-sin(time)));
	for (int r=0; r<80; r++) {
		vec3 p=from+totdist*dir;
		float d=de(p);
		float distfade=exp(-1.5*pow(totdist/maxdist,1.5));
		intens=min(distfade,glassfade);

		// refraction
		if (d>0.0 && ref>.5) {
			ref=0.;
			vec3 n=normal(p);
			if (dot(dir,n)<-.5) dir=normalize(refract(dir,n,1./.87));
			vec3 refl=reflect(dir,n);
			spec+=lightcol1*pow(max(dot(refl,light1),0.0),40.)*intens*.7;
			spec+=lightcol2*pow(max(dot(refl,light2),0.0),40.)*intens*.7;

		}
		if (d<0.0 && ref<.05) {
			ref=1.;
			vec3 n=normal(p);
			if (dot(dir,n)<-.05) dir=normalize(refract(dir,n,.87));
			vec3 refl=reflect(dir,n);
			glassfade*=.75;
			spec+=lightcol1*pow(max(dot(refl,light1),0.0),40.)*intens;
			spec+=lightcol2*pow(max(dot(refl,light2),0.0),40.)*intens;
		}
		
		totdist+=max(0.005,abs(d)); //advance ray 
		if (totdist>maxdist) break; 

		vol+=intens; //accumulate current intensity
	}
	
	vol=pow(vol,1.5)*.0005;
	vec3 col=vec3(vol)+vec3(spec)*.4+vec3(.05);

	//lights
	col+=1.5*lightcol1*pow(max(0.,max(0.,dot(dir,light1))),10.)*glassfade; 
	col+=1.5*lightcol2*pow(max(0.,max(0.,dot(dir,light2))),10.)*glassfade; 
	//col+=vec3(sin(time*10.)+1.,0.,0.)*.8*pow(max(0.,max(0.,dot(dir,vec3(0.,0.,1.)))),5.)*glassfade; 
	
	//col*=min(1.,time); //fade in
	return col;
}


#if 0
void main(void)
{
	float time = 100.0;// iGlobalTime*.6; 

	//camera
	mat3 rotview=rotmat(normalize(vec3(1.)),sin(time*.6));
	vec2 coord = gl_FragCoord.xy / iResolution.xy *2. - vec2(1.);
	coord.y *= iResolution.y / iResolution.x;
	float fov=min((time*.2+.05),0.8); //animate fov at start
	vec3 from = vec3(cos(time)*2.,sin(time*.5)*10.,time*5.);

	vec3 dir=normalize(vec3(coord.xy*fov,1.))*rotview; 
	vec3 col = getSceneColor( from, dir );
	gl_FragColor = vec4(col,1.0);	
}
#endif
