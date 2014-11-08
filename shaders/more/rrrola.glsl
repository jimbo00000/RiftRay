// rrrola, ported over from https://code.google.com/p/boxplorer2/
// Mandelbox shader by Rrrola
// Original formula by Tglad
// - http://www.fractalforums.com/3d-fractal-generation/amazing-fractal

// @var title Mandelbox
// @var author Rrrola
// @var url https://code.google.com/p/boxplorer2/

// @var headSize 0.19
// @var eyePos 1.613 1.615 -2.150

mat3  rotationMatrix3(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat3(
		c +
		(1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
		(1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
		(1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
		);
}


float hash( float n ) {
    return fract(sin(n)*5345.8621276);
}


float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*61.0;

    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);

    return fract(res);
}


const int iters = 14;
mat3 rotationMatrix = mat3(1,0,0, 0,1,0, 0,0,1);
const float minRad2 = 0.1;
const float MB_SCALE = -1.77;

const float absScalePowIters = 0.001;
const float DIST_MULTIPLIER = 1.0;

vec4 scale = vec4(MB_SCALE, MB_SCALE, MB_SCALE, abs(MB_SCALE)) / minRad2;


float de_mandelbox(vec3 pos) {
  vec4 p = vec4(pos,1.0), p0 = p;  // p.w is the distance estimate
  for (int i=0; i<iters; i++) {
    p = vec4(rotationMatrix * p.xyz, p.w);
    p = vec4(clamp(p.xyz, -1.0, 1.0) * 2.0 - p.xyz, p.w);
    float r2 = dot(p.xyz, p.xyz);
    p *= clamp(max(minRad2/r2, minRad2), 0.0, 1.0);
    p = p*scale + p0;
// if (r2 > 100.0) break;
  }
  return ((length(p.xyz) - abs(MB_SCALE - 1.0)) / p.w
            - absScalePowIters) * 0.95 * DIST_MULTIPLIER;
}



const float MB_MINRAD2 = 0.25;
const int color_iters = 10;

// Colors. Can be negative or >1 for interestiong effects.
vec3 backgroundColor = vec3(0.07, 0.06, 0.16),
  surfaceColor1 = vec3(0.95, 0.64, 0.1),
  surfaceColor2 = vec3(0.89, 0.95, 0.75),
  surfaceColor3 = vec3(0.55, 0.06, 0.03),
  specularColor = vec3(1.0, 0.8, 0.4),
  glowColor = vec3(0.03, 0.4, 0.4),
  aoColor = vec3(0, 0, 0);

// Compute the color at `pos`.
vec3 c_mandelbox(vec3 pos) {
  float minRad2 = clamp(MB_MINRAD2, 1.0e-9, 1.0);
  vec3 scale = vec3(MB_SCALE, MB_SCALE, MB_SCALE) / minRad2;
  vec3 p = pos, p0 = p;
  float trap = 1.0;
  for (int i=0; i<color_iters; i++) {
    p = vec3(rotationMatrix * p.xyz);
    p = clamp(p, -1.0, 1.0) * 2.0 - p;
    float r2 = dot(p, p);
    p *= clamp(max(minRad2/r2, minRad2), 0.0, 1.0);
    p = p*scale + p0;
    trap = min(trap, r2);
  }
  // c.x: log final distance (fractional iteration count)
  // c.y: spherical orbit trap at (0,0,0)
  vec2 c = clamp(vec2( 0.33*log(dot(p,p))-1.0, sqrt(trap) ), 0.0, 1.0);
  return mix(mix(surfaceColor1, surfaceColor2, c.y), surfaceColor3, c.x);
}


// distance estimator func
#ifndef d
#define d de_mandelbox // PKlein,combi,menger,mandelbox,ssponge
#endif

// surface coloring func
#ifndef c
#define c c_mandelbox  // PKlein,menger
#endif

float normal_eps = 0.000001;
// Compute the normal at `pos`.
// `d_pos` is the previously computed distance at `pos` (for forward differences).
vec3 normal(vec3 pos, float d_pos) {
  vec2 Eps = vec2(0, max(normal_eps, d_pos));
  return normalize(vec3(
    -d(pos-Eps.yxx)+d(pos+Eps.yxx),
    -d(pos-Eps.xyx)+d(pos+Eps.xyx),
    -d(pos-Eps.xxy)+d(pos+Eps.xxy)
  ));
}



// Blinn-Phong shading model with rim lighting (diffuse light bleeding to the other side).
// `normal`, `view` and `light` should be normalized.
vec3 blinn_phong(vec3 normal, vec3 view, vec3 light, vec3 diffuseColor) {
  vec3 halfLV = normalize(light + view);
  float spe = pow(max( dot(normal, halfLV), 0.0 ), 32.0);
  float dif = dot(normal, light) * 0.5 + 0.75;
  return dif*diffuseColor + spe*specularColor;
}


const float ao_eps = 0.0005;
const float ao_strength = 0.1;

// FAKE Ambient occlusion approximation.
// uses current distance estimate as first dist. the size of AO is independent from distance from eye
float ambient_occlusion(vec3 p, vec3 n, float DistAtp, float side, float m_dist) {
  float ao_ed=ao_eps;
  float ao = 1.0, w = ao_strength/ao_ed;
  float dist = 2.0 * ao_ed;

  for (int i=0; i<5; i++) {
    float D = side * d(p + n*dist);
    ao -= (dist-abs(D)) * w;
    w *= 0.5;
    dist = dist*2.0 - ao_ed;  // 2,3,5,9,17
  }
  return clamp(ao, 0.0, 1.0);
}


const int max_steps = 128;
const float MAX_DIST = 5.0;
const float dist_to_color = 0.2;
vec3 getSceneColor( in vec3 eye_in, in vec3 dp )
{

  vec3 p = eye_in;
  float D = d(p);
  float side = sign(D);
  float totalD = side * D;   // Randomize first step.

	
	float m_zoom = 0.0001;
	
	
  // Intersect the view ray with the Mandelbox using raymarching.
  float m_dist = m_zoom * totalD;
	
  for (int steps=0; steps<max_steps; steps++) {
    D = (side * d(p + totalD * dp));
    if (D < m_dist) break;
    totalD += D;
    if (totalD > MAX_DIST) break;
    m_dist =  m_zoom * totalD;
  }

	
  // If we got a hit, find desired distance to surface.
  // Likely our hit was lot closer than m_dist; make it approx. m_dist.
  if (D < m_dist) {
    for (int i = 0; i < 5; ++i) {
      totalD += D - m_dist;
      m_dist =  m_zoom * totalD;
      D = d(p + totalD * dp);
    }
  }

	
  p += totalD * dp;

  // Color the surface with Blinn-Phong shading, ambient occlusion and glow.
  vec3 col = backgroundColor;

  // We've got a hit or we're not sure.
  if (totalD < MAX_DIST) {
    vec3 n = normal(p, D/*m_dist*/);
    col = c(p);
    col = blinn_phong(n, -dp, normalize(eye_in+vec3(0,1,0)+dp), col);
    col = mix(aoColor, col, ambient_occlusion(p, n, abs(D), side, m_dist));

    // We've gone through all steps, but we haven't hit anything.
    // Mix in the background color.
    if (D > m_dist) {
      col = mix(col, backgroundColor, clamp(log(D/m_dist) * dist_to_color, 0.0, 1.0));
    }
  }

  // Glow is based on the number of steps.
  //col = mix(col, glowColor, (float(steps)+noise)/float(max_steps) * glow_strength);

  return col;
}
	

#if 0
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy*2.-1.;
	uv.y*=iResolution.y/iResolution.x;

	vec3 from = vec3(0.0, 0.0, -4. );
	vec3 dir  = normalize(vec3(uv*.85,1.));
	
	// Mouse interaction for view direction
	vec2 mouse=(iMouse.xy/iResolution.xy-.5);
	// Use a rotation here so we can look around backwards
	dir = dir * rotationMatrix3(vec3(1.0, 0.0, 0.0), mouse.y*-300.0);
	dir = dir * rotationMatrix3(vec3(0.0, 1.0, 0.0), mouse.x*300.0);
	
	
	vec3 color = getSceneColor( from, dir );
	
	
	gl_FragColor = vec4(color,1.);
}
#endif
