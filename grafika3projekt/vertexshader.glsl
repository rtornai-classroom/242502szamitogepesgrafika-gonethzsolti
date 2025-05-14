#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out vec3 fragPos;      // vil�gkoordin�t�s poz�ci�
out vec3 vertColor;    // sz�n, ha nincs vil�g�t�s
out vec3 normal;       // norm�lvektor (egyszer�s�tett)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragPos = vec3(model * vec4(position, 1.0));
    vertColor = color;

    // Ha nincs explicit norm�lvektor, akkor felt�telez�nk egy fel�leti norm�lt (pl. a vertex poz�ci�b�l)
    normal = normalize(mat3(transpose(inverse(model))) * position); 

    gl_Position = projection * view * vec4(fragPos, 1.0);
}