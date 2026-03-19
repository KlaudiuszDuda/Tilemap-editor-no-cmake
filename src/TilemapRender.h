#include "BufferObject.h"
#include "Shader.h"
#include "Allocator.h"
#include "types.h"
#include "window.h"
#include "Viewport.h"
#include "Entity.h"
#include "Consts.h"
#include "Syncing.h"

struct Block {
	u32 m_Offset;
	u32 m_Size;

	Block(u32 offset, u32 size)
	{
		m_Offset = offset;
		m_Size = size;
	}
};

struct tileChunk {
	u32 offset;
	u32 index;
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
	std::vector<u32> tilemapBuffer;
	std::vector<u32> tilemapChunkPointer;
	std::vector<tileChunk> transferingTilemapChunkPointer;
	u32 freeSlot;
	u32 needToBeFreed = 0u - 1;

	Syncing fence;
	VertexArray<1> textureVertexArray;
	Shader shader;
	u32 texture;

	i32 tilemapChunkSizeX;
	i32 tilemapChunkSizeY;

	u32 lastTileIndex = 0;

	Viewport camera;
};