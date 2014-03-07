// basic.frag

varying vec3 vfColor;

void main()
{
    gl_FragColor = vec4(vfColor, 1.0);
} 