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
	std::unordered_map<u32, u32> tileChunkIndex;

	u32 textureSelected = 0;

	Syncing fence;
	VertexArray<1> textureVertexArray;
	Shader shader;

	u32 texture;
	int width, height;

	i32 tilemapChunkSizeX;
	i32 tilemapChunkSizeY;

	u32 lastTileIndex = 0u-1;

	Viewport camera;
};