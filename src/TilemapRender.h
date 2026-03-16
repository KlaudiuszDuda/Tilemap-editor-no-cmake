#include "BufferObject.h"
#include "Shader.h"
#include "Allocator.h"
#include "types.h"
#include "window.h"
#include "Viewport.h"
#include "Entity.h"
#include "Consts.h"
#include "Syncing.h"

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
	Syncing fence;
	VertexArray<1> textureVertexArray;
	Shader shader;
	u32 texture;

	i32 tilemapChunkSizeX;
	i32 tilemapChunkSizeY;

	Viewport camera;
};