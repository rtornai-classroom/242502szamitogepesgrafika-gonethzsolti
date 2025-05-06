#version 430

in vec4 myColor;
out vec4 color;

uniform bool isPoint;

void main(void)
{
    if (isPoint) {
        vec2 coord = gl_PointCoord * 2.0 - 1.0;
        if (dot(coord, coord) > 1.0) discard;
    }

    color = vec4(myColor.rgb, 1.0);
}
