#version 430

in vec3 fragPos;
in vec3 vertColor;
in vec3 normal;

out vec4 FragColor;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform bool lightingEnabled;

void main()
{
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    // Er�teljesebb s�rg�s f�ny
    vec3 warmLight = vec3(1.5, 0.8, 0.2); // er�sebben szatur�lt narancss�rga

    vec3 baseColor = vertColor;
    vec3 resultColor = baseColor;

    if (lightingEnabled) {
        // diff �rt�k felsk�l�zva, hogy jobban �rv�nyes�lj�n a vil�g�t�s
        float lightStrength = min(diff * 2.0, 1.0); // max 1.0
        resultColor = mix(baseColor, warmLight, lightStrength);
    }

    resultColor = clamp(resultColor, 0.0, 1.0);
    FragColor = vec4(resultColor, 1.0);
}
