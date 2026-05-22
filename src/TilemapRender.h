#include "BufferObject.h"
#include "Shader.h"
#include "Allocator.h"
#include "types.h"
#include "window.h"
#include "Viewport.h"
#include "Entity.h"
#include "Consts.h"
#include "Syncing.h"
#include "iostream"

#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include <multiPlot.h>

struct GPUupload
{
	u32 offset;
	u32 chunkID;
	Syncing fence;

	GPUupload(u32 off, u32 id, Syncing sync)
	{
		offset = off;
		chunkID = id;
		fence = sync;
	}
	GPUupload()
	{

	}
};

struct TextureData
{
	u32 textureData;
	u32 R;
	u32 G;
	u32 B;

	TextureData(u32 texture, u32 red, u32 green, u32 blue)
	{
		textureData = texture;
		R = red;
		G = green;
		B = blue;
	}
	TextureData()
	{
	}
};

struct TilemapChunkData
{
	TextureData data[CHUNK_SIZE_SQUARED];
};

struct ChunkMetaData
{
	u32 offset;
	i32 x;
	i32 y;

	ChunkMetaData(u32 off, i32 X, i32 Y)
	{
		offset = off;
		x = X;
		y = Y;
	}
	ChunkMetaData()
	{
	}
};

struct ChunkLookup
{
	u32 offset;
	u32 id;

	ChunkLookup(u32 off, u32 chunkID)
	{
		offset = off;
		id = chunkID;
	}
};

struct IVec2Hash
{
	std::size_t operator()(const glm::ivec2& v) const
	{
		std::size_t hash = 17;

		hash = hash * 31 + std::hash<int>()(v.x);
		hash = hash * 31 + std::hash<int>()(v.y);

		return hash;
	}
};

class TilemapRender
{
public:
	TilemapRender();
	~TilemapRender();

	void updateInput();
	void updateCanvasEdit();

	void saveTilemap();
	void sliceTextureAtlas(char* filename);
	void openTilemapFile(char* filename);
	void newTilemapFile(char* filename, u32 sizeX, u32 sizeY);

	void update();
	void draw();

private:
	kl::BucketGPUMemory<CHUNK_SIZE_SQUARED * sizeof(TextureData)> textureTileData;
	std::vector<std::vector<TextureData>> tilemapBuffer;
	std::unordered_map<glm::ivec2, ChunkLookup, IVec2Hash> tilemapChunkPointerLookup;
	std::vector<ChunkMetaData> tilemapChunkPointer;

	std::unordered_map<glm::ivec2,GPUupload,IVec2Hash> GPUUploadQueueLookup;
	std::vector<glm::ivec2> GPUUploadQueue;

	u32 textureSelect = 1;
	glm::uvec3 colorChoosen = glm::uvec3(0xFF, 0xFF, 0xFF);
	u32 tileActionType = 0;

	bool isInitialized = false;

	bool my_tool_active;
	float colorSelected[3] = { 0.f, 0.f, 0.f};

	VertexArray<1> textureVertexArray;
	Shader shader;

	u32 texture = 0;
	int width, height;
	int tileWidthAndHeight[2];

	int tilemapChunkSizeImGUI[2];

	Viewport camera;

	char textureAtlasName[50];
	char tilemapDataName[50];

	std::string tilemapFile;
};