#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 fragColor;

uniform vec2 circleCenter;
uniform bool colorSwap; // Swap colors on collision

void main() {
    // Move the circle's position based on uniform value
    gl_Position = vec4(position.x + circleCenter.x, position.y + circleCenter.y, position.z, 1.0);
    
    // Swap colors when needed
    if (colorSwap) {
        if (color.g > color.r) {  // If originally green (outer part)
            fragColor = vec3(1.0f, 0.0f, 0.0f); // Change to red
        } else { // If originally red (center)
            fragColor = vec3(0.0f, 1.0f, 0.0f); // Change to green
        }
    } else {
        fragColor = color;
    }
}
