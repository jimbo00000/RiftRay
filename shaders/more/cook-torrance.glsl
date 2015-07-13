////////////////////////////////////////
// Classic raytracing
// Cook-Torrance shading 
//
// The shaders displays 3 series of balls with different materials:
// - Ground: Basic (no reflection, no refraction), roughness and density varying foreach ball.
// - Along blue wall: Reflective materials, roughness and density varying foreach ball.
// - Along orange wall: Refractive materials,roughness and density varying foreach ball.
// - Center: the ball at the center is both reflective and refractive.
//
// Hard shadow are supported but enabled only for the ground balls.
//

// @var title Cook-Torrance
// @var author xbe
// @var url https://www.shadertoy.com/view/XsXXDB

// @var eyePos 0.0 4.4 -7.0

struct Material {
	vec3  color;		// diffuse color
	bool reflection;	// has reflection 
	bool refraction;	// has refraction
	float n;			// refraction index
	float roughness;	// Cook-Torrance roughness
	float fresnel;		// Cook-Torrance fresnel reflectance
	float density;		// Cook-Torrance color density i.e. fraction of diffuse reflection
};

struct Light {
	vec3 pos;
	vec3 color;
};

//////////////////////////////////////
/// Ray-Primitive intersections
/// fast version test the existence of 
/// an intersection

struct Inter {
	vec3 p;		//pos
	vec3 n; 	//normal
	vec3 vd;	// viewdir
	float d;	//distance
	bool inside; // inside object
	Material mat; // object material
};

float fastintSphere(vec3 ro, vec3 rd, vec3 p, float r)
{
	float dist = -1.;
	vec3 v = ro-p;
	float b = dot(v,rd);
	float c = dot(v,v) - r*r;
	float d = b*b-c;
	if (d>0.)
	{
		float t1 = (-b-sqrt(d));
		float t2 = (-b+sqrt(d));
		if (t2>0.)
			dist = t1>0.?t1:t2;
	}
	return dist;
}

void intSphere(vec3 ro, vec3 rd, vec3 p, float r, Material mat, inout Inter i)
{
	float dist = -1.;
	vec3 v = ro-p;
	float b = dot(v,rd);
	float c = dot(v,v) - r*r;
	float d = b*b-c;
	if (d>0.)
	{
		float t1 = (-b-sqrt(d));
		float t2 = (-b+sqrt(d));
		if (t2>0.)
		{
			dist = t1>0.?t1:t2;
			if ((dist<i.d)||(i.d<0.))
			{
				i.p = ro+dist*rd;
				i.n = normalize(i.p-p);
				i.d = dist;
				i.vd = -rd;
				i.inside = t1<0.;
				if (i.inside)
					i.n *= -1.; //invert the normal when hitting inside during refraction
				i.mat = mat;
			}
		}
	}
}

float fastintPlane(vec3 ro, vec3 rd, vec3 p, vec3 n)
{
	float res = -1.;
	float dpn = dot(rd,n);
	if (abs(dpn)>0.00001)
		res = (-(dot(n, p) + dot(n,ro)) / dpn);
	return res;
}

bool intPlane(vec3 ro, vec3 rd, vec3 p, vec3 n, Material mat, inout Inter i)
{
	float d = -1.;
	float dpn = dot(rd,n);
	if (abs(dpn)>0.00001)
	{
		d = -(dot(n, p) + dot(n,ro)) / dpn;
		if ((d>0.)&&((d<i.d)||(i.d<0.)))
		{
			i.p = ro+d*rd;
			i.n = n;
			i.d = d;
			i.vd = -rd;
			i.inside = false;
			i.mat = mat;
		}
	}
	return (i.d==d);
}

//////////////////////////////////////
/// Shading functions
vec3 shadeBlinnPhong( Inter i, vec3 lp )
{
	float diffuse = 0.6;
	float specular = 0.4;
	
	vec3 res = vec3(0.);
	vec3 ld = normalize(lp-i.p);
	res = i.mat.color*diffuse*dot(i.n,ld);
	vec3 h = normalize(i.vd+ld);
	res += specular*pow(dot(i.n,h), 16.);
	return res;
}

