#version 330 core

in vec3 fragColor;  // Incoming color from vertex shader
out vec4 FragColor;

uniform vec3 uColor;

void main() {
    FragColor = vec4(fragColor, 1.0);
    //FragColor = vec4(uColor, 1.0);
}
