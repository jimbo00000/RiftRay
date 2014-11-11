// @var title Random spheres
// @var author chalky
// @var url https://www.shadertoy.com/view/4s23zR

vec3 Q1=vec3(0,0,0);
vec3 Q2=vec3(1,0,0);
vec3 Q3=vec3(0,1,0);
vec3 Q4=vec3(1,1,0);

float tan_fov = 1.1917;
float fov = 0.8726646;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

bool calc_light(vec3 ray_origin, vec3 ray_dir, vec3 light_pos, out float q)
{
	bool found = false;
	
	vec3 d = -light_pos + ray_origin;
	float r = 0.15;
	
	float A = dot(ray_dir, ray_dir);
	float B = 2.0*dot(ray_dir, d);
	float C = dot(d, d) - r*r;
	
	float discriminant = B*B - 4.0*A*C;
	if( discriminant > 0.0 )
	{
		q = -(B + sqrt(discriminant))/(2.0*A);
		found = (q > 0.0);
	}
	return found;
}

bool calc_sphere(vec3 point, vec3 ray_origin, vec3 ray_dir, vec3 offset, out vec3 norm, out float q)
{
	vec3 ixyz = floor(point)-offset; 
	vec3 centre = ixyz + 0.5; //sphere centres
	centre.x += rand(ixyz.xy);
	centre.y += rand(ixyz.xz);
	//centre.z += random3(ixyz.yz);
	
	vec3 d = point - centre; //from centres of spheres to point
	bool found = false;
	
	float t;

	float r = 0.05 + 0.35*rand(ixyz.xy+150.0*ixyz.z);
	
	float A = dot(ray_dir, ray_dir);
	float B = 2.0*dot(ray_dir, d);
	float C = dot(d, d) - r*r;
	
	float discriminant = B*B - 4.0*A*C;
	
	if( discriminant > 0.0 )
	{
		t = -(B + sqrt(discriminant))/(2.0*A);
		
		vec3 p = d + t*ray_dir; //vector from centre to surface of sphere
		
		q = dot(point + t*ray_dir - ray_origin, ray_dir);
		if(q > 0.0)
		{
			norm = normalize(p);
			found = true;
		}
	}    
	
	return found;
}