vec3 shadePhong( Inter i, vec3 lp )
{
	float diffuse = 0.6;
	float specular = 0.4;
	
	vec3 res = vec3(0.);
	vec3 ld = normalize(lp-i.p);
	res = i.mat.color*diffuse*dot(i.n,ld);
	res += specular*pow( clamp(dot(reflect(i.vd,i.n),ld),0.,1.), 16.);
	return res;
}

/// References:
/// http://content.gpwiki.org/index.php/D3DBook:%28Lighting%29_Cook-Torrance
/// http://ruh.li/GraphicsCookTorrance.html
vec3 shadeCookTorrance( Inter i, Light lig )
{
	float roughness = i.mat.roughness;
	float F0 = i.mat.fresnel;
	float K = i.mat.density;
	//
	vec3 ld = normalize(lig.pos-i.p);
	vec3 h = normalize(i.vd+ld);
	float NdotL = clamp( dot( i.n, ld ),0.,1. );
	float NdotH = clamp( dot( i.n, h ),0.,1. );
	float NdotV = clamp( dot( i.n, i.vd ),0.,1. );
	float VdotH = clamp( dot( h, i.vd ),0.,1. );
	float rsq = roughness * roughness;
	
	// Geometric Attenuation
	float NH2   = 2. * NdotH / VdotH;
	float geo_b = (NH2 * NdotV );
	float geo_c = (NH2 * NdotL );
	float geo   = min( 1., min( geo_b, geo_c ) );
	
	// Roughness
	// Beckmann distribution function
	float r1 = 1. / ( 4. * rsq * pow(NdotH, 4.));
	float r2 = (NdotH * NdotH - 1.) / (rsq * NdotH * NdotH);
	float rough = r1 * exp(r2);
	
	// Fresnel			
	float fres = pow( 1.0 - VdotH, 5. );
	fres *= ( 1.0 - F0 );
	fres += F0;
	
	vec3 spec = (NdotV * NdotL==0.) ? vec3(0.) : vec3 ( fres * geo * rough ) / ( NdotV * NdotL );
	vec3 res = NdotL * ( (1.-K)*spec + K*i.mat.color ) * lig.color;// * exp(-0.001*length(lig.pos-i.p));
	return res;
}

////////////////////////////////////
// Raytracing

float hidden( Inter i, vec3 lp)
{
	vec3 ro = i.p;
	float dmax = length(lp-ro);
	vec3 rd = normalize(lp-ro);
	ro += 0.001*rd;
	//
	float hit = -1.;
	vec3 p = vec3(0.,0.,0.);
	vec3 n = vec3(0.,1.,0.);
	hit = fastintPlane( ro, rd, p, n);
	hit = hit>dmax?-1.:hit;
	//
	if (hit<0.)
	{
		float pi = 1.25;
		p = vec3(-2.5,0.5,-2.5);
		for (int k=0; k<5; ++k)
		{
			p.z = -2.5;
			for (int l=0;l<5;++l)
			{
				hit = fastintSphere( ro, rd, p, 0.5);
				if ((hit>0.) && (hit<dmax)) break;
				p.z += pi;
			}
			if (hit>0.) break;
			p.x += pi;
		}
	}
	return hit;
}

