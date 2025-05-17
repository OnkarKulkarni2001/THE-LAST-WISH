#version 330 core

layout(location = 0) in vec3 aPos;  // Position attribute
layout(location = 1) in vec3 aColor; // Color attribute

uniform mat4 uView;
uniform mat4 uProj;

out vec3 fragColor;  // Output color to fragment shader

void main() {
    gl_Position = uProj * uView * vec4(aPos, 1.0);
    fragColor = aColor;  // Pass color to fragment shader
}
