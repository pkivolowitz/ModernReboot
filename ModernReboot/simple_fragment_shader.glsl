#version 400

layout (location = 0) out vec4 FragColor;

in vec3 color;
in vec3 p;

void main()
{
	FragColor = vec4(color, 1.0);
	if (abs(p.x) < 0.10)
		discard;
}
