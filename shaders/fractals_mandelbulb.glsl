// @var url https://github.com/rajabala/Fractals/blob/master/Assets/Mandelbulb/Mandlebulb.shader
// @var author rajabala @github
// @var title Mandelbulb from Fractals

// @var eyePos 0.7 -.11 -1.5

#define black vec4(0,0,0,1)
#define green vec4(0,1,0,1)	
#define white vec4(1,1,1,1)
#define blue  vec4(0,0,0.7,1)		
#define red vec4(1,0,0,1)	
#define orange vec4(1, 0.64, 0,1)
#define yellow  vec4(1,1,0,1)	
#define seethrough vec4(0,0,0,0)


float		_Exponent = 10. + 5. * sin(.1 * iGlobalTime);
int			_NumIterations = 32;
int			_NumRayMarchSteps = 32;

/********* math stuff ******************/
vec2 
complex_mult(vec2 c1, vec2 c2)
{
	return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

vec2 
recurse_complex_mult(vec2 cin, int n)
{
	int ii;
	vec2 cout = cin;

	for(ii = 0; ii < n; ii++)
	{
		cout = complex_mult(cout, cin);
	}

	return cout;
}


// theta -- angle vector makes with the XY plane
// psi -- angle the projected vector on the XY plane makes with the X axis
// note: this might not be the usual convention
void 
cartesian_to_polar(vec3 p, out float r, out float theta, out float psi)
{
	r = length(p);
	float r1 = p.x*p.x + p.y*p.y;
	theta = atan(p.z / r1); // angle vector makes with the XY plane
	psi	= atan(p.y / p.x); // angle of xy-projected vector with X axis
}


// theta -- angle vector makes with the XY plane
// psi -- angle the projected vector on the XY plane makes with the X axis
// note: this might not be the usual convention	
void
polar_to_cartesian(float r, float theta, float psi, out vec3 p)
{
	p.x = r * cos(theta) * cos(psi);
	p.y = r * cos(theta) * sin(psi);
	p.z = r * sin(theta);
}


// [-1,1] to [0,1]
float
norm_to_unorm(float i)
{
	return (i + 1) * 0.5;
}



/*************************************** distance estimators ******************************/
// distance to the surface of a sphere
float
de_sphere_surface(vec3 p, vec3 c, float r)
{
	return abs(length(p - c) - r);
}


// instancing can be done really cheap. the function below creates infinite spheres on the XY plane
float
de_sphere_instances(vec3 p)
{
	p.xy = mod( (p.xy), 1.0 ) - 0.5;
	return length(p) - 0.2;
}

// distance estimator for the mandelbulb	
float
de_mandelbulb(vec3 c)
{		
	// i believe that similar to the mandelbrot, the mandelbulb is enclosed in a sphere of radius 2 (hence the delta) 
	const float delta = 2;	
	
	bool converges = true; // unused
	float divergenceIter = 0; // unused
	vec3 p = c;
	float dr = 2.0, r = 1.0;

	int ii;
	for(ii = 0; ii < _NumIterations; ii++)
	{			
		// equation used: f(p) = p^_Exponent + c starting with p = 0			

		// get polar coordinates of p
		float theta, psi;
		cartesian_to_polar(p, r, theta, psi);

		// rate of change of points in the set
		dr = _Exponent * pow(r, _Exponent - 1) *dr + 1.0;

		// find p ^ _Exponent
		r = pow(r,_Exponent);
		theta *= _Exponent;
		psi *= _Exponent;

		// convert to cartesian coordinates
		polar_to_cartesian(r, theta, psi, p);
		
		// add c
		p += c;

		// check for divergence
		if (length(p) > delta) {
			divergenceIter = ii;
			converges = false;
			break;
		}
	}

	return log(r) * r / dr; // Greens formula
}

// iterate through scene objects to find min distance from a point to the scene
float 
de_scene(vec3 p)
{
	//float ds1 = de_sphere_surface(p, vec3(6,0,0), 0.9);
	//return ds1;
	
	float ds2 = de_mandelbulb(p);		
	return ds2;//min(ds1, ds2);
}


// Raymarch scene given an origin and direction using sphere tracing
// Each step, we move by the closest distance to any object in the scene from the current ray point.
// We stop when we're really close to an object (or) if we've exceeded the number of steps
// Returns a greyscale color based on number of steps marched (white --> less, black --> more)
vec4
raymarch(vec3 rayo, vec3 rayd) 
{			
	const float minimumDistance = 0.0001;
	vec3 p = rayo;
	bool hit = false;		
	float distanceStepped = 0.0;
	int steps;

	for(steps = 0; steps < _NumRayMarchSteps; steps++)
	{			
		float d = de_scene(p);
		distanceStepped += d;

		if (d < minimumDistance) {
			hit = true;
			break;
		}

		p += d * rayd;
	}			

	float greyscale = 1 - (steps/float(_NumRayMarchSteps));
	return vec4(greyscale, greyscale, greyscale, 1);			
}

vec3 getSceneColor(in vec3 ro, in vec3 rd)
{
	return raymarch(ro,rd).xyz;
}
