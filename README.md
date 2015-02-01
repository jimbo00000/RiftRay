# RiftRay

[Oculus Rift DK2](http://www.oculus.com/) enabled viewer for the worlds of [Shadertoy](https://shadertoy.com).  


## Instructions  
Tap the side of the Rift with one finger to dismiss the Health and Safety Warning and recenter your view. Gaze at a picture in the gallery and tap the Rift to be transported. Tap again to return to the gallery.  


## Features  
- Floating AntTweakbar pane for live shader parameter editing  
- Dynamic framebuffer scaling  
- Vertical FOV adjustment(per developer guide section 8.5.4)  
- Xbox controller, Gamepad support  
- Sixense Razer Hydra support  
- Tracking volume display(on when near the boundary)  
- Auxiliary window display mirroring  


## Controls  
Gamepad and Hydra are supported, but optional. You can get by with only the HMD for input by tapping on it to select a shader in the gallery. To highlight a shader's thumbnail in the gallery, look straight at it or point the right Hydra controller at it.  

#### HMD:  
- **Tap on Headset** - Enter shader/return to gallery  

#### Keyboard:  
- **Space** - Recenter view  
- **Enter** - Select shader/return to gallery  
- **Escape** - Quit
- **WASD** - Movement  
- **QE13** - Movement(elevation/yaw)  
- **Shift/Control** - Slow/fast movement  
- **PgUp/PgDn** - Open/Close cinemascope(letterbox)  
- **F5-F8** - Adjust render target size manually  
- **F9-F11** - VSync Off/On/Adaptive  
- **Tab** - Toggle Tweakbar pane  
- **\\(Backslash)** - Click in pane  
- **/(Slash)** - Reposition pane  
- **R** - Reset world position  

#### Mouse:  
- **Wheel - adjust vertical** - Cinemascope | Letterbox | vFOV  
- **Left click & drag** - Yaw  
- **Right click & drag** - Movement in horizontal plane  
- **Middle click & drag** - Movement in vertical plane  

#### Xbox Controller:  
- **XYAB** - Movement  
- **Start** - Enter shader  
- **Left analog stick** - Yaw, Height  
- **Right analog stick** - Resolution, vFOV  
- **Left Trigger** - Move quickly  
- **Right Trigger** - Move slowly  
- **Back** - Toggle Tweakbar pane  
- **Left Bumper** - Reposition pane  
- **Right Bumper** - Click in pane  

#### Gravis Gamepad Pro:  
- **Right side buttons** - Movement  
- **Left Shoulder 1(upper)** - Move quickly/Reposition pane  
- **Left Shoulder 2(lower)** - Move slowly  
- **Start** - Enter shader  
- **Select** - Toggle Tweakbar pane  
- **Right Shoulder 1(upper)** - Click in pane  

#### Razer Hydra:  
- **Right Controller translation/rotation** - Select shader, pointer in pane  
- **Right Stick** - Movement(press in for vertical plane)  
- **Right Trigger** - Move quickly  
- **Right Start Button** - Enter shader  
- **Right Bumper** - Toggle Tweakbar pane  
- **Right Button 1** - Click in pane  
- **Right Button 2** - Reposition pane  
- **Left Stick** - Resolution, vFOV  

## Related work
  - [Shadertoy VR](http://www.reddit.com/r/oculus/comments/2q0ard/new_build_of_shadertoy_vr/) - Qt-enabled shader editing in VR  
    - Part of Brad Davis(jherico)'s [Oculus Rift in Action]()
  - [boxplorer2](https://code.google.com/p/boxplorer2/) - Marius Schilder  
  - [Fragmentarium](http://syntopia.github.io/Fragmentarium/) - native cross-platform shader IDE
  - [GLSL Sandbox](http://glslsandbox.com/) - web app, similar to Shadertoy
  - [Synthclipse](http://synthclipse.sourceforge.net/user_guide/shadertoy.html) - plugin for Eclipse


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
- Christophe Riccio for [GLM](http://glm.g-truc.net/0.9.5/index.html)  
