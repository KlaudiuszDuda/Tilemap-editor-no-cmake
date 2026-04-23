#version 430 core

layout (location = 0) in uvec4 textureData;
layout (location = 1) in uint baseInstanceX;
layout (location = 2) in uint baseInstanceY;

out vec2 TexCoord;
//out vec3 RGB;
out float brightness;

out vec3 corner0; // bottom-left
out vec3 corner1; // bottom-right
out vec3 corner2; // top-right
out vec3 corner3; // top-left

out vec2 vQuadUV;

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
    uint quadVertexID = gl_VertexID & 3;
    uint tileX = (textureData.x & 255) % atlasTileSize.x;
    uint tileY = (textureData.x & 255) / atlasTileSize.x;

    int rotation = int((textureData.x >> 8) & 3u);
    int flip = int((textureData.x >> 10) & 1u);

    int rotatedIndex = rotLUT[rotation][gl_VertexID];

    ivec2 uv = baseUVs[rotatedIndex];

    vQuadUV = vec2(uv);

    float u = float(tileX + uv.x) / float(atlasTileSize.x);
    float v = float(tileY + uv.y) / float(atlasTileSize.y);

    TexCoord = vec2(u, v);
    float R0 = 1.0f - float(textureData.y & 255u) / 255.f;
    float G0 = 1.0f - float(textureData.y & 255u) / 255.f;
    float B0 = 1.0f - float(textureData.y & 255u) / 255.f;

    float R1 = 1.0f - float((textureData.y >> 8) & 255u) / 255.f;
    float G1 = 1.0f - float((textureData.y >> 8) & 255u) / 255.f;
    float B1 = 1.0f - float((textureData.y >> 8) & 255u) / 255.f;

    float R2 = 1.0f - float((textureData.y >> 16) & 255u) / 255.f;
    float G2 = 1.0f - float((textureData.y >> 16) & 255u) / 255.f;
    float B2 = 1.0f - float((textureData.y >> 16) & 255u) / 255.f;

    float R3 = 1.0f - float((textureData.y >> 24) & 255u) / 255.f;
    float G3 = 1.0f - float((textureData.y >> 24) & 255u) / 255.f;
    float B3 = 1.0f - float((textureData.y >> 24) & 255u) / 255.f;
    
    corner0 = vec3(R0, G0, B0);
    corner1 = vec3(R1, G1, B1);
    corner2 = vec3(R2, G2, B2);
    corner3 = vec3(R3, G3, B3);
    brightness = 0;
    
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