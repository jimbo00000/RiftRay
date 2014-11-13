# Shader Porting Instructions

- Write or find a cool pixel shader on [http://www.shadertoy.com](http://www.shadertoy.com)  
- Leave camera setup intact in main  
- Extract the function:  

    **vec3 getSceneColor( in vec3 ro, in vec3 rd )**

    Ray directions will be calculated by RiftRay based on the HMD's orientation (and soon position)  
    @todo: Should we add a vec2 uv parameter to that function for post-processing?  

- Wrap main in **#ifndef RIFTRAY**
- Set RiftRay-specific variables using comments in the following format:  


	// @var author iq  
	// @var license CC BY-NC-SA 3.0  
	// @var url https://www.shadertoy.com/view/Mds3z2  
	// @var headSize 1.0  
	// @var eyePos 9.515 2.489 -23.0.993  
	// @var tex0 tex00.jpg  
	// @var tex1 tex09.jpg  
	// @var tex2 tex16.png  
	
	Texture filenames can be extracted from Chrome's developer console. Be sure to copy them in the right order. Copies of the image files from shadertoy reside in the textures/ directory.

- Save as a .glsl file in **shaders/**

The shader should still work in shadertoy, but will hopefully be drop-in compatible with RiftRay. The goal here is to keep it all in one text file of annotated frag shader source. (unless there's a cleaner way to do it)
