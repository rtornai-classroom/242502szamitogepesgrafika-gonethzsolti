#version 430

layout (location = 0) in vec3 aPos;

uniform vec3 color = vec3(0.0, 1.0, 0.0);

out vec4 myColor;

void main(void)
{
	gl_Position = vec4(aPos, 1.0);
	myColor = vec4(color, 0.00);
}