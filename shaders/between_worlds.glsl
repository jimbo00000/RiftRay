/*
    Between Worlds
    -------------

    Minor tweak of Shane's "Cave Entrance" to add atmosphere.
    Original: https://www.shadertoy.com/view/ltjXzd

*/

// @var title Between Worlds
// @var author tbabb
// @var url https://www.shadertoy.com/view/4scGWr

// @var tex0 tex08.jpg

#define RMITERATIONS 56
#define PRECISION 0.004
#define FAR 32.
#define PI 3.14159265358979

//#define SHOW_HEATMAP

// Grey scale.
float getGrey(vec3 p){ return p.x*0.299 + p.y*0.587 + p.z*0.114; }

// 2x2 matrix rotation.
mat2 rot2( float a ){ float c = cos(a), s = sin(a);	return mat2( c, -s,	s, c ); }

// Simplified vec3-to-vec3 hash function.
vec3 hash33(vec3 p){
    float n = sin(dot(p, vec3(7, 157, 113)));    
    return fract(vec3(2097152, 262144, 32768)*n); 
}

// Tri-Planar blending function. Based on an old Nvidia writeup:
// GPU Gems 3 - Ryan Geiss: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
vec3 tex3D( sampler2D tex, in vec3 p, in vec3 n ){
  
    n = max((abs(n) - 0.2)*7., 0.001); // n = max(abs(n), 0.001), etc.
    n /= (n.x + n.y + n.z );  
    
    // Note the "1-tex." That's just for this particular example. Normally, the "1" isn't there.
	return 1.-(texture2D(tex, p.yz)*n.x + texture2D(tex, p.zx)*n.y + texture2D(tex, p.xy)*n.z).xyz;
}

// The cobble stone, rocky slate surface... or whatever it's supposed to be. Not a lot of thought 
// was put into this. :)
//
// Basically, mutate a couple of sinusoidal layers using the usual methods - Changes in frequency,
// amplitude, feedback, etc. For the final layer, take a variation on the absolute value. The rest
// was tweaking things until it looked right.
float surfFunc(in vec3 p){

    // Layer 1 - Amplitude, 1.
    vec3 t = sin(p.yzx + cos(p.zxy+1.57/2.));
    float res = dot(t + 1., vec3(0.166));
    p = p*2.5 + (t*0.5 - 0.5)*3.14159265; // Adding "t" gives a bit of curl.

    // Layer 2 - Amplitude, 0.5.
    t = sin(p.yzx + cos(p.zxy+1.57/2.));
    res += dot(t + 1., vec3(0.166))*0.5;
    p = p*3.5 + (t*0.5 - 0.5)*3.14159265;

    // Layer 3 - Amplitude, 0.15.
    t = sin(p + cos(p+1.57/2.));
    res += (1.-abs(dot(t, vec3(0.333))))*0.15; // Take "abs" for rockiness.
	
	// Divide by the total amplitude.  
	return (res/1.65); // Range: [0, 1] 

}


float map(vec3 p){
 
     return 1.-abs(p.y) - (0.5-surfFunc(p))*1.5;
 
}

// Texture bump mapping. Four tri-planar lookups, or 12 texture lookups in total.
vec3 doBumpMap( sampler2D tex, in vec3 p, in vec3 nor, float bumpfactor){
   
    const float eps = 0.001;
    vec3 grad = vec3( getGrey(tex3D(tex, vec3(p.x-eps, p.y, p.z), nor)),
                      getGrey(tex3D(tex, vec3(p.x, p.y-eps, p.z), nor)),
                      getGrey(tex3D(tex, vec3(p.x, p.y, p.z-eps), nor)));
    
    grad = (grad - getGrey(tex3D(tex,  p , nor)))/eps; 
            
    grad -= nor*dot(nor, grad);          
                      
    return normalize( nor + grad*bumpfactor );
	
}

// Tetrahedral normal: I remember a similar version on "Pouet.net" years ago, but this one is courtesy of IQ.
vec3 getNormal( in vec3 p ){

    vec2 e = vec2(0.5773,-0.5773)*0.001;
    return normalize( e.xyy*map(p+e.xyy ) + e.yyx*map(p+e.yyx ) + e.yxy*map(p+e.yxy ) + e.xxx*map(p+e.xxx ));
}

