#include "TilemapRender.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TilemapRender::TilemapRender()
{
	tilemapChunkSizeX = 4;
	tilemapChunkSizeY = 4;

	tilemapBuffer.resize(tilemapChunkSizeX * tilemapChunkSizeY * CHUNK_SIZE_SQUARED);
	tilemapChunkPointer.resize(tilemapChunkSizeX * tilemapChunkSizeY);
	transferingTilemapChunkPointer.resize(tilemapChunkSizeX * tilemapChunkSizeY);

	textureVertexArray.bind(0);
	textureTileData.init((tilemapChunkSizeX * tilemapChunkSizeY + 1) * CHUNK_SIZE_SQUARED * sizeof(u32));

	auto& buffer = textureTileData.getBuffer();
	u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
	void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, 0, 1, flags);

	for (u32 i = 0; i < tilemapChunkSizeX * tilemapChunkSizeY; i++)
	{
		transferingTilemapChunkPointer[i].oldOffset = 0u - 1;
		transferingTilemapChunkPointer[i].newOffset = textureTileData.alloc_init(dst, CHUNK_SIZE_SQUARED * sizeof(u32));
		transferingTilemapChunkPointer[i].chunkIndex = i;
	}
	buffer.UnmapBuffer(GL_ARRAY_BUFFER);

	fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	textureVertexArray.EnableVertexAttribArray(0);
	textureVertexArray.VertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u32), (void*)0);
	textureVertexArray.VertexAttribDivisor(0, 1);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int channels;
	uchar* data = stbi_load("resources/CosmicLilac_Tiles.png", &width, &height, &channels, 0);

	GLenum format = channels == 4 ? GL_RGBA : GL_RGB;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	shader.LoadShader("resources/vertex.vert", "resources/fragment.frag");
	shader.bind();

	width /= 16;
	height /= 16;

	shader.SetVector2i("atlasTileSize", width, height);
	shader.SetVector2i("tilemapChunkSize", tilemapChunkSizeX, tilemapChunkSizeY);
	shader.SetInt("TextureAtlasID", 0);
}

TilemapRender::~TilemapRender()
{

}

void TilemapRender::updateCamera()
{
	if (input.isActionActive(MappedInput::A))
	{
		camera.Position.x -= Time.dt * 5;
	}
	if (input.isActionActive(MappedInput::D))
	{
		camera.Position.x += Time.dt * 5;
	}
	if (input.isActionActive(MappedInput::S))
	{
		camera.Position.y -= Time.dt * 5;
	}
	if (input.isActionActive(MappedInput::W))
	{
		camera.Position.y += Time.dt * 5;
	}
	if (input.isActionActive(MappedInput::Q))
	{
		camera.Position.z -= Time.dt * 5;
	}
	if (input.isActionActive(MappedInput::E))
	{
		camera.Position.z += Time.dt * 5;
	}

	if (input.isActionActiveDigital(MappedInput::Z, GLFW_PRESS))
	{
		textureSelected--;
		if (textureSelected < 0)
		{
			textureSelected = height * width;
		}
	}
	if (input.isActionActiveDigital(MappedInput::X, GLFW_PRESS))
	{
		textureSelected++;
		if (textureSelected > height * width)
		{
			textureSelected = 0;
		}
	}

	if (input.isActionActiveDigital(MappedInput::T, GLFW_PRESS))
	{
		tileActionType = 0;
	}
	if (input.isActionActiveDigital(MappedInput::R, GLFW_PRESS))
	{
		tileActionType = 1;
	}
	if (input.isActionActiveDigital(MappedInput::F, GLFW_PRESS))
	{
		tileActionType = 2;
	}
}

