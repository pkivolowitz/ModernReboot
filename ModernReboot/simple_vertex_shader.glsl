#version 400

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color;

uniform mat4 mvp;

out vec3 color;
out vec3 p;

void main()
{
	gl_Position = mvp * vec4(vertex_position, 1.0);
	color = vertex_color;
	p = vec3(vertex_position);
}
