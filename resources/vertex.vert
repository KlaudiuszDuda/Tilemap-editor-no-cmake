#version 430 core

layout (location = 0) in uint textureData;
layout (location = 1) in uint baseInstanceX;
layout (location = 2) in uint baseInstanceY;

out vec2 TexCoord;

uniform mat4 viewAndProjection;
uniform ivec2 atlasTileSize;
uniform ivec2 tilemapChunkSize;

void main()
{
    uint tileX = textureData % atlasTileSize.x;
    uint tileY = textureData / atlasTileSize.x;

    int ux = gl_VertexID % 2;
    int uy = (gl_VertexID / 2) % 2;

    float u = float(tileX + ux) / float(atlasTileSize.x);
    float v = float(tileY + uy) / float(atlasTileSize.y);

    TexCoord = vec2(u, v);
    
    int vx = gl_VertexID % 2;
    int vy = (gl_VertexID / 2) % 2;

    float x = float(gl_InstanceID % 8) + baseInstanceX * 8;
    float y = float(gl_InstanceID / 8) + baseInstanceY * 8;

    float px = x + float(vx);
    float py = y + float(vy);

    gl_Position = viewAndProjection * vec4(px, py, 0.0, 1.0);
}