vec3 raytraceRay( vec3 ro, vec3 rd, inout Inter i)
{
	Material mat;
	mat.color = vec3(0.75);
	mat.reflection = false;
	mat.refraction = false;
	mat.n = 1.;
	mat.fresnel = 0.8;
	mat.roughness = 1.;
	mat.density = 1.;
	vec3 p = vec3(0.,0.,0.);
	vec3 n = vec3(0.,1.,0.);
	if (intPlane( ro, rd, p, n, mat, i))
	{
		// checker plane hack
		i.mat.color = vec3(0.75)*mod(floor(i.p.x)+floor(i.p.z),2.)+0.25;
	}
	//
	p = vec3(-8.,0.,0.);
	n = vec3(-1.,0.,0.);
	if (intPlane( ro, rd, p, n, mat, i))
	{
		// checker plane hack
		i.mat.color = vec3(0.95,0.35,0.)*mod(floor(i.p.y)+floor(i.p.z),2.)+0.25;
	}
	//
	p = vec3(0.,0.,8.);
	n = vec3(0.,0.,1.);
	if (intPlane( ro, rd, p, n, mat, i))
	{
		// checker plane hack
		i.mat.color = vec3(0.35,0.65,0.95)*mod(floor(i.p.x)+floor(i.p.y),2.)+0.25;
	}
	//
	mat.color = vec3(1.0,1.0,0.25);
	mat.reflection = false;
	mat.refraction = false;
	mat.n = 1.;
	mat.fresnel = 0.8;
	mat.roughness = 0.1;
	mat.density = 0.95;
	float pi = 1.25;
	float ri = 0.2;
	p = vec3(-2.5,0.5,-2.5);
	for (int k=0; k<5; ++k)
	{
		mat.roughness = 0.1;
		p.z = -2.5;
		for (int l=0; l<5; ++l)
		{
			intSphere( ro, rd, p, 0.5, mat, i);
			mat.roughness += ri;
			p.z += pi;
		}
		mat.density -= ri;
		p.x += pi;
	}
	//
	mat.color = vec3(1.0,1.0,0.25);
	mat.reflection = true;
	mat.refraction = false;
	mat.n = 1.;
	mat.fresnel = 0.8;
	mat.roughness = 0.1;
	mat.density = 0.95;
	pi = 1.25;
	ri = 0.2;
	p = vec3(-2.5,1.,-4.);
	for (int k=0; k<5; ++k)
	{
		mat.roughness = 0.1;
		p.y = 1.;
		for (int l=0; l<5; ++l)
		{
			intSphere( ro, rd, p, 0.5, mat, i);
			mat.roughness += ri;
			p.y += pi;
		}
		mat.density -= ri;
		p.x += pi;
	}
	//
	mat.color = vec3(1.0,1.0,0.25);
	mat.reflection = false;
	mat.refraction = true;
	mat.n = 1.16;
	mat.fresnel = 0.8;
	mat.roughness = 0.9;
	mat.density = 0.15;
	pi = 1.25;
	ri = 0.2;
	p = vec3(4.,1.,2.5);
	for (int k=0; k<5; ++k)
	{
		mat.density = 0.15;
		p.y = 1.;
		for (int l=0; l<5; ++l)
		{
			intSphere( ro, rd, p, 0.5, mat, i);
			mat.density += ri;
			p.y += pi;
		}
		mat.roughness -= ri;
		p.z -= pi;
	}
	//
	mat.color = vec3(0.0,1.0,1.0);
	mat.reflection = true;
	mat.refraction = true;
	mat.n = 1.33;
	mat.fresnel = 0.8;
	mat.roughness = .1;
	mat.density = 0.5;
	p = vec3(0.,4.0,0.);
	intSphere( ro, rd, p, 1.5, mat, i);
	//
	vec3 col = vec3(0.1,0.1,0.1);
	if (i.d>0.)
	{
		// ambiant
		float ambiant = 0.1;
		col = ambiant*i.mat.color;
		
		if (!i.inside)
		{
			// lighting
			Light lig;
			lig.color = vec3(1.,1.,1.);
			lig.pos = vec3(0., 6., 0.);
			if (hidden(i,lig.pos)<0.)
				col += 0.5*shadeCookTorrance(i, lig);
			lig.pos = vec3(-4., 6., -4.);
			if (hidden(i,lig.pos)<0.)
				col += 0.5*shadeCookTorrance(i, lig);
		}
	}
	return clamp(col,0.,1.);
}

