#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textureMap; // Texture for the quad mesh (optional)
uniform bool useTexture;      // Toggle texture vs. flat color

void main()
{
    if (useTexture) {
        FragColor = texture(textureMap, TexCoord);
    } else {
        FragColor = vec4(0.5, 0.5, 0.5, 1.0); // Flat color (e.g., gray)
    }
}