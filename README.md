# RiftRay

Oculus Rift DK2 enabled viewer for the worlds of [Shadertoy](https://shadertoy.com).  


## Features  
- Adaptive performance scaling  
- Tracking volume display(on when near the boundary)  
- Auxiliary window display mirroring  
- Gamepad support  
- Sixense Razer Hydra support  
- Vertical FOV adjustment(per developer guide section 8.5.4)  


## Controls  
Gamepad and Hydra are supported, but optional. You can get by with only the HMD for input by tapping on it to select a shader in the gallery.  

To highlight a shader's thumbnail in the gallery, look straight at it or point the right Hydra controller at it.  

### Keyboard and HMD:  
- WASD - movement  
- Space bar - recenter  
- Enter - select shader/return to gallery  
- Tap on HMD - select shader/return to gallery  

### Mouse
- Wheel -adjust vertical FOV
- Right click & drag - movement in horizontal plane  
- Left click & drag - yaw  

### Gamepad:  
- Right hand buttons - movement  
- Shoulder buttons - height adjustment/speed boost  
- "Start" button - select shader/return to gallery  
- Hold "select" and press Left Shoulder buttons - adjust vertical FOV  

### Hydra:  
- Right stick - movement  
  - Press in and move stick for vertical movement
- Trigger - speed boost  
- Start button - select shader/return to gallery  
- Left trigger - adjust vertical FOV  


## Acknowledgments

All shader code under shaders/shadertoy/ was written by the top developers in the field and is covered by the **Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License** <http://creativecommons.org/licenses/by-nc-sa/3.0/us/> unless stated otherwise in the code.  

### Huge Thanks to:
- Iñigo Quilez for [Shadertoy](https://shadertoy.com) and all the code and tutorials on his site [www.iquilezles.org](http://www.iquilezles.org/)  
- All the members of [Shadertoy](https://shadertoy.com) for their beautiful GLSL code  
- Mikael Hvidtfeldt Christensen for the blog [blog.hvidtfeldts.net](http://blog.hvidtfeldts.net/)  
- Micah Dedmon for Mac porting and packaging  
- elmindreda for the awesome [Glfw3 framework](https://github.com/glfw/glfw)   
- Palmer Luckey and Oculus for the [Oculus Rift and OVR SDK](http://www.oculusvr.com/)  
- Philip Rideout for the [excellent CMake/OpenGL code](http://github.prideout.net/)  
- Philippe Decaudin for [AntTweakBar](http://anttweakbar.sourceforge.net/doc/)  
- Milan Ikits and Marcelo Magallon for [GLEW](http://glew.sourceforge.net/)  
