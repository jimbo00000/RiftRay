// rwwtt_header.glsl
// Designed to be as compatible as possible with ShaderToy
// http://shadertoy.com
// Huge Thanks to Inigo Quilez for all the open code and tutorials!
#version 330
#define RIFTRAY
#define texture2D texture
#define gl_FragColor fragColor


in vec2 vfFragCoord;
out vec4 fragColor;


// ShaderToy Inputs:
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
//uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
//uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
//uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
//uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

// Oculus-specific additions:
uniform float u_eyeballCenterTweak;
uniform float u_fov_y_scale;
uniform mat4 mvmtx;
uniform mat4 prmtx;
uniform mat4 obmtx;

uniform float u_trigger;