void TilemapRender::updateCanvasEdit()
{
	u32 result = fence.ClientWaitSync(0, 0);

	if (!(result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) && transferingTilemapChunkPointer.size() == 0) return;

	if (input.getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_REPEAT)
	{
		glm::ivec3 mouseWorldPosition = window.getWorldMousePosition(camera);
		u32 mouseWorldPositionX = mouseWorldPosition.x;
		u32 mouseWorldPositionY = mouseWorldPosition.y;

		u32 mouseWorldChunkPositionX = mouseWorldPositionX / CHUNK_SIZE;
		u32 mouseWorldChunkPositionY = mouseWorldPositionY / CHUNK_SIZE;

		u32 mouseWorldChunkPositionIndexX = mouseWorldPositionX % CHUNK_SIZE;
		u32 mouseWorldChunkPositionIndexY = mouseWorldPositionY % CHUNK_SIZE;

		u32 mouseWorldChunkIndex = mouseWorldChunkPositionX + mouseWorldChunkPositionY * tilemapChunkSizeX;
		u32 mouseWorldIndex = mouseWorldChunkPositionIndexX + mouseWorldChunkPositionIndexY * CHUNK_SIZE;

		if (tileActionType == 0)
		{
			if (mouseWorldChunkPositionX > tilemapChunkSizeX - 1) return;
			if (mouseWorldChunkPositionY > tilemapChunkSizeY - 1) return;
			if (mouseWorldChunkPositionX < 0) return;
			if (mouseWorldChunkPositionY < 0) return;
			if (tilemapBuffer[mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED] == textureSelected) return;

			lastTileIndex = mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED;
			tilemapBuffer[lastTileIndex] = textureSelected;

			u32 offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(u32));

			auto& buffer = textureTileData.getBuffer();

			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(u32), flags);

			memcpy(dst, tilemapBuffer.data() + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED, CHUNK_SIZE_SQUARED * sizeof(u32));

			buffer.UnmapBuffer(GL_ARRAY_BUFFER);

			transferingTilemapChunkPointer.emplace_back(tileChunk(tilemapChunkPointer[mouseWorldChunkIndex], offset, mouseWorldChunkIndex));
		}
	}
	if (input.getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glm::ivec3 mouseWorldPosition = window.getWorldMousePosition(camera);
		u32 mouseWorldPositionX = mouseWorldPosition.x;
		u32 mouseWorldPositionY = mouseWorldPosition.y;

		u32 mouseWorldChunkPositionX = mouseWorldPositionX / CHUNK_SIZE;
		u32 mouseWorldChunkPositionY = mouseWorldPositionY / CHUNK_SIZE;

		u32 mouseWorldChunkPositionIndexX = mouseWorldPositionX % CHUNK_SIZE;
		u32 mouseWorldChunkPositionIndexY = mouseWorldPositionY % CHUNK_SIZE;

		u32 mouseWorldChunkIndex = mouseWorldChunkPositionX + mouseWorldChunkPositionY * tilemapChunkSizeX;
		u32 mouseWorldIndex = mouseWorldChunkPositionIndexX + mouseWorldChunkPositionIndexY * CHUNK_SIZE;

		if (tileActionType == 1)
		{
			if (mouseWorldChunkPositionX > tilemapChunkSizeX - 1) return;
			if (mouseWorldChunkPositionY > tilemapChunkSizeY - 1) return;
			if (mouseWorldChunkPositionX < 0) return;
			if (mouseWorldChunkPositionY < 0) return;

			lastTileIndex = mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED;
			u32 rotation = ((((tilemapBuffer[lastTileIndex] >> 8) & 3u) + 1) % 4);
			u32 bits = (rotation << 8);
			u32 mask = bits;
			mask |= 255;
			mask |= (1u << 21u) << 10u;
			tilemapBuffer[lastTileIndex] &= mask;
			tilemapBuffer[lastTileIndex] |= bits;

			u32 offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(u32));

			auto& buffer = textureTileData.getBuffer();

			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(u32), flags);

			memcpy(dst, tilemapBuffer.data() + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED, CHUNK_SIZE_SQUARED * sizeof(u32));

			buffer.UnmapBuffer(GL_ARRAY_BUFFER);

			transferingTilemapChunkPointer.emplace_back(tileChunk(tilemapChunkPointer[mouseWorldChunkIndex], offset, mouseWorldChunkIndex));
		}
		if (tileActionType == 2)
		{
			if (mouseWorldChunkPositionX > tilemapChunkSizeX - 1) return;
			if (mouseWorldChunkPositionY > tilemapChunkSizeY - 1) return;
			if (mouseWorldChunkPositionX < 0) return;
			if (mouseWorldChunkPositionY < 0) return;

			lastTileIndex = mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED;

			u32 flip = ((((tilemapBuffer[lastTileIndex] >> 10) & 1u) + 1) % 2);
			u32 bits = (flip << 10);
			u32 mask = bits;
			mask |= 1023;
			mask |= (1u << 19u) << 8u;
			tilemapBuffer[lastTileIndex] &= mask;
			tilemapBuffer[lastTileIndex] |= bits;

			u32 offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(u32));

			auto& buffer = textureTileData.getBuffer();

			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(u32), flags);

			memcpy(dst, tilemapBuffer.data() + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED, CHUNK_SIZE_SQUARED * sizeof(u32));

			buffer.UnmapBuffer(GL_ARRAY_BUFFER);

			transferingTilemapChunkPointer.emplace_back(tileChunk(tilemapChunkPointer[mouseWorldChunkIndex], offset, mouseWorldChunkIndex));
		}
	}
}

void TilemapRender::update()
{
	updateCanvasEdit();
	updateCamera();
}

void TilemapRender::draw()
{
	u32 result = fence.ClientWaitSync(0, 0);

	if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED && transferingTilemapChunkPointer.size())
	{
		for (u32 i = 0; i < transferingTilemapChunkPointer.size(); i++)
		{
			if (transferingTilemapChunkPointer[i].oldOffset != 0u - 1)
			{
				kl::Block b = kl::Block(transferingTilemapChunkPointer[i].oldOffset, CHUNK_SIZE_SQUARED * sizeof(u32));
				textureTileData.freeRegion(b);
			}
			tilemapChunkPointer[transferingTilemapChunkPointer[i].chunkIndex] = transferingTilemapChunkPointer[i].newOffset;
		}
		transferingTilemapChunkPointer.clear();
	}
	glDisable(GL_CULL_FACE);
	shader.SetMatrix4("viewAndProjection", camera.getViewAndProjection());

	const Frustum camFrustum = createFrustumFromCamera(camera, window.getWindowSize().x / window.getWindowSize().y, glm::radians(camera.Fov), 0.1f, 100000.0f);
	for (i32 i = 0; i < tilemapChunkPointer.size(); i++)
	{
		i32 x = i % tilemapChunkSizeX;
		i32 y = i / tilemapChunkSizeX;

		if (isAABBOnFrustum(setAABB({ x * CHUNK_SIZE, y * CHUNK_SIZE, 0 }, { (x + 1) * CHUNK_SIZE, (y + 1) * CHUNK_SIZE, 0 }), camFrustum))
		{
			textureVertexArray.VertexAttribI1ui(1, x);
			textureVertexArray.VertexAttribI1ui(2, y);
			textureVertexArray.VertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u32), (void*)(tilemapChunkPointer[i]));
			textureVertexArray.DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, CHUNK_SIZE_SQUARED);
		}
	}
	glEnable(GL_CULL_FACE);
}