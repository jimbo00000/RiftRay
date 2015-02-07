// @var title sierpinski_twister
// @var author Mana
// @var url https://www.shadertoy.com/view/MlX3W8

// @var eyePos -6.8 -0.8 0.33

#define MAX_RAY_STEPS 80
#define MAX_RAY_DISTANCE 100.0
#define DISTANCE_EPSILON 0.012
#define PI 3.141592


vec3 gammaCorrect(vec3 c)
{
    return pow(c, vec3(2.2));
}

mat3 rotationMatrix(vec3 axis, float angle)
{
    float ca = cos(angle);
    float ica = 1.0 - ca;
    float sa = sin(angle);
    
	return mat3(ca + axis.x*axis.x * ica, axis.x*axis.y*ica - axis.z*sa, axis.x*axis.z*ica + axis.y*sa,
                axis.y*axis.x*ica + axis.z*sa, ca + axis.y*axis.y*ica, axis.y*axis.z*ica - axis.x*sa,
                axis.z*axis.x*ica - axis.y*sa, axis.z*axis.y*ica + axis.x*sa, ca + axis.z*axis.z*ica);

}
 //[0->1->0]
float linearInterval(vec2 interval, float t)
{
    float intervalC = interval.x +  (interval.y - interval.x) * 0.5;
    return step(t, intervalC) * smoothstep(interval.x, intervalC, t) + step(intervalC,t) * (1.0 - smoothstep(intervalC, interval.y, t));
     
}

vec3 fold(vec3 p, vec4 plane)
{
	return p - 2.0 * min(0.0, dot(p, plane.xyz) + plane.a) * plane.xyz;    
}

float opComplement(float a, float b)
{
	return max(-a,b);    
}

vec2 estimateDistance(vec3 p, float phase)
{
    
    const float scale = 2.0;
    const int n = 16;
    const vec2 rotInterv = vec2(0.08,1.92);
    const vec2 transInterval = vec2(0.9, 1.1);
    float repr = pow(scale, -float(n));
    const float size = 3.0;
    
    //calculate current point rotation
    float t = linearInterval(rotInterv, phase);
   	mat3 rot = rotationMatrix(normalize(vec3(1.2, 0.5, 3.0)), PI * 0.6 * t) 
        * rotationMatrix(normalize(vec3(0.2, 0.5, 0.0)), PI * 0.4 * t);
    
    //fractal1
    vec3 fp1 = p;
    fp1.xz = -abs(fp1.xz);
    fp1.y = abs(fp1.y);

	for( int i = 0; i < n; ++i)
    {
        
        fp1 = rot * fp1;
    	fp1 = fold(fp1, vec4(0.7, 0.7, 0.0, size));
        fp1 = fold(fp1, vec4(0.7, 0.0, -0.7, size));
        fp1 = fold(fp1, vec4(0.0, -0.7,  0.7, size));
        

        fp1 = fp1*scale;
    }
	
    //fractal2 (sierpinski tetrahedrons)     
    vec3 fp2 = p;
    fp2 = -abs(fp2);

	for( int i = 0; i < n; ++i)
    {
        
        fp2 = rot * fp2;
    	fp2 = fold(fp2, vec4(0.7, 0.7, 0.0, size));
        fp2 = fold(fp2, vec4(0.7, 0.0, 0.7, size));
        fp2 = fold(fp2, vec4(0.0, 0.7, 0.7, size));
        

        fp2 = fp2*scale;
    }
    
    //merge
    t = smoothstep(transInterval.x, transInterval.y, phase);
    float fd1 = length(fp2) * repr;
	float fd2 = opComplement( length(p) - 0.6,length(fp1) * repr); 
    vec2 fd = vec2(t * fd1 + (1.0 - t)*fd2, 0.0);
    
    
    
    return fd;              
}

vec3 calculateNormal(vec3 p, float phase)
{
    const vec3 e = vec3( 0.002, 0.0, 0.0);
    
    
	return normalize(vec3( estimateDistance(p + e,phase).x - estimateDistance(p - e,phase).x,
                 estimateDistance(p + e.yxz, phase).x - estimateDistance(p - e.yxz, phase).x,
               estimateDistance(p + e.zyx, phase).x - estimateDistance(p - e.zyx, phase).x));    
}




