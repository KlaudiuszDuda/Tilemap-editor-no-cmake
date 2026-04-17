#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
in vec4 RGBA;

uniform sampler2D TextureAtlasID;

void main()
{
    FragColor = texture(TextureAtlasID, TexCoord) * RGBA + vec4(0.0f, 0.0f, 0.0f, 0.0f);
}