// Based on original by IQ.
float calculateAO(vec3 p, vec3 n){

    const float AO_SAMPLES = 5.0;
    float r = 0.0, w = 1.0, d;
    
    for (float i=1.0; i<AO_SAMPLES+1.1; i++){
        d = i/AO_SAMPLES;
        r += w*(d - map(p + n*d));
        w *= 0.5;
    }
    
    return 1.0-clamp(r,0.0,1.0);
}

// Cool curve function, by Shadertoy user, Nimitz.
//
// I wonder if any of it relates back to the  discrete finite difference approximation to 
// the continuous Laplace differential operator? Either way, it gives you a scalar curvature
// value for an object's signed distance function, which is pretty handy. I used it to do a 
// bit of fake, shadowy occlusion.
//
// From an intuitive sense, the function returns a weighted difference between a surface 
// value and some surrounding values - arranged in a simplex tetrahedral fashion for minimal
// calculations, I'm assuming. Almost common sense... almost. :)
//
// Original usage (I think?) - Cheap curvature: https://www.shadertoy.com/view/Xts3WM
// Other usage: Xyptonjtroz: https://www.shadertoy.com/view/4ts3z2
float curve(in vec3 p, in float w){

    vec2 e = vec2(-1., 1.)*w;
    
    float t1 = map(p + e.yxx), t2 = map(p + e.xxy);
    float t3 = map(p + e.xyx), t4 = map(p + e.yyy);
    
    return 0.125/(w*w) *(t1 + t2 + t3 + t4 - 4.*map(p));
}

