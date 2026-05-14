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

struct GPUupload
{
	u64 offset;
	Syncing fence;
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
	void newTilemapFile(char* filename);

	void update();
	void draw();

private:
	kl::ChunkBufferAllocator textureTileData;
	std::vector<TextureData> tilemapBuffer;
	std::vector<u32> tilemapChunkPointer;

	std::vector<GPUupload> GPUUploadQueue;

	u32 textureSelect = 1;
	glm::uvec3 colorChoosen = glm::uvec3(0xFF, 0x40, 0xFF);
	u32 tileActionType = 0;

	bool isInitialized = false;

	bool my_tool_active;
	float colorSelected[3] = { 0.f, 0.f, 0.f};

	VertexArray<1> textureVertexArray;
	Shader shader;

	u32 texture = 0;
	int width, height;
	int tileWidthAndHeight[2];

	int tilemapChunkSize[2];
	int tilemapChunkSizeImGUI[2];

	Viewport camera;

	char textureAtlasName[50];
	char tilemapDataName[50];

	std::string tilemapFile;
};