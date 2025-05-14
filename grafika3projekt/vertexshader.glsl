#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

out vec3 fragPos;      // világkoordinátás pozíció
out vec3 vertColor;    // szín, ha nincs világítás
out vec3 normal;       // normálvektor (egyszerûsített)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragPos = vec3(model * vec4(position, 1.0));
    vertColor = color;

    // Ha nincs explicit normálvektor, akkor feltételezünk egy felületi normált (pl. a vertex pozícióból)
    normal = normalize(mat3(transpose(inverse(model))) * position); 

    gl_Position = projection * view * vec4(fragPos, 1.0);
}