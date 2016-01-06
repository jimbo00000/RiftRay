# RiftRay - [github.com/jimbo00000/RiftRay](https://github.com/jimbo00000/RiftRay)

[Oculus Rift DK2](http://www.oculus.com/) enabled viewer for the worlds of [Shadertoy](https://shadertoy.com).  


## Instructions  
Tap the side of the Rift with one finger to dismiss the Health and Safety Warning and recenter your view. Gaze at a picture in the gallery and tap the Rift to be transported. Tap again to return to the gallery.  


## Features  
- Floating AntTweakbar pane for live shader parameter editing  
- Dynamic framebuffer scaling  
- Vertical FOV adjustment(per developer guide section 8.5.4)   
- Display mirroring  


## Controls  
Gamepad and Hydra are supported, but optional. You can get by with only the HMD for input by tapping on it to select a shader in the gallery. To highlight a shader's thumbnail in the gallery, look straight at it or point the right Hydra controller at it.  

#### HMD:  
- **Tap on Headset** - Enter shader/return to gallery  

#### Keyboard:  
- **Space** - Recenter view  
- **Enter** - Enter shader/return to gallery  
- **Escape** - Quit  
- **WASD** - Movement  
- **QE13** - Movement(elevation), Yaw  
- **Shift/Control** - Slow/fast movement  
- **PgUp/PgDn** - Open/Close cinemascope(letterbox)  
- **Tab** - Toggle Tweakbar pane  
- **Backspace** - Toggle OVR Perf HUD  
- **\\(Backslash)** - Click in pane  
- **/(Slash)** - Reposition pane  
- **R** - Reset world position  

#### Mouse:  
- **Wheel - adjust vertical** - Cinemascope | Letterbox | vFOV  
- **Middle click** - Toggle Tweakbar pane  
- **Hold Right click & Left Click** - Enter shader/return to gallery  
- **Hold Left click & Wheel** - Adjust render target size manually  

#### Xbox Controller:  
- **Start** - Enter shader/return to gallery  
- **Left analog stick** - Movement  
- **Right analog stick** - Yaw  
- **A** - Move up  
- **B** - Move down  
- **Y** - Toggle Tweakbar pane  
- **X** - Click in pane  
- **Left Bumper** - Recenter view  
- **Right Bumper** - Reset position  
- **Left Trigger** - Move quickly  
- **Right Trigger** - Move slowly  
- **Dpad** - Resolution, vFOV  

## Related work
  - [Shadertoy VR](http://www.reddit.com/r/oculus/comments/2q0ard/new_build_of_shadertoy_vr/) - Qt-enabled shader editing in VR  
    - Part of Brad Davis(jherico)'s [Oculus Rift in Action]()
  - [boxplorer2](https://code.google.com/p/boxplorer2/) - Marius Schilder  
  - [Fragmentarium](http://syntopia.github.io/Fragmentarium/) - native cross-platform shader IDE
  - [GLSL Sandbox](http://glslsandbox.com/) - web app, similar to Shadertoy
  - [Synthclipse](http://synthclipse.sourceforge.net/user_guide/shadertoy.html) - plugin for Eclipse
  - [Julius Horsthuis' videos](http://www.julius-horsthuis.com/vr-projects/)
  - [Mandelbulb VR](http://www.trapcode.com/journal/2015/12/18/mandelbulb-vr.html) - Peder Norrby, Trapcode
  - [Shdr](http://bkcore.com/blog/3d/shdr-online-glsl-shader-editor-viewer-validator.html) - Online GLSL shader editor, viewer and validator
  - [ShaderShop](http://tobyschachman.com/Shadershop/) - shader manipulation interface  

## Acknowledgments

All shader code under shaders/shadertoy/ was written by the top developers in the field and is covered by the **Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License** <http://creativecommons.org/licenses/by-nc-sa/3.0/us/> unless stated otherwise in the code.  

### Huge Thanks to:
- BeautyPi: Iñigo Quilez and Pol Jeremias for [Shadertoy](https://shadertoy.com) and all the code and tutorials on his site [www.iquilezles.org](http://www.iquilezles.org/)  
- All the members of [Shadertoy](https://shadertoy.com) for their beautiful GLSL code  
- Mikael Hvidtfeldt Christensen for this blog post [blog.hvidtfeldts.net](http://blog.hvidtfeldts.net/index.php/2014/01/combining-ray-tracing-and-polygons/)  
- Micah Dedmon for Mac porting and packaging  
- elmindreda for the awesome [Glfw3 framework](https://github.com/glfw/glfw)   
- Palmer Luckey and Oculus for the [Oculus Rift and OVR SDK](http://www.oculusvr.com/)  
- Philip Rideout for the [excellent CMake/OpenGL code](http://github.prideout.net/)  
- Philippe Decaudin for [AntTweakBar](http://anttweakbar.sourceforge.net/doc/)  
- Milan Ikits and Marcelo Magallon for [GLEW](http://glew.sourceforge.net/)  
- Christophe Riccio for [GLM](http://glm.g-truc.net/0.9.5/index.html)  
