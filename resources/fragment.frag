#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
in float brightness;

in vec3 corner0; // bottom-left
in vec3 corner1; // bottom-right
in vec3 corner2; // top-right
in vec3 corner3; // top-left

in vec2 vQuadUV;

uniform sampler2D TextureAtlasID;

void main()
{
    vec3 color =
        mix(
            mix(corner0, corner1, vQuadUV.x),
            mix(corner2, corner3, vQuadUV.x),
            vQuadUV.y
        );

    FragColor = texture(TextureAtlasID, TexCoord) * vec4(color, 1.f) + vec4(brightness, brightness, brightness, 0.0f);
}