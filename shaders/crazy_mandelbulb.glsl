// @var url https://www.shadertoy.com/view/MljGDz
// @var title Crazy Mandelbulb
// @var author inferno

// @var eyePos -0.73 0.48 -1.08
// @var headSize 0.18

/**
 * Fractal Lab's uber 3D fractal shader
 * Last update: 26 February 2011
 *
 * Changelog:
 *      0.1     - Initial release
 *      0.2     - Refactor for Fractal Lab
 *
 * 
 * Copyright 2011, Tom Beddard
 * http://www.subblue.com
 *
 * For more generative graphics experiments see:
 * http://www.subblue.com
 *
 * Licensed under the GPL Version 3 license.
 * http://www.gnu.org/licenses/
 *
 * 
 * Credits and references
 * ======================
 * 
 * http://www.fractalforums.com/3d-fractal-generation/a-mandelbox-distance-estimate-formula/
 * http://www.fractalforums.com/3d-fractal-generation/revenge-of-the-half-eaten-menger-sponge/msg21700/
 * http://www.fractalforums.com/index.php?topic=3158.msg16982#msg16982
 * 
 * Various other discussions on the fractal can be found here:
 * http://www.fractalforums.com/3d-fractal-generation/
 *
 *
*/

#define HALFPI 1.570796
#define MIN_EPSILON 6e-7
#define MIN_NORM 1.5e-7
#define minRange 6e-5
#define bailout 4.0


#define dE Mandelbulb 
int maxIterations = 8;
int stepLimit = 1000;
int aoIterations = 4;

float power = 4.09; 
float surfaceDetail = 0.68;       
float surfaceSmoothness = .48;
float boundingRadius = 4.73;

vec3 offset = vec3(5.32, -4.020, 22.94);
vec3 objRotation = vec3(17.8, 5.152, 7.301);
float juliaFactor = -0.02; 
float radiolariaFactor = 01.75;
float radiolaria = 1.2;

float cameraRoll = 100.40;
float cameraPitch = -14.0;  
float cameraYaw = 146.0;   
float cameraFocalLength = 0.4;
vec3 cameraPosition = vec3(-0.52, 0.47, -0.443);

int   colorIterations= 4;
vec3  color1 = vec3(1.0); 
float color1Intensity = 0.45;
vec3  color2 = vec3(0.0, 0.0, 1.0);
float color2Intensity = 0.3; 
vec3  color3 = vec3(0.8,0.0,0.2);
float color3Intensity = 0.0; 
float ambientColor = -4.39;
float ambientIntensity =1.0;
vec3  background1Color = vec3(1.0);
vec3  background2Color = vec3(1.0);

// Shading TAB
vec3  light = vec3(0.0, 0.0, 4.3);              
vec3  innerGlowColor = vec3(0.0);
float innerGlowIntensity = 5.32;
vec3  outerGlowColor = vec3(1.0);
float outerGlowIntensity = 0.0;
float fog = 8.0;                 
float fogFalloff = 10.46;        
float specularity = 0.46;        
float specularExponent = 0.0; 
float aoIntensity = 0.03;     
float aoSpread = 5.54;

#define pi 3.1415926535897932384624433832795

// Degrees to radians
float deg2rad(float angle) 
{
	return(angle/(180.0/pi));
}
vec3  w = vec3(0, 0, 1);
vec3  v = vec3(0, 1, 0);
vec3  u = vec3(1, 0, 0);
mat3  cameraRotation;

float x;
float y;
float z;
mat3 rx;
mat3 ry;
mat3 rz;
mat3 objectRotation ;


float aspectRatio;
float fovfactor;
float pixelScale;
float epsfactor;


