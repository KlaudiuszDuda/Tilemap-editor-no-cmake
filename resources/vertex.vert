#version 430 core

layout (location = 0) in uvec2 textureData;
layout (location = 1) in uint baseInstanceX;
layout (location = 2) in uint baseInstanceY;

out vec2 TexCoord;
out vec4 RGBA;

uniform mat4 viewAndProjection;
uniform ivec2 atlasTileSize;
uniform ivec2 tilemapChunkSize;

// Quad corners (0..3)
ivec2 baseUVs[4] = ivec2[](
    ivec2(0, 0), // bottom-left
    ivec2(1, 0), // bottom-right
    ivec2(0, 1), // top-left
    ivec2(1, 1)  // top-right
);

// Rotation lookup (no branches, fast)
const int rotLUT[4][4] = int[4][4](
    int[4](0, 1, 2, 3), // 0°
    int[4](2, 3, 0, 1), // 90°
    int[4](3, 2, 1, 0), // 180°
    int[4](1, 0, 3, 2)  // 270°
);

void main()
{
    uint tileX = (textureData.x & 255) % atlasTileSize.x;
    uint tileY = (textureData.x & 255) / atlasTileSize.x;

    int rotation = int((textureData.x >> 8) & 3u);
    int flip = int((textureData.x >> 10) & 1u);

    int rotatedIndex = rotLUT[rotation][gl_VertexID];

    ivec2 uv = baseUVs[rotatedIndex];

    float u = float(tileX + uv.x) / float(atlasTileSize.x);
    float v = float(tileY + uv.y) / float(atlasTileSize.y);

    TexCoord = vec2(u, v);
    float R = 1.0f - float(textureData.y & 255u) / 255.f;
    float G = 1.0f - float((textureData.y >> 8) & 255u) / 255.f;
    float B = 1.0f - float((textureData.y >> 16) & 255u) / 255.f;
    float A = 1.0f - float((textureData.y >> 24) & 255u) / 255.f;
    
    RGBA = vec4(R, G, B, A);
    
    int vx = gl_VertexID % 2;
    int vy = (gl_VertexID / 2) % 2;

    if (flip == 1)
    {
        vx = 1 - vx;
    }

    float x = float(gl_InstanceID % 8) + baseInstanceX * 8;
    float y = float(gl_InstanceID / 8) + baseInstanceY * 8;

    float px = x + float(vx);
    float py = y + float(vy);

    gl_Position = viewAndProjection * vec4(px, py, 0.0, 1.0);
}