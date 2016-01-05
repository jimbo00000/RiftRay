/*
	
    Transparent 3D Noise
	--------------------

	On occasion, I use this simple transparent setup to visualize certain 3D algorithms.
	Obviously, it wouldn't pass for proper transparency, but to be fair, it's not running 
	at minus 2 frames per second either. :)

	Here, it's being used to render stock standard, smooth, 3D value noise.

	I'm using a variation on IQ's textureless version. It does the same thing, but is 
	written in a more succinct, self contained form. I couldn't say whether it's faster 
	or not (probably not), but it's faster to cut and paste, so that's almost the same 
	thing. :)

	Seriously, though, it's reasonably quick, but I'm hoping some clever soul might know 
	of a way to improve it.

	IQ's really fast, textureless version is there as well, but it's commented out. If 
	you require more speed, that's the one to use.

	In regards to the transparency itself, there are a bunch of variables that effect the
	overall look. Too many to mention, in fact. The easiest way to get accustomed to
	them all is to change them and see what happens.

	Related shaders:

	Cloudy Spikeball - Duke
    https://www.shadertoy.com/view/MljXDw
    // Port from a demo by Las - Worth watching.
    // http://www.pouet.net/topic.php?which=7920&page=29&x=14&y=9

	Virtually the same thing, but with rounded cubes.
	Transparent Cube Field - Shane
	https://www.shadertoy.com/view/ll2SRy
	
*/

// @var title Transparent 3D Noise
// @var author Shane
// @var url https://www.shadertoy.com/view/lstGRB

// Cheap vec3 to vec3 hash. Works well enough, but there are other ways.
vec3 hash33(vec3 p){ 
    
    float n = sin(dot(p, vec3(7, 157, 113)));    
    return fract(vec3(2097152, 262144, 32768)*n); 
}

/*
// IQ's texture lookup noise... in obfuscated form. There's less writing, so
// that makes it faster. That's how optimization works, right? :) Seriously,
// though, refer to IQ's original for the proper function.
// 
// By the way, I have tried to better this formula with a textureless version, 
// but so far, nothing comes close. If you're after speed, this is the one
// to use... or improve on, if you can. :)
//
// If not loaded, you need to load up the correct noise texture with this one. 
// It's important that the "vFlip" checkbox is unchecked, also.
float noise3DTex( in vec3 p ){
    
    vec3 i = floor(p); p -= i; p *= p*(3. - 2.*p);
	p.xy = texture2D(iChannel0, (p.xy + i.xy + vec2(37, 17)*i.z + .5)/256., -100.).yx;
	return mix(p.x, p.y, p.z);
}
*/

// This is a rewrite of IQ's original. It's self contained, which makes it much
// easier to copy and paste. I've also tried my best to minimize the amount of 
// operations to lessen the work the GPU has to do, but I think there's room for
// improvement. I have no idea whether it's faster or not. It could be slower,
// for all I know, but it doesn't really matter, because in its current state, 
// it's still no match for IQ's texture-based, smooth 3D value noise.
//
// By the way, a few people have managed to reduce the original down to this state, 
// but I haven't come across any who have taken it further. If you know of any, I'd
// love to hear about it.
//
// I've tried to come up with some clever way to improve the randomization line
// (h = mix(fract...), but so far, nothing's come to mind.
float noise3D(vec3 p){
    
    // Just some random figures, analogous to stride. You can change this, if you want.
	const vec3 s = vec3(7, 157, 113);
	
	vec3 ip = floor(p); // Unique unit cell ID.
    
    // Setting up the stride vector for randomization and interpolation, kind of. 
    // All kinds of shortcuts are taken here. Refer to IQ's original formula.
    vec4 h = vec4(0., s.yz, s.y + s.z) + dot(ip, s);
    
	p -= ip; // Cell's fractional component.
	
    // A bit of cubic smoothing, to give the noise that rounded look.
    p = p*p*(3. - 2.*p);
    
    // Smoother version of the above. Weirdly, the extra calculations can sometimes
    // create a surface that's easier to hone in on, and can actually speed things up.
    // Having said that, I'm sticking with the simpler version above.
	//p = p*p*p*(p*(p * 6. - 15.) + 10.);
    
    // Even smoother, but this would have to be slower, surely?
	//vec3 p3 = p*p*p; p = ( 7. + ( p3 - 7. ) * p ) * p3;	
	
    // Cosinusoidal smoothing. OK, but I prefer other methods.
    //p = .5 - .5*cos(p*3.14159);
    
    // Standard 3D noise stuff. Retrieving 8 random scalar values for each cube corner,
    // then interpolating along X. There are countless ways to randomize, but this is
    // the way most are familar with: fract(sin(x)*largeNumber).
    h = mix(fract(sin(h)*43758.5453), fract(sin(h + s.x)*43758.5453), p.x);
	
    // Interpolating along Y.
    h.xy = mix(h.xz, h.yw, p.y);
    
    // Interpolating along Z, and returning the 3D noise value.
    return mix(h.x, h.y, p.z); // Range: [0, 1].
	
}