// Return rotation matrix for rotating around vector v by angle
mat3 rotationMatrixVector(vec3 v, float angle)
{
    float c = cos(radians(angle));
    float s = sin(radians(angle));
    
    return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
              (1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
              (1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z);
}



// Scalar derivative approach by Enforcer:
// http://www.fractalforums.com/mandelbulb-implementation/realtime-renderingoptimisations/
void powN(float p, inout vec3 z, float zr0, inout float dr)
{
    float zo0 = asin(z.z / zr0);
    float zi0 = atan(z.y, z.x);
    float zr = pow(zr0, p - 1.0);
    float zo = zo0 * p;
    float zi = zi0 * p;
    float czo = cos(zo);

    dr = zr * dr * p + 1.0;
    zr *= zr0;

    z = zr * vec3(czo * cos(zi), czo * sin(zi), sin(zo));
}

vec3 Mandelbulb(vec3 w)
{
    w *= objectRotation;
    
    vec3 z = w;
    vec3 c = mix(w, offset, juliaFactor * sin(iGlobalTime * 0.03));
    vec3 d = w;
    float dr = 1.0;
    float r  = length(z);
    float md = 10000.0;
    
    for (int i = 0; i < 8; i++) {
        powN(power, z, r, dr);
        
        z += c;
            
        if (z.y > radiolariaFactor) {
            z.y = mix(z.y, radiolariaFactor, radiolaria);
        }
        
        r = length(z);
        
        if (i < colorIterations) {
            md = min(md, r);
            d = z;
        }
        
        if (r > bailout) break;
    }

    return vec3(0.5 * log(r) * r / dr, md, 0.33 * log(dot(d, d)) + 1.0);
}



// ============================================================================================ //



// Define the ray direction from the pixel coordinates
vec3 rayDirection(vec2 pixel)
{
    vec2 p = (0.5 * iResolution.xy - pixel) / vec2(iResolution.x, -iResolution.y);
    p.x *= aspectRatio;
    vec3 d = (p.x * u + p.y * v - cameraFocalLength * w);
    
    return normalize(cameraRotation * d);
}



// Intersect bounding sphere
//
// If we intersect then set the tmin and tmax values to set the start and
// end distances the ray should traverse.
bool intersectBoundingSphere(vec3 origin,
                             vec3 direction,
                             out float tmin,
                             out float tmax)
{
    bool hit = false;
    float b = dot(origin, direction);
    float c = dot(origin, origin) - boundingRadius;
    float disc = b*b - c;           // discriminant
    tmin = tmax = 0.0;

    if (disc > 0.0) {
        // Real root of disc, so intersection
        float sdisc = sqrt(disc);
        float t0 = -b - sdisc;          // closest intersection distance
        float t1 = -b + sdisc;          // furthest intersection distance

        if (t0 >= 0.0) {
            // Ray intersects front of sphere
            tmin = t0;
            tmax = t0 + t1;
        } else if (t0 < 0.0) {
            // Ray starts inside sphere
            tmax = t1;
        }
        hit = true;
    }

    return hit;
}




// Calculate the gradient in each dimension from the intersection point
vec3 generateNormal(vec3 z, float d)
{
    float e = max(d * 0.5, MIN_NORM);
    
    float dx1 = dE(z + vec3(e, 0, 0)).x;
    float dx2 = dE(z - vec3(e, 0, 0)).x;
    
    float dy1 = dE(z + vec3(0, e, 0)).x;
    float dy2 = dE(z - vec3(0, e, 0)).x;
    
    float dz1 = dE(z + vec3(0, 0, e)).x;
    float dz2 = dE(z - vec3(0, 0, e)).x;
    
    return normalize(vec3(dx1 - dx2, dy1 - dy2, dz1 - dz2));
}


// Blinn phong shading model
// http://en.wikipedia.org/wiki/BlinnPhong_shading_model
// base color, incident, point of intersection, normal
vec3 blinnPhong(vec3 color, vec3 p, vec3 n)
{
    // Ambient colour based on background gradient
    vec3 ambColor = clamp(mix(background2Color, background1Color, (sin(n.y * HALFPI) + 1.0) * 0.5), 0.0, 1.0);
    ambColor = mix(vec3(ambientColor), ambColor, ambientIntensity);
    
    vec3  halfLV = normalize(light - p);
    float diffuse = max(dot(n, halfLV), 0.0);
    float specular = pow(diffuse, specularExponent);
    
    return ambColor * color + color * diffuse + specular * specularity;
}

vec3 depthMatte(vec3 color, vec3 p, vec3 n)
{
    // Ambient colour based on background gradient
    vec3 depthColor = clamp(mix(background2Color, background1Color, (sin(n.y * HALFPI) + 1.0) * 0.5), 0.0, 1.0);
    depthColor = mix(vec3(ambientColor), depthColor, ambientIntensity);
    
    vec3  halfLV = normalize(light - p);
    float diffuse = max(dot(n, halfLV), 0.0);
    float specular = pow(diffuse, specularExponent);
    
    return depthColor * color + color * diffuse;
}

// Ambient occlusion approximation.
// Based upon boxplorer's implementation which is derived from:
// http://www.iquilezles.org/www/material/nvscene2008/rwwtt.pdf
float ambientOcclusion(vec3 p, vec3 n, float eps)
{
    float o = 1.0;                  // Start at full output colour intensity
    eps *= aoSpread;                // Spread diffuses the effect
    float k = aoIntensity / eps;    // Set intensity factor
    float d = 2.0 * eps;            // Start ray a little off the surface
    
    for (int i = 0; i < 4; ++i) {
        o -= (d - dE(p + n * d).x) * k;
        d += eps;
        k *= 0.5;                   // AO contribution drops as we move further from the surface 
    }
    
    return clamp(o, 0.0, 1.0);
}

vec3 z_depth = vec3(0.0);

vec3 getSceneColor( in vec3 cameraPosition, in vec3 ray_direction )
{
// Rotates in ZXY order
 x = deg2rad(objRotation.x) * iGlobalTime * 0.05;
 y = deg2rad(objRotation.y);
 z = deg2rad(objRotation.z);
 rx = mat3(1.0, 0.0, 0.0, 0.0, cos(x), sin(x), 0.0, -sin(x), cos(x));
 ry = mat3(cos(y), 0.0, -sin(y), 0.0, 1.0, 0.0, sin(y), 0.0, cos(y));
 rz = mat3(cos(z), sin(z), 0.0, -sin(z), cos(z), 0.0, 0.0, 0.0, 1.0);
 objectRotation = ry * rx * rz;


 aspectRatio = iResolution.x / iResolution.y;
 fovfactor = 1.0 / sqrt(1.0 + cameraFocalLength * cameraFocalLength);
 pixelScale = 1.0 / min(iResolution.x, iResolution.y);
 epsfactor = 1.0 * fovfactor * pixelScale * surfaceDetail;
    

    float ray_length = minRange;
    vec3  ray = cameraPosition + ray_length * ray_direction;
    vec4  color = vec4(0.0);

	vec4 bg_color = vec4(clamp(mix(background2Color, background1Color, (sin(ray_direction.y * HALFPI) + 1.0) * 0.5), 0.0, 1.0), 1.0);
    
    float eps = MIN_EPSILON;
    vec3  dist;
    vec3  normal = vec3(0);
    int   steps = 0;
    bool  hit = false;
    float tmin = 0.0;
    float tmax = 10000.0;
	
    
    if (intersectBoundingSphere(ray, ray_direction, tmin, tmax)) {
        ray_length = tmin;
        ray = cameraPosition + ray_length * ray_direction;
        
        for (int i = 0; i < 1000; i++) {
            steps = i;
            dist = dE(ray);
            dist.x *= surfaceSmoothness;
            
            // If we hit the surface on the previous step check again to make sure it wasn't
            // just a thin filament
            if (hit && dist.x < eps || ray_length > tmax || ray_length < tmin) {
                steps--;
                break;
            }
            
            hit = false;
            ray_length += dist.x;
            ray = cameraPosition + ray_length * ray_direction;
            eps = ray_length * epsfactor;

            if (dist.x < eps || ray_length < tmin) {
                hit = true;
            }
        }
    }
    
    // Found intersection?
    float glowAmount = float(steps)/float(stepLimit);
    float glow;
    
    if (hit) {
        float aof = 1.0, shadows = 1.0;
        glow = clamp(glowAmount * innerGlowIntensity * 3.0, 0.0, 1.0);

        if (steps < 1 || ray_length < tmin) {
            normal = normalize(ray);
        } else {
            normal = generateNormal(ray, eps);
            aof = ambientOcclusion(ray, normal, eps);
        }

	
			// creating the beauty pass 
	        color.rgb = mix(color1, mix(color2, color3, dist.y * color2Intensity), dist.z * color3Intensity);
		
			// add shading
			color.rgb = blinnPhong(clamp(color.rgb * color1Intensity, 0.0, 1.0), ray, normal);
        
			// add inner glow
			color.rgb = mix(color.rgb, innerGlowColor, glow);
		
			// add AO
			color.rgb *= aof;
        
			// add fog
			color.rgb = mix(bg_color.rgb, color.rgb, exp(-pow(ray_length * exp(fogFalloff * .1), 2.0) * fog * .1));	

		
    } else {
        // Apply outer glow and fog
        ray_length = tmax;
		color.rgb = mix(bg_color.rgb, color.rgb, exp(-pow(ray_length * exp(fogFalloff * .1), 2.0)) * fog * .1);
		glow = clamp(glowAmount * outerGlowIntensity * 3.0, 0.0, 1.0);
	    color.rgb = mix(color.rgb, outerGlowColor, glow);

    }
    
    return color.xyz;
}

#ifndef RIFTRAY
// Calculate the output colour for each input pixel
vec4 render(vec2 pixel)
{
    vec3  ray_direction = rayDirection(pixel);
    return vec4(getSceneColor(cameraPosition, ray_direction),1.);
}

// ============================================================================================ //


// The main loop
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    
    vec4 color = vec4(0.0);
		
    float n = 0.0;
    
    cameraRotation = rotationMatrixVector(v, 180.0 - cameraYaw) * rotationMatrixVector(u, -cameraPitch) * rotationMatrixVector(w, cameraRoll);
    
    
	color = render(fragCoord.xy);
	

	fragColor = vec4(color.rgb, 1.0);
	
}
#endif
