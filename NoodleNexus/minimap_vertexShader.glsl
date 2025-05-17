#version 330 core

layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 1) in vec2 aTexCoord; // UV coordinates (if textured)

out vec2 TexCoord;

uniform mat4 model;      // Model matrix for the quad mesh
uniform mat4 view;       // View matrix (camera)
uniform mat4 projection; // Orthographic projection for minimap

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord; // Pass UVs to fragment shader
}