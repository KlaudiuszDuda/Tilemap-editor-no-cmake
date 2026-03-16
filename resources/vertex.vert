#version 430 core

layout (location = 0) in uint textureData;
layout (location = 1) in uint baseInstance;

out vec2 TexCoord;

uniform mat4 viewAndProjection;
uniform ivec2 atlasTileSize;
uniform ivec2 tilemapChunkSize;

void main()
{
    uint tileX = textureData % atlasTileSize.x;
    uint tileY = textureData / atlasTileSize.x;

    int vx = gl_VertexID % 2;
    int vy = (gl_VertexID / 2) % 2;

    float u = float(tileX + vx) / float(atlasTileSize.x);
    float v = float(tileY + vy) / float(atlasTileSize.y);

    TexCoord = vec2(u, v);

    float x = float(gl_InstanceID % 8) + baseInstance % tilemapChunkSize.x * 8;
    float y = float(gl_InstanceID / 8) + baseInstance / tilemapChunkSize.y * 8;

    float px = x + float(vx);
    float py = y + float(vy);

    gl_Position = viewAndProjection * vec4(px, py, 0.0, 1.0);
}