vec3 raymarch(vec3 ro, vec3 rd, float phase)
{
	float d = 0.0;
    float didHit = -1.0;
    float lastId = -1.0;
    int steps;
    for(int i = 0; i < MAX_RAY_STEPS; ++i)
    {
        
    	vec2 dist = estimateDistance( ro + rd * d, phase);
        if(dist.x < DISTANCE_EPSILON)
        {
            
            didHit = 1.0;
        	break;
        }
        d += dist.x;
        lastId = dist.y;
        if(d > MAX_RAY_DISTANCE)
        {
            didHit = -1.0;
            d = MAX_RAY_DISTANCE;
            break;
        }
        steps = i;
    }
    return vec3(d * didHit, lastId, float(steps));
}



vec3 calculateColor(vec3 ro, vec3 rd, float phase)
{
    vec3 g1 = vec3(0.9, 0.4, 0.1);
    vec3 g2 = vec3(0.1, 0.7, 0.3);
    
    const vec3 fc = vec3(0.4, 0.4, 0.4);
    
	vec3 res = raymarch(ro, rd, phase);

   
    vec3 c = vec3( 0.0, 0.0, 0.0);
    if(res.x > 0.0)
    {
        vec3 p = ro + rd * res.x;
    	vec3 n = calculateNormal(p, phase);
		c += clamp(0.0, 1.0, dot(n, -rd)) * fc;
    }
    
    //add glow
    float t = smoothstep(0.0, 2.0, phase);
    vec3 g = mix(g1,g2,t);
    c +=  (res.z / float(MAX_RAY_STEPS)) * g * 3.5;
    
    
	return c;    
}



vec3 viewCoordinatesToRayDir(vec3 eyeDir, vec3 eyeUp,  float fovy, float near, float aspect, vec2 screen_coord) 
{

	float projY = tan(fovy * 0.5) * 2.0 * near;
	float projX = projY * aspect;
  

	vec3 right = normalize( cross(eyeDir, eyeUp) );
  

	float dx = projX * (screen_coord.x - 0.5);
	float dy = projY * (screen_coord.y - 0.5);
  
	return normalize( eyeDir * near + dx * right + dy * eyeUp);
}

vec3 sphericalToCartesian(float polar, float azimuth, float r)
{
	float sinp = sin(polar);
    float sina = sin(azimuth);
    float cosp = cos(polar);
    float cosa = cos(azimuth);
    
    return vec3( r * cosa * sinp , r * sina * sinp,  r * cosp);
}

vec3 getSceneColor(in vec3 ro, in vec3 rd)
{
    float phase = cos(iGlobalTime * 0.2 + PI * 0.5) + 1.0;
    return gammaCorrect(calculateColor(ro,rd,phase));
}

#ifndef RIFTRAY
void main(void)
{
	vec2 vp = gl_FragCoord.xy / iResolution.xy;
    
    vec2 mp = iMouse.xy / iResolution.xy;
   
    float phase = cos(iGlobalTime * 0.2 + PI * 0.5) + 1.0;
    
    float camRad = mix(3.3, 12.0, 1.0 - mp.y);
    
    float azimuth = mp.x * 2.0 * PI + PI*0.5;
    float polar = 0.6 * PI;
    
    vec3 eyePos = sphericalToCartesian(polar,azimuth,camRad);
    vec3 eyeDir = normalize(-eyePos);
    vec3 eyeUp = normalize(sphericalToCartesian(polar+ 0.5 * PI, azimuth , camRad));
    float near = 0.1;
    float fovy = radians(60.0);
    float aspect =iResolution.x / iResolution.y;
    
    
    vec3 rd = viewCoordinatesToRayDir(eyeDir, eyeUp, fovy, near, aspect, vp);
    vec3 ro = eyePos;
    
    gl_FragColor = vec4(getSceneColor(ro, rd), 1.0); 
}
#endif
