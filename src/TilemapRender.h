#include "BufferObject.h"
#include "Shader.h"
#include "Allocator.h"
#include "types.h"
#include "window.h"
#include "Viewport.h"
#include "Entity.h"
#include "Consts.h"
#include "Syncing.h"

struct tileChunk {
	u32 oldOffset;
	u32 newOffset;
	u32 chunkIndex;

	tileChunk(u32 oldoffset, u32 newoffset, u32 chunkindex)
	{
		oldOffset = oldoffset;
		newOffset = newoffset;
		chunkIndex = chunkindex;
	}
	tileChunk()
	{

	}
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
	std::vector<tileChunk> transferingTilemapChunkPointer;

	u32 textureSelect = 1;
	glm::uvec3 colorSelected = glm::uvec3(0xFF, 0x40, 0xFF);
	u32 tileActionType = 0;

	Syncing fence;
	VertexArray<1> textureVertexArray;
	Shader shader;

	u32 texture;
	int width, height;

	i32 tilemapChunkSizeX;
	i32 tilemapChunkSizeY;

	Viewport camera;
};