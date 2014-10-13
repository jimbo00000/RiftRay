// basic.frag
#version 330

in vec3 vfColor;
out vec4 fragColor;

void main()
{
    fragColor = vec4(vfColor, 1.0);
}