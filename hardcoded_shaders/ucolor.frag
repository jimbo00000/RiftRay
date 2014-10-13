// ucolor.frag
#version 330

uniform vec4 u_Color;

out vec4 fragColor;

void main()
{
    fragColor = u_Color;
}
