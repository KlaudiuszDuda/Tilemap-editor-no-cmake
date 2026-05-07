#version 330 core

layout (location = 0) in uvec4 textureData;
layout (location = 1) in int baseInstanceX;
layout (location = 2) in int baseInstanceY;

out vec2 TexCoord;
out float brightness;

out vec3 corner0; // bottom-left
out vec3 corner1; // bottom-right
out vec3 corner2; // top-right
out vec3 corner3; // top-left

out vec2 vQuadUV;

uniform mat4 viewAndProjection;
uniform ivec2 atlasTileSize;

// Quad corners (0..3)
ivec2 baseUVs[4] = ivec2[4](
    ivec2(0, 0),
    ivec2(1, 0),
    ivec2(0, 1),
    ivec2(1, 1)
);

// Rotation lookup (no branches, fast)
const int rotLUT[16] = int[16](
    0, 1, 2, 3,
    2, 3, 0, 1,
    3, 2, 1, 0,
    1, 0, 3, 2
);

void main()
{
    int quadVertexID = gl_VertexID & 3;
    
    int tile = int(textureData.x & 255u);
    int tileX = tile % atlasTileSize.x;
    int tileY = tile / atlasTileSize.x;

    int rotation = int((textureData.x >> 8) & 3u);
    int flip = int((textureData.x >> 10) & 1u);

    int rotatedIndex = rotLUT[rotation * 4 + quadVertexID];

    ivec2 uv = baseUVs[rotatedIndex];

    vQuadUV = vec2(uv);

    float u = float(tileX + uv.x) / float(atlasTileSize.x);
    float v = float(tileY + uv.y) / float(atlasTileSize.y);

    TexCoord = vec2(u, v);
    float R0 = float(textureData.y & 255u) / 255.f;
    float G0 = float(textureData.z & 255u) / 255.f;
    float B0 = float(textureData.w & 255u) / 255.f;
               
    float R1 = float((textureData.y >> 8) & 255u) / 255.f;
    float G1 = float((textureData.z >> 8) & 255u) / 255.f;
    float B1 = float((textureData.w >> 8) & 255u) / 255.f;
               
    float R2 = float((textureData.y >> 16) & 255u) / 255.f;
    float G2 = float((textureData.z >> 16) & 255u) / 255.f;
    float B2 = float((textureData.w >> 16) & 255u) / 255.f;
               
    float R3 = float((textureData.y >> 24) & 255u) / 255.f;
    float G3 = float((textureData.z >> 24) & 255u) / 255.f;
    float B3 = float((textureData.w >> 24) & 255u) / 255.f;
    
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