float map(vec3 p){
    
    // One layer of noise at isolevel "0.3" - I have no idea whether that's the
    // correct terminology. :)
    return noise3D(p*2.) - .3;
    
    // Two layers. The texture based 3D algorithm does this with ease, even on
    // slower machines, but the function based one starts having trouble. 
    // By the way, you need to load up the correct noise texture with the correct 
    // "vFlip" setting, if you wish to use this.
    //return noise3DTex(p*2.)*.66 + noise3DTex(p*4.)*.34 - .4;
	
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
    vec2 uv = vec2(0.);
    vec3 col = vec3(0.);
    vec3 sp;
    
    // Unit ray jitter is another way to hide artifacts. It can also trick the viewer into believing
    // something hard core, like global illumination, is happening. :)
    rd *= 0.99 + hash33(rd)*0.02;
    
    // Some more randomization, to be used for color based jittering inside the loop.
    vec3 rnd = hash33(rd+311.);
    
    
	// Ray distance, bail out layer number, surface distance and normalized accumulated distance.
	float t=0., layers=0., d, aD;
    
    // Surface distance threshold. Smaller numbers gives a thinner membrane, but lessens detail... 
    // hard to explain. It's easier to check it out for yourself.
    float thD = .025; // + smoothstep(-0.2, 0.2, sin(iGlobalTime*0.75 - 3.14159*0.4))*0.025;
 
	
    // Only a few iterations seemed to be enough. Obviously, more looks better, but is slower.
	for(float i=0.; i<56.; i++)	{
        
        // Break conditions. Anything that can help you bail early usually increases frame rate.
        if(layers>23. || col.x > 1. || t>10.) break;
        
        // Current ray postion. Slightly redundant here, but sometimes you may wish to reuse
        // it during the accumulation stage.
        sp = ro+rd*t;
		
        d = map(sp); // Distance to nearest point on the noise surface.
        
        // If we get within a certain distance of the surface, accumulate some surface values.
        // Values further away have less influence on the total.
        //
        // aD - Accumulated distance. You could smoothly interpolate it, if you wanted.
        //
        // 1/.(1. + t*t*0.1) - Basic distance attenuation. Feel free to substitute your own.
        
         // Normalized distance from the surface threshold value to our current isosurface value.
        aD = (thD-abs(d)*23./24.)/thD;
        
        // If we're within the surface threshold, accumulate some color.
        // Two "if" statements in a shader loop makes me nervous. I don't suspect there'll be any
        // problems, but if there are, let us know.
        if(aD>0.) { 
            
            // Add the accumulated surface distance value, along with some basic falloff (fog, if 
            // you prefer) using the camera to surface distance, "t." If you wanted, you could
            // add a light, and use the light position to ray position distance instead.
            // There's a bit of color jitter there, too.
            
            //col += aD*aD*(3.-2.*aD)/(1. + t*t*0.125)*.1 + (fract(rnd + i*27.)-.5)*0.02;
            
            col += aD/(1. + t*t*0.1)*.1 + (fract(rnd + i*27.)-.5)*0.02;
            
            // The layer number is worth noting. Accumulating more layers gives a bit more glow.
            // Lower layer numbers allow a quicker bailout. A lot of it is guess work.
            layers++;  
        }

		
        // Kind of weird the way this works. I think not allowing the ray to hone in properly is
        // the very thing that gives an even spread of values. The figures are based on a bit 
        // of knowledge versus trial and error. If you have a faster computer, feel free to tweak
        // them a bit.
        t += max(d*.7, thD*.7); 
        
			    
	}
    
     
    
    // Mixing the greytone color with a firey orange vignette. There's no meaning
    // behind it. I just thought the artsy greyscale was a little too artsy.
    //uv = abs(fragCoord.xy/iResolution.xy - .5); // Wasteful, but the GPU can handle it.
    col = mix(col, vec3(min(col.x*1.5, 1.), pow(col.x, 2.5), pow(col.x, 12.)),
               min( dot(pow(uv, vec2(4.)), vec2(1))*8., 1.));

    
	// Mixing the vignette colors up a bit more.
    col = mix(col, col.xzy, dot(sin(rd*5.), vec3(.166)) + 0.166);
    return col;
}

#ifndef RIFTRAY
void mainImage( out vec4 fragColor, vec2 fragCoord ) {

    
    // Screen coordinates.
	vec2 uv = (fragCoord.xy - iResolution.xy*.5 )/iResolution.y;
	
    // Unit direction ray. The last term is one of many ways to fish-lens the camera.
    // For a regular view, set "rd.z" to something like "0.5."
    vec3 rd = normalize(vec3(uv, (1.-dot(uv, uv)*.5)*.5)); // Fish lens, for that 1337, but tryhardish, demo look. :)
    
    // There are a few ways to hide artifacts and inconsistencies. Making things go fast is one of them. :)
    // Ray origin, scene color, and surface postion vector.
    vec3 ro = vec3(0., iGlobalTime*1.5, iGlobalTime*1.5), col=vec3(0), sp;
	
    // Swivel the unit ray to look around the scene.
    // Compact 2D rotation matrix, courtesy of Shadertoy user, "Fabrice Neyret."
    vec2 a = sin(vec2(1.5707963, 0) + iGlobalTime*0.375);
    rd.xz = mat2(a, -a.y, a.x)*rd.xz;    
    rd.xy = mat2(a, -a.y, a.x)*rd.xy; 
    
    col = getSceneColor(ro, rd);
	// Presenting the color to the screen.
	fragColor = vec4( clamp(col, 0., 1.), 1.0 );
 }
#endif