vec3 getSceneColor( vec3 origin, vec3 ray )
{
	vec3 light = origin + vec3(sin(iGlobalTime)+0.3*sin(3.0*iGlobalTime), cos(iGlobalTime), 3.0+cos(iGlobalTime));
	vec3 ixyz;

    vec3 sphere_colour = vec3(1.0,1.0,1.0);
	vec4 pixel = vec4(0.0,0.0,0.0,1.0);
	vec3 point;
	vec3 exact_point;
	vec3 norm;
	vec3 final_normal;
	vec3 light_ray;
	float t = 0.0;//3.0;//15.0;
	float direct = 0.0;
	float refl;
	
	bool found;
	int found_count = 0;
	
	float to_camera = 100000.0;
	float q;
	
	if(calc_light(origin, ray, light, q))
	{
		direct = 1.0;
		to_camera = q;
		refl = 0.0;
		sphere_colour = vec3(1.0,1.0,1.0);
		final_normal = vec3(1,1,1);
		exact_point = vec3(0,0,0);
	}
	
	for(int n = 0; n < 20 ; n++)
	{
		point = origin + t*ray;
		if(pixel.w > 0.01)
		{
			if(found_count < 3)
			//if(true)
			{
				found = false;
				if(calc_sphere(point, origin, ray, Q1, norm, q) )
				{
					if(q < to_camera)
					{
						exact_point = origin+q*ray;
						light_ray = light-exact_point;
						direct = dot(norm, light_ray)/dot(light_ray, light_ray);
						to_camera = q;
						ixyz = floor(point)-Q1;
						sphere_colour.g = 0.2 + 0.5*rand(ixyz.xy+200.0);
						sphere_colour.b = 0.2 + 0.5*rand(ixyz.xz+200.0);
						final_normal = norm;
						refl = 0.05;
					}
					found=true;
				}
				
				
				if(calc_sphere(point, origin, ray, Q2, norm, q) )
				{
					if(q < to_camera)
					{
						exact_point = origin+q*ray;
						light_ray = light-exact_point;
						direct = dot(norm, light_ray)/dot(light_ray, light_ray);
						to_camera = q;
						ixyz = floor(point)-Q2;
						sphere_colour.g = 0.2 + 0.5*rand(ixyz.xy+200.0);
						sphere_colour.b = 0.2 + 0.5*rand(ixyz.xz+200.0);
						final_normal = norm;
						refl = 0.05;
					}
					found=true;
				}
				
				if(calc_sphere(point, origin, ray, Q3, norm, q) )
				{
					if(q < to_camera)
					{
						exact_point = origin+q*ray;
						light_ray = light-exact_point;
						direct = dot(norm, light_ray)/dot(light_ray, light_ray);
						to_camera = q;
						ixyz = floor(point)-Q3;
						sphere_colour.g = 0.2 + 0.5*rand(ixyz.xy+200.0);
						sphere_colour.b = 0.2 + 0.5*rand(ixyz.xz+200.0);
						final_normal = norm;
						refl = 0.05;
					}
					found=true;
				}
				
				
				if(calc_sphere(point, origin, ray, Q4, norm, q) )
				{
					if(q < to_camera)
					{
						exact_point = origin+q*ray;
						light_ray = light-exact_point;
						direct = dot(norm, light_ray)/dot(light_ray, light_ray);
						to_camera = q;
						ixyz = floor(point)-Q4;
						sphere_colour.g = 0.2 + 0.5*rand(ixyz.xy+200.0);
						sphere_colour.b = 0.2 + 0.5*rand(ixyz.xz+200.0);
						final_normal = norm;
						refl = 0.05;
					}
					found=true;
				}
				
				
				if((found)||(found_count != 0))found_count++;
				//if(found)found_count++;
				
				/*
				if(calc_sphere(point, origin, ray, vec3(0,0,1), norm, q) )
				{
					if(q < to_camera)
					{
						direct = dot(norm, vec3(0.0,0.0,-1.0));
						to_camera = q;	
					}
					//found = true;
				}
				if(calc_sphere(point, origin, ray, vec3(1,0,1), norm, q) )
				{
					if(q < to_camera)
					{
						direct = dot(norm, vec3(0.0,0.0,-1.0));
						to_camera = q;	
					}
					//found = true;
				}
				if(calc_sphere(point, origin, ray, vec3(0,1,1), norm, q) )
				{
					if(q < to_camera)
					{
						direct = dot(norm, vec3(0.0,0.0,-1.0));
						to_camera = q;	
					}
					//found = true;
				}
				if(calc_sphere(point, origin, ray, vec3(1,1,1), norm, q) )
				{
					if(q < to_camera)
					{
						direct = dot(norm, vec3(0.0,0.0,-1.0));
						to_camera = q;	
					}
					//found = true;
				}
				*/
			}
			else
			{	
				found_count = 0;
				
				
				direct = direct*(1.0 - to_camera/16.0);
				if(direct<0.0)direct=0.0;
					
				pixel.xyz += pixel.w*(1.0-refl)*sphere_colour*direct;
				pixel.w = pixel.w*refl;
					
				t = -0.5;//5;
				to_camera = 10000.0;
				ray = reflect(ray, final_normal);
				origin = exact_point;// + 1.01*ray;
				
				if(calc_light(origin, ray, light, q))
				{
					direct = 5.0;
					to_camera = q;
					refl = 0.0;
					sphere_colour = vec3(1.0,1.0,1.0);
					final_normal = vec3(1,1,1);
					exact_point = vec3(0,0,0);
				}
			}
		}
		
		t+=0.8;	
	}
	
	direct = direct*(1.0 - to_camera/16.0);
	if(direct<0.0)direct=0.0;
					
	pixel.xyz += pixel.w*(1.0-refl)*sphere_colour*direct;
	pixel.w = pixel.w*refl;
	//pixel.
	
	
	
	//gl_FragColor = vec4(to_camera/20.0, to_camera/20.0, to_camera/20.0, 1.0);
	
	//sphere_colour = direct*sphere_colour;
	
	//gl_FragColor = vec4(direct,direct,direct,1.0);
	//gl_FragColor = vec4(sphere_colour,1.0);
	
	pixel.r = pow(pixel.r,0.6);
	pixel.g = pow(pixel.g,0.6);
	pixel.b = pow(pixel.b,0.6);
	pixel.w = 1.0;
	//gl_FragColor = pixel;
    return pixel.xyz;
}

#ifndef RIFTRAY
void main(void)
{
	vec3 ray;
	ray.x = (2.0*gl_FragCoord.x/iResolution.y - iResolution.x/iResolution.y);
	ray.y = (2.0*gl_FragCoord.y/iResolution.y - 1.0);
	ray.z = (0.5/tan_fov) + 0.5*length(ray.xy)/(0.001+tan(fov*length(ray.xy)));
	
	ray = normalize(ray);//20.0;
	
	mat3 matrix = mat3(cos(0.1*iGlobalTime),sin(0.1*iGlobalTime),0.0,
					-sin(0.1*iGlobalTime),cos(0.1*iGlobalTime),
					0.0,0.0, 0.0, 1.0);
	ray=matrix*ray;
		
	vec3 origin = vec3(0.6*sin(0.5*iGlobalTime), 0.6*cos(0.5*iGlobalTime),iGlobalTime);
	
    vec3 col = getSceneColor( origin, ray );
    gl_FragColor = vec4(col, 1.0);
}
#endif