vec3 getSceneColor( in vec3 camPos, in vec3 rd )
{
    
    // Light position. Set at a reasonable distance to the right and in front or the camera.
    // Keeping it at a constant distance from the camera gives a slight distant-sun impression.
    // By keeping it within a measurable distance, you can still receive a bit of a point light 
    // effect, which tends to look a little nicer when bumped... because of the changing angles, 
    // I guess. All of it is fake, of course, so feel free to swap in a directional light setup.
 	vec3 lp = camPos + vec3(16.0, 0., 8.);// Put it a bit in front of the camera.

    

	// I always groan when I see convoluted lines like this. :) It's just a way to flip the camera 180 degrees about the 
	// XY plane every now and again. The end term is a bit of camera tilt. There are probably other ways to get this done.
//	rd.xy *= rot2( smoothstep(0.3, 0.7, sin(iGlobalTime*0.1875 - 2.)*0.5+0.5)*3.14159 + sin(iGlobalTime*0.1875 -2.)*0.1 );

		
    // Standard ray marching routine, with a couple of tiny tweaks. Note that PRECISION is multiplied by "t." You'll see it 
    // around here and there. The idea is that you don't need as much precision as you get further away. Makes sense. Having 
    // said that, I'm always amazed at how well it can work in certain situations. The "step" business just varies the 
    // amount you jump by, depending on whether "t" is greater than or equal to 1. It's a weird tweak, but can be effective
    // at times. "Aiekick" uses a variation on occasion.
    //
    // By the way, if you're interested in optimizing raymarching routines, there's a lot of examples on this site. Shadertoy
    // user "huwb" does some interesting stuff. His "Raymarching as root finding," et al, is pretty cool. Nimitz's
    // "Log-Bisection Tracing" example is also worth a look.
	float t = 0.0, dt = 0.0, itTotal = 1.;
	
	for(int i=0; i<RMITERATIONS; i++){
		dt = map(camPos + rd*t);
		if(abs(dt)<PRECISION*t || t>FAR){ break; } // Break statement, and nothing else.
		t += dt*(0.75 + step(t, 1.)*0.25);
		#ifdef SHOW_HEATMAP
		itTotal++; // Total iterations for the heat map, if used.
		#endif
	}

	// Initiate the scene color to black.
	vec3 sceneCol = vec3(0.);
	
	// The ray has effectively hit the surface, so light it up.
	if(t<FAR){
	
	    // Fudge to gain a little more accuracy. Not really necessary, and according to user BigWIngs,
	    // could cause inaccuracies at grazing angles, so comment it out if that's a concern. I like 
	    // to live on the edge, so I'm leaving it in. :) BigWIngs's "Dying Universe shader is pretty
	    // cool, by the way.
	    t += dt;
    	
    	// Surface position and surface normal.
	    vec3 sp = t * rd+camPos;
	    vec3 sn = getNormal(sp);
        
        // Texture scale factor.
        const float tSize0 = 1./4.;

	    sn = doBumpMap(iChannel0, sp*tSize0, sn, 0.01);

	    // Ambient occlusion.
	    float ao = calculateAO(sp, sn);
    	
    	// Curvature - Nimitz.
    	// Cool for all sorts of things, but used for shading here.
	    float crv = clamp(curve(sp, 0.125)*0.5+0.5, .0, 1.);
	    
    	// Light direction vector.
	    vec3 ld = lp-sp;

        // Distance from the lights to the surface point.
	    float distlpsp = max(length(ld), 0.001);
    	
    	// Normalize the light direction vector.
	    ld /= distlpsp;
	    
	    // Light attenuation, based on the distance above. I've scaled down the distance before
        // attenuating just to make the figures easier to work with.
	    distlpsp /= 18.;
	    float atten = min(1./(1. + distlpsp*distlpsp*0.025), 1.);
  	
    	// Ambient light.
	    float ambience = 0.55;
    	
    	// Diffuse lighting.
	    float diff = max( dot(sn, ld), 0.0);
  	
    	// Specular lighting.
	    float spec = pow(max( dot( reflect(-ld, sn), -rd ), 0.0 ), 8.);

        
        // Obtaining the texel color. 
	    vec3 texCol = tex3D(iChannel0, sp*tSize0, sn);//*0.66+tex3D(texChannel0, sp.yzx*tSize0*2., sn)*0.34;

    	
    	// Darkening the crevices. Otherwise known as cheap, scientifically-incorrect shadowing.	
	    float shading =  crv*0.5+0.5; //1.-surfFunc(sp)*0.5;////
	
    	
    	// Coloring and lighting the surface color.
        sceneCol = texCol*(diff*vec3(1.0,0.56, 0.18) + ambience) + vec3(1.0, 0.5, 0.2)*spec*2.;
        
        // Green moss. Off the top of my head, so not paricularly well thought out. :)
        //
	    // Fresnel term. Good for giving a surface a bit of a reflective glow.
        //float fre = pow( clamp(dot(sn, rd) + 1., .0, 1.), 1.); 
        //
        //sceneCol = (1.-texCol)*(diff*0.25 + ambience*0.1) + fre*texCol.yzx*texCol.yzx + vec3(1.,0.5, 0.2)*spec*1.5;
     
        // Shading.
        sceneCol *= shading*ao*atten;
     
	
	}
	
	sceneCol = clamp(sceneCol, 0., 1.);

    float fogAtten = 1./(1.+t*t*0.025);
    vec3    fogCol = mix(vec3(0.77, 0.89, 1.), 
                     1.1*vec3(1., 0.89, 0.77), 
                         dot(rd, vec3(1.,1.,1.)));
    sceneCol = mix(fogCol, sceneCol, fogAtten);
	
    // Alternative falloff.
    //float fogAtten = smoothstep(0., FAR/2., t);
	//sceneCol = mix(sceneCol, vec3(0), fogAtten);
	
	#ifdef SHOW_HEATMAP
	float heat = min(itTotal/float(RMITERATIONS), 1.);	
	const float hLayers = 8.;
	heat = floor(heat*(hLayers-0.001))/(hLayers-1.);
	sceneCol = vec3(heat);//vec3(min(heat*1.5, 1.), pow(heat, 2.5), pow(heat, 10.));
	#endif
	
	return sceneCol;
}

#ifndef RIFTRAY
void mainImage( out vec4 fragColor, in vec2 fragCoord ){
	
	// Screen coordinates.
	vec2 uv = (fragCoord - iResolution.xy*0.5)/iResolution.y;
	
	// Camera Setup.
	vec3 lookAt = vec3(iGlobalTime*4.+1., 0.0, iGlobalTime*2.+ 1.);  // "Look At" position.
	vec3 camPos = lookAt + vec3(.0, 0.1, -1.0); // Camera position, doubling as the ray origin.
 
    // Using the above to produce the unit ray-direction vector.
    float FOV = PI/2.; // FOV - Field of view.
    vec3 forward = normalize(lookAt-camPos);
    vec3 right = normalize(vec3(forward.z, 0., -forward.x )); 

    // rd - Ray direction.
    vec3 rd = normalize(forward + FOV*uv.x*right + FOV*uv.y*cross(forward, right)); // Cross term is the "up" vector.
    
    vec3 sceneCol = getSceneColor( camPos, rd );
	
	fragColor = vec4(sceneCol, 1.0);
	
}
#endif
