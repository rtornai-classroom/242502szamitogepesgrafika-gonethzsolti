#version 430

in vec4 myColor;
out vec4 color;

void main(void)
{
	color = vec4(myColor);
}