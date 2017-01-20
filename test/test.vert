#version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color;

out vec3 color;

uniform mat4 transform;
uniform float time;

void main()
{
	color = vertex_color;
	gl_Position = transform*vec4(vertex_position.xyz, 1.0);
}