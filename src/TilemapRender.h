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
};

class TilemapRender
{
public:
	TilemapRender();
	~TilemapRender();

	void updateCamera();
	void updateCanvasEdit();

	void update();
	void draw();

private:
	kl::ChunkBufferAllocator textureTileData;
	std::vector<TextureData> tilemapBuffer;
	std::vector<u32> tilemapChunkPointer;

	std::vector<GPUupload> GPUUploadQueue;

	u32 textureSelect = 1;
	glm::uvec3 colorSelected = glm::uvec3(0xFF, 0x40, 0xFF);
	u32 tileActionType = 0;

	VertexArray<1> textureVertexArray;
	Shader shader;

	u32 texture;
	int width, height;

	glm::ivec2 tilemapChunkSize;

	Viewport camera;
};