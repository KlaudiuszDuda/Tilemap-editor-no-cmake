#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
in vec4 RGBA;
in float brightness;

uniform sampler2D TextureAtlasID;

void main()
{
    FragColor = texture(TextureAtlasID, TexCoord) * RGBA + vec4(brightness, brightness, brightness, 0.0f);
}