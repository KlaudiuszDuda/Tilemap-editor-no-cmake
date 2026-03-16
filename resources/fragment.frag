#version 430 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D TextureAtlasID;

void main()
{
	FragColor = texture(TextureAtlasID, TexCoord);
}