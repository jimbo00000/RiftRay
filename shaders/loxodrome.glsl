// Loxodrome

// @var title Loxodrome
// @var author fb39ca4
// @var license CC BY-NC-SA 3.0
// @var url https://www.shadertoy.com/view/MsX3D2

// @var headSize 5.0
// @var eyePos 0.0 0.0 -6.4
// @var tex0 tex05.jpg

//Thank you iquilez for some of the primitive distance functions!

const float PI = 3.14159265358979323846264;

const int MAX_PRIMARY_RAY_STEPS = 48; //decrease this number if it runs slow on your computer
const int MAX_SECONDARY_RAY_STEPS = 24;

//Uncomment to see the full loxodrome shadow
#define LOXODROME_ONLY

//mix takes a number between 0 and 1 and maps it to a number between a and b.
//inverseMix takes a number between a and b and maps it to a number between 0 and 1.
float inverseMix(float a, float b, float x) {
	return (x - a) / (b - a);	
}

vec2 rotate2d(vec2 v, float a) { 
	return vec2(v.x * cos(a) - v.y * sin(a), v.y * cos(a) + v.x * sin(a)); 
}

float polarDist(vec2 v1, vec2 v2) { 
	//Formula ripped from 
	//http://math.ucsd.edu/~wgarner/math4c/derivations/distance/distancepolar.htm
	return sqrt(v1.x * v1.x + v2.x * v2.x - 2.0 * v1.x * v2.x * cos(v1.y - v2.y));
}

vec3 rectToSpher(vec3 v) {
	const float pi = 3.14159265359;
	vec3 res;
	res.x = length(v);
	res.y = atan(v.z, v.x); //longditude
	res.y += 2.0 * pi * step(0.0, res.y);
	res.z = atan(v.y, length(v.xz)); //latitude
	return res;
}

float secIntegral(float x) {
	return log(1.0 / cos(x) + tan(x));
}

float invSecIntegral(float x) {
	return acos(2.0 / (exp(-x) + exp(x))) * sign(x);
}

float floorCustom(float x, float c) {
	return floor(x / c) * c;	
}

float ceilCustom(float x, float c) {
	return ceil(x / c) * c;	
}

float sdLoxodrome(vec3 p, float twist, float rotSymm, float thickness) {
	const float pi = 3.14159265359;
	vec3 s = rectToSpher(p);
	s.y += iGlobalTime;
	float offset = thickness * cos(s.z);
	vec2 s1 = vec2(1.0 - offset, invSecIntegral((ceilCustom(secIntegral(s.z) * twist - s.y, pi * 2.0 / rotSymm) + s.y) / twist));
	vec2 s2 = vec2(1.0 - offset, invSecIntegral((floorCustom(secIntegral(s.z) * twist - s.y, pi * 2.0 / rotSymm) + s.y) / twist));
	float res = min(polarDist(s.xz, s1), polarDist(s.xz, s2));
	res -= offset;
	return res * 1.0;
}

float sdCylinder( vec3 p, vec2 c ) {
    return max(length(p.xy) - c.x, abs(p.z) - c.y);
}

float distanceField(vec3 p) {
	float wall = -p.z + 1.5;
	float loxodrome = sdLoxodrome(p - vec3(0.0, 0.0, 0.0), -2.0, 4.0, 0.075);
	float holder = 9001.0;
	#ifndef LOXODROME_ONLY
	holder = sdCylinder(p.xzy - vec3(0.0, 0.0, -1.125), vec2(0.5, 0.25));
	holder = min(holder, sdCylinder(p.xzy - vec3(0.0, 0.0, -1.625), vec2(0.125, 0.75)));
	holder = min(holder, sdCylinder(p.xzy - vec3(0.0, 0.0, -2.125), vec2(0.25, 0.25)));
	holder = min(holder, sdCylinder(p.xyz - vec3(0.0, -2.125, 0.75), vec2(0.125, 0.75)));
	holder = min(holder, sdCylinder(p.xyz - vec3(0.0, -2.125, 1.5), vec2(0.5, 0.125)));
	#endif
	return min(wall, min(holder, loxodrome));
}

float shadowDistanceField(vec3 p) {
	float loxodrome = sdLoxodrome(p - vec3(0.0, 0.0, 0.0), -2.0, 4.0, 0.075);
	float holder = 9001.0;
	#ifndef LOXODROME_ONLY
	holder = sdCylinder(p.xzy - vec3(0.0, 0.0, -1.125), vec2(0.5, 0.25));
	#endif
	return min(holder, loxodrome);
}

float getMaterial(vec3 p) {
	float wall = -p.z + 1.5;
	float loxodrome = sdLoxodrome(p - vec3(0.0, 0.0, 0.0), -2.0, 4.0, 0.075);
	float holder = 9001.0;
	#ifndef LOXODROME_ONLY
	holder = sdCylinder(p.xzy - vec3(0.0, 0.0, -1.125), vec2(0.5, 0.25));
	holder = min(holder, sdCylinder(p.xzy - vec3(0.0, 0.0, -1.625), vec2(0.125, 0.75)));
	holder = min(holder, sdCylinder(p.xzy - vec3(0.0, 0.0, -2.125), vec2(0.25, 0.25)));
	holder = min(holder, sdCylinder(p.xyz - vec3(0.0, -2.125, 0.75), vec2(0.125, 0.75)));
	holder = min(holder, sdCylinder(p.xyz - vec3(0.0, -2.125, 1.5), vec2(0.5, 0.125)));
	#endif
	return step(min(holder, loxodrome), wall);
}

vec3 calcNormal(vec3 pos) {
	const float derivDist = 0.0001;
	vec3 surfaceNormal;
	surfaceNormal.x = distanceField(vec3(pos.x + derivDist, pos.y, pos.z)) 
					- distanceField(vec3(pos.x - derivDist, pos.y, pos.z));
	surfaceNormal.y = distanceField(vec3(pos.x, pos.y + derivDist, pos.z)) 
					- distanceField(vec3(pos.x, pos.y - derivDist, pos.z));
	surfaceNormal.z = distanceField(vec3(pos.x, pos.y, pos.z + derivDist)) 
					- distanceField(vec3(pos.x, pos.y, pos.z - derivDist));
	return normalize(surfaceNormal / derivDist);
}

vec3 castRay(vec3 pos, vec3 dir, float treshold) {
	for (int i = 0; i < MAX_PRIMARY_RAY_STEPS; i++) {
			float dist = distanceField(pos);
			//if (abs(dist) < treshold) break;
			pos += dist * dir;
	}
	return pos;
}

float castSoftShadowRay(vec3 pos, vec3 lightPos) {
	const float pi = 3.14159265359;
	const float k = 0.005;
	float res = 1.0;
	vec3 rayDir = normalize(lightPos - pos);
	float maxDist = length(lightPos - pos);
	
	vec3 rayPos = pos + 0.01 * rayDir;
	float distAccum = 0.1;
	
	for (int i = 1; i <= MAX_SECONDARY_RAY_STEPS; i++) {
		rayPos = pos + rayDir * distAccum;
		float dist = shadowDistanceField(rayPos);
		float penumbraDist = distAccum * k;
		res = min(res, inverseMix(-penumbraDist, penumbraDist, dist));
		distAccum += (dist + penumbraDist) * 0.5;
		distAccum = min(distAccum, maxDist);
	}
	res = max(res, 0.0);
	res = res * 2.0 - 1.0;
	return (0.5 * (sqrt(1.0 - res * res) * res + asin(res)) + (pi / 4.0)) / (pi / 2.0);
}

float lightPointDiffuseSoftShadow(vec3 pos, vec3 lightPos, vec3 normal) {
	vec3 lightDir = normalize(lightPos - pos);
	float lightDist = length(lightPos - pos);
	float color = max(dot(normal, lightDir), 0.0) / (lightDist * lightDist);
	if (color > 0.00) color *= castSoftShadowRay(pos, lightPos);
	return max(0.0, color);
}

vec3 getSceneColor( in vec3 cameraPos, in vec3 rayDir )
{
	vec3 rayPos = castRay(cameraPos, rayDir, 0.01);
	vec3 normal = calcNormal(rayPos);
	
	float material = getMaterial(rayPos);
	
	vec3 color;
	if (material == 0.0) color = pow(texture2D(iChannel0, rayPos.yx * 0.125).rgb, vec3(2.2));
	if (material == 1.0) color = vec3(0.05);
	
	color *= 32.0 * lightPointDiffuseSoftShadow(rayPos, vec3(0.0, 0.0, -0.6), normal) + 0.05 * smoothstep(10.0, 0.0, length(rayPos));
	
	color = pow(color, vec3(1.0 / 2.2));	
	return color;
}

#ifndef RIFTRAY
void main(void)
{
	vec4 mousePos = (iMouse / iResolution.xyxy) * 2.0 - 1.0;
	mousePos *= vec2(PI / 2.0, PI / 2.0).xyxy;
	if (iMouse.zw == vec2(0.0)) mousePos.xy = vec2(0.5, -0.2);
	
	vec2 screenPos = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;
	
	vec3 cameraPos = vec3(0.0, 0.0, -8.0);
	//vec3 cameraPos = vec3(0.0);
	
	vec3 cameraDir = vec3(0.0, 0.0, 1.0);
	vec3 planeU = vec3(1.0, 0.0, 0.0);
	vec3 planeV = vec3(0.0, iResolution.y / iResolution.x * 1.0, 0.0);
	vec3 rayDir = normalize(cameraDir + screenPos.x * planeU + screenPos.y * planeV);
	
	cameraPos.yz = rotate2d(cameraPos.yz, mousePos.y);
	rayDir.yz = rotate2d(rayDir.yz, mousePos.y);
	
	cameraPos.xz = rotate2d(cameraPos.xz, mousePos.x);
	rayDir.xz = rotate2d(rayDir.xz, mousePos.x);
	
	vec3 color = getSceneColor( cameraPos, rayDir );
	gl_FragColor = vec4(color, 1.0);
}
#endif
