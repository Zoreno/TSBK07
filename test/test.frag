#version 330 core

out vec4 color;

uniform float time;

void main()
{
	color = vec4(0.5f + 0.5f*sin(time), 0.5f + 0.5f*cos(time), 0.f, 1.0f);
}