// Simple ray marching example
// @var url https://www.shadertoy.com/view/ldB3Rw
// @var author gltracy
// @var license CC BY-NC-SA 3.0

// @var headSize 6.0
// @var eyePos -2.5952096 5.4259381 -20.277588

const int max_iterations = 255;
const float stop_threshold = 0.001;
const float grad_step = 0.1;
const float clip_far = 1000.0;

// math
const float PI = 3.14159265359;
const float DEG_TO_RAD = PI / 180.0;

// distance function
float dist_sphere( vec3 pos, float r ) {
	return length( pos ) - r;
}

float dist_box( vec3 pos, vec3 size ) {
	return length( max( abs( pos ) - size, 0.0 ) );
}

float dist_box( vec3 v, vec3 size, float r ) {
	return length( max( abs( v ) - size, 0.0 ) ) - r;
}

// get distance in the world
float dist_field( vec3 pos ) {
	// ...add objects here...

	// box
	float d2 = dist_box( pos + vec3( 0.0, 4.5, 0.0 ), vec3( 8.0, 0.05, 8.0 ), 0.05 );

#ifdef RIFTRAY
	pos = vec4( obmtx * vec4(pos,1.0) ).xyz;
#endif

	// object 0 : sphere
	float d0 = dist_sphere( pos, 2.7 );
	
	// object 1 : cube
	float d1 = dist_box( pos, vec3( 2.0 ) );

	// union     : min( d0,  d1 )
	// intersect : max( d0,  d1 )
	// subtract  : max( d1, -d0 )
	//return max( d1, -d0 );
	return min( d2, max( d1, -d0 ) );
}

// phong shading
vec3 shading( vec3 v, vec3 n, vec3 eye ) {
	// ...add lights here...
	
	float shininess = 16.0;
	
	vec3 final = vec3( 0.0 );
	
	vec3 ev = normalize( v - eye );
	vec3 ref_ev = reflect( ev, n );
	
	// light 0
	{
		vec3 light_pos   = vec3( 20.0, 20.0, 20.0 );
		vec3 light_color = vec3( 1.0, 0.7, 0.7 );
	
		vec3 vl = normalize( light_pos - v );
	
		float diffuse  = max( 0.0, dot( vl, n ) );
		float specular = max( 0.0, dot( vl, ref_ev ) );
		specular = pow( specular, shininess );
		
		final += light_color * ( diffuse + specular ); 
	}
	
	// light 1
	{
		vec3 light_pos   = vec3( -20.0, -20.0, -20.0 );
		vec3 light_color = vec3( 0.3, 0.7, 1.0 );
	
		vec3 vl = normalize( light_pos - v );
	
		float diffuse  = max( 0.0, dot( vl, n ) );
		float specular = max( 0.0, dot( vl, ref_ev ) );
		specular = pow( specular, shininess );
		
		final += light_color * ( diffuse + specular ); 
	}

	return final;
}

// get gradient in the world
vec3 gradient( vec3 pos ) {
	const vec3 dx = vec3( grad_step, 0.0, 0.0 );
	const vec3 dy = vec3( 0.0, grad_step, 0.0 );
	const vec3 dz = vec3( 0.0, 0.0, grad_step );
	return normalize (
		vec3(
			dist_field( pos + dx ) - dist_field( pos - dx ),
			dist_field( pos + dy ) - dist_field( pos - dy ),
			dist_field( pos + dz ) - dist_field( pos - dz )	
		)
	);
}

// ray marching
float ray_marching( vec3 origin, vec3 dir, float start, float end ) {
	float depth = start;
	for ( int i = 0; i < max_iterations; i++ ) {
		float dist = dist_field( origin + dir * depth );
		if ( dist < stop_threshold ) {
			return depth;
		}
		depth += dist;
		if ( depth >= end) {
			return end;
		}
	}
	return end;
}

// get ray direction
vec3 ray_dir( float fov, vec2 size, vec2 pos ) {
	vec2 xy = pos - size * 0.5;

	float cot_half_fov = tan( ( 90.0 - fov * 0.5 ) * DEG_TO_RAD );	
	float z = size.y * 0.5 * cot_half_fov;
	
	return normalize( vec3( xy, -z ) );
}

// camera rotation : pitch, yaw
mat3 rotationXY( vec2 angle ) {
	vec2 c = cos( angle );
	vec2 s = sin( angle );
	
	return mat3(
		c.y      ,  0.0, -s.y,
		s.y * s.x,  c.x,  c.y * s.x,
		s.y * c.x, -s.x,  c.y * c.x
	);
}


///////////////////////////////////////////////////////////////////////////////
// Agreeing on certian entry points, we can patch in the Rift's heading:
//
// vec3 getEyePoint( mat4 mvmtx );
// vec3 getEyeDirection( vec2 uv, mat4 mvmtx );
// vec2 getSamplerUV( vec2 fragCoord );
// vec3 getSceneColor( in vec3 ro, in vec3 rd );
//
///////////////////////////////////////////////////////////////////////////////
vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
	// ray marching
	float depth = ray_marching( ro, rd, 0.0, clip_far );
	if ( depth >= clip_far ) {
		discard;
	}
	
	// shading
	vec3 pos = ro + rd * depth;
	vec3 n = gradient( pos );
	return shading( pos, n, ro );
}

#ifndef RIFTRAY
void main(void)
{
	// default ray dir
	vec3 dir = ray_dir( 45.0, iResolution.xy, gl_FragCoord.xy );
	
	// default ray origin
	vec3 eye = vec3( 0.0, 0.0, 10.0 );

	// rotate camera
	mat3 rot = rotationXY( vec2( 1.0, 1.0 )*vec2( 0.01 * iMouse.yx ) );
	dir = rot * dir;
	eye = rot * eye;
	
	gl_FragColor = vec4( getSceneColor( eye, dir ), 1.0 );
}
#endif