vec3 raytrace( vec3 ro, vec3 rd)
{
	Inter i;
	i.p = vec3(0.,0.,0.);
	i.n = vec3(0.,0.,0.);
	i.d = -1.;
	i.vd = vec3(0.,0.,0.);
	i.inside = false;
	//
	vec3 accum = vec3(0.);
	vec3 col = vec3(0.);
	float refl = 1.;
	float refr = 1.;
	col = raytraceRay(ro, rd, i);
	accum += col; // * exp(-0.0005*i.d*i.d);
	if (i.mat.reflection)
	{
		Inter li = i;
		vec3 lro = ro;
		vec3 lrd = rd;
		lro = li.p;
		lrd = reflect(-li.vd,li.n);
		lro += 0.0001*lrd;
		for (int k=1; k<4; ++k)
		{
			li.d = -1.;
			refl *= 1.-i.mat.density;
			//
			col = raytraceRay(lro, lrd, li);
			//
			accum += col * refl; // * exp(-0.005*i.d*i.d);
			if ((li.d<.0)||(!li.mat.reflection)) break;
			lro = li.p;
			lrd = reflect(-li.vd,li.n);
			lro += 0.0001*lrd;
		}
	}
	if (i.mat.refraction)
	{
		Inter li = i;
		vec3 lro = ro;
		vec3 lrd = rd;
		float n = 1./li.mat.n;
		float cosI = -dot(li.n,li.vd);
		float cost2 = 1.-n*n*(1.-cosI*cosI);
		if (cost2>0.)
		{
			lro = li.p;
			lrd = normalize(-li.vd*n+li.n*(n*cosI - sqrt(cost2)));
			lro += 0.0001*lrd;
			for (int k=1; k<4; ++k)
			{
				li.d = -1.;
				refr *= 1.-li.mat.density;
				//
				col = raytraceRay(lro, lrd, li);
				//
				accum += col * refr; //* exp(-0.005*i.d*i.d);
				if ((li.d<.0)||(!li.mat.refraction)) break;
				if (li.inside)
					n = li.mat.n;
				else
					n = 1./li.mat.n;
				cosI = -dot(li.n,li.vd);
				cost2 = 1.-n*n*(1.-cosI*cosI);
				if (cost2<=0.) break;
				lro = li.p;
				lrd = normalize(-li.vd*n+li.n*(n*cosI - sqrt(cost2)));
				lro += 0.0001*lrd;
			}
		}
	}
	return clamp(accum,0.,1.);
}

vec3 getSceneColor(in vec3 ro, in vec3 rd)
{
	ro = ro.zyx;
	rd = rd.zyx;
    vec3 col = raytrace( ro, rd );
    return col;
}

#ifndef RIFTRAY
void main(void)
{
	vec2 q = gl_FragCoord.xy/iResolution.xy;
    vec2 p = -1.0+2.0*q;
	p.x *= iResolution.x/iResolution.y;
		 
	float Time = 0.45*(15.0 + iGlobalTime);
	// camera	
	vec3 ro = vec3( 8.0*cos(Time), 6.0, 8.0*sin(Time) );
//	vec3 ro = vec3( -8.0, 6.0, 8.0 );
	vec3 ta = vec3( 0.0, 2.5, 0. );

	vec2 m = iMouse.xy / iResolution.xy;
	if( iMouse.z>0.0 )
	{
		float hd = -m.x * 14.0 + 3.14159;
		float elv = m.y * 3.14159 * 0.4 - 3.14159 * 0.25;
		ro = vec3(sin(hd) * cos(elv), sin(elv), cos(hd) * cos(elv));
		ro = ro * 8.0 + vec3(0.0, 6.0, 0.0);
	}
	
	// camera tx
	vec3 cw = normalize( ta-ro );
	vec3 cp = vec3( 0.0, 1.0, 0.0 );
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
	vec3 rd = normalize( p.x*cu + p.y*cv + 2.5*cw );

	vec3 col = getSceneColor(ro, rd);
	gl_FragColor=vec4( col, 1.0 );
}
#endif
