#include "TilemapRender.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TilemapRender::TilemapRender()
{
	
}

TilemapRender::~TilemapRender()
{
	saveTilemap();
}

void TilemapRender::updateInput()
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
		if (camera.Position.z < 0.1f)
		{
			camera.Position.z = 0.1f;
		}
	}
	if (input.isActionActive(MappedInput::E))
	{
		camera.Position.z += Time.dt * 5;
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
	if (input.isActionActiveDigital(MappedInput::C, GLFW_PRESS))
	{
		tileActionType = 3;
	}
	if (input.isActionActiveDigital(MappedInput::X, GLFW_PRESS))
	{
		tileActionType = 4;
	}
}

void TilemapRender::updateCanvasEdit()
{
	if (!isInitialized) return;
	if (input.getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_REPEAT)
	{
		glm::ivec3 mouseWorldPosition = (glm::ivec3)window.getWorldMousePosition(camera);
		glm::ivec3 floatMouseWorldPosition = (glm::ivec3)(window.getWorldMousePosition(camera) * glm::vec3(2.f, 2.f, 2.f));

		if (mouseWorldPosition.x < 0) return;
		if (mouseWorldPosition.y < 0) return;

		u32 mouseWorldPositionXsigned = mouseWorldPosition.x;
		u32 mouseWorldPositionYsigned = mouseWorldPosition.y;
		
		u32 mouseWorldChunkPositionX = mouseWorldPositionXsigned / CHUNK_SIZE;
		u32 mouseWorldChunkPositionY = mouseWorldPositionYsigned / CHUNK_SIZE;
		
		u32 mouseWorldChunkPositionIndexX = mouseWorldPositionXsigned % CHUNK_SIZE;
		u32 mouseWorldChunkPositionIndexY = mouseWorldPositionYsigned % CHUNK_SIZE;
		
		u32 mouseWorldIndex = mouseWorldChunkPositionIndexX + mouseWorldChunkPositionIndexY * CHUNK_SIZE;
		
		u32 quadVertexIDX = floatMouseWorldPosition.x % 2;
		u32 quadVertexIDY = floatMouseWorldPosition.y % 2;
		u32 quadVertexID = quadVertexIDX + quadVertexIDY * 2;

		if (tileActionType == 0)
		{
			auto tilemapChunkpointer = tilemapChunkPointerLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (tilemapChunkpointer == tilemapChunkPointerLookup.end()) return;
			auto GPUUpload = GPUUploadQueueLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (GPUUpload != GPUUploadQueueLookup.end()) return;

			if (tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].textureData == textureSelect) return;

			tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].textureData = textureSelect;

			u32 offset = textureTileData.alloc();

			auto& buffer = textureTileData.getBuffer();

			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);

			memcpy(dst, tilemapBuffer[tilemapChunkpointer->second.id].data(), CHUNK_SIZE_SQUARED * sizeof(TextureData));

			buffer.UnmapBuffer(GL_ARRAY_BUFFER);

			Syncing fence;
			fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

			GPUUploadQueueLookup.emplace(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY), GPUupload(offset, tilemapChunkpointer->second.id, fence));
			GPUUploadQueue.emplace_back(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
		}
		if (tileActionType == 3)
		{
			auto tilemapChunkpointer = tilemapChunkPointerLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (tilemapChunkpointer == tilemapChunkPointerLookup.end()) return;
			auto GPUUpload = GPUUploadQueueLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (GPUUpload != GPUUploadQueueLookup.end()) return;

			u32 colorR = (u32)(colorSelected[0] * 255);
			u32 colorG = (u32)(colorSelected[1] * 255);
			u32 colorB = (u32)(colorSelected[2] * 255);

			i32 rotation = tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].textureData >> 30 & 3;
			i32 rot = rotation;
			i32 vertex = quadVertexID;
			i32 shift;

			if(rot == 3)
			{
				rotation = 2;
				shift = ((rotation + quadVertexID) & 3) * 8;
			}
			if(rot == 1)
			{
				if (vertex == 1) quadVertexID = 3;
				if (vertex == 3) quadVertexID = 1;
				shift = ((rotation - quadVertexID) & 3) * 8;
			}
			if(rot == 2)
			{
				if (vertex == 0) quadVertexID = 3;
				if (vertex == 1) quadVertexID = 0;
				if (vertex == 2) quadVertexID = 1;
				if (vertex == 3) quadVertexID = 2;
				shift = ((rotation - quadVertexID) & 3) * 8;
			}
			else
			{
				shift = ((rotation + quadVertexID) & 3) * 8;
			}

			u32 R = (tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].R >> shift) & 255u;
			u32 G = (tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].G >> shift) & 255u;
			u32 B = (tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].B >> shift) & 255u;
			if (R == colorR && G == colorG && B == colorB) return;

			tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].R =
				(tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].R & ~(0xFFu << shift)) |
				((colorR) << shift);

			tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].G =
				(tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].G & ~(0xFFu << shift)) |
				((colorG) << shift);

			tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].B =
				(tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].B & ~(0xFFu << shift)) |
				((colorB) << shift);

			u32 offset = textureTileData.alloc();

			auto& buffer = textureTileData.getBuffer();

			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);

			memcpy(dst, tilemapBuffer[tilemapChunkpointer->second.id].data(), CHUNK_SIZE_SQUARED * sizeof(TextureData));

			buffer.UnmapBuffer(GL_ARRAY_BUFFER);

			Syncing fence;
			fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

			GPUUploadQueueLookup.emplace(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY), GPUupload(offset, tilemapChunkpointer->second.id, fence));
			GPUUploadQueue.emplace_back(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
		}
	}

	if (input.getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glm::ivec3 mouseWorldPosition = window.getWorldMousePosition(camera);

		if (mouseWorldPosition.x < 0) return;
		if (mouseWorldPosition.y < 0) return;
		u32 mouseWorldPositionX = mouseWorldPosition.x;
		u32 mouseWorldPositionY = mouseWorldPosition.y;

		u32 mouseWorldChunkPositionX = mouseWorldPositionX / CHUNK_SIZE;
		u32 mouseWorldChunkPositionY = mouseWorldPositionY / CHUNK_SIZE;

		u32 mouseWorldChunkPositionIndexX = mouseWorldPositionX % CHUNK_SIZE;
		u32 mouseWorldChunkPositionIndexY = mouseWorldPositionY % CHUNK_SIZE;

		u32 mouseWorldIndex = mouseWorldChunkPositionIndexX + mouseWorldChunkPositionIndexY * CHUNK_SIZE;

		if (tileActionType == 1)
		{
			auto tilemapChunkpointer = tilemapChunkPointerLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (tilemapChunkpointer == tilemapChunkPointerLookup.end()) return;
			auto GPUUpload = GPUUploadQueueLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (GPUUpload != GPUUploadQueueLookup.end()) return;

			u32 rotation = ((((tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].textureData >> 30) & 3u) + 1) % 4);
			u32 bits = (rotation << 30);
			u32 mask = bits;
			mask |= ((0u - 1) >> 2);
			tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].textureData &= mask;
			tilemapBuffer[tilemapChunkpointer->second.id][mouseWorldIndex].textureData |= bits;

			u32 offset = textureTileData.alloc();

			auto& buffer = textureTileData.getBuffer();

			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);

			memcpy(dst, tilemapBuffer[tilemapChunkpointer->second.id].data(), CHUNK_SIZE_SQUARED * sizeof(TextureData));

			buffer.UnmapBuffer(GL_ARRAY_BUFFER);

			Syncing fence;
			fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

			GPUUploadQueueLookup.emplace(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY), GPUupload(offset, tilemapChunkpointer->second.id, fence));
			GPUUploadQueue.emplace_back(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
		}
		if (tileActionType == 2)
		{
			auto tilemapChunkpointer = tilemapChunkPointerLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (tilemapChunkpointer != tilemapChunkPointerLookup.end()) return;

			tilemapBuffer.emplace_back();
			tilemapBuffer[(tilemapBuffer.size() - 1)].resize(CHUNK_SIZE_SQUARED);
			std::fill(tilemapBuffer[(tilemapBuffer.size() - 1)].data(), tilemapBuffer[(tilemapBuffer.size() - 1)].data() + CHUNK_SIZE_SQUARED, TextureData(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
			
			tilemapChunkPointer.emplace_back(ChunkMetaData(0u - 1, mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			tilemapChunkPointerLookup.emplace(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY), ChunkLookup(0u-1, (tilemapChunkPointer.size() - 1)));
			
			u32 offset = textureTileData.alloc();
			
			auto& buffer = textureTileData.getBuffer();
			
			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);
			
			memcpy(dst, tilemapBuffer[(tilemapBuffer.size() - 1)].data(), CHUNK_SIZE_SQUARED * sizeof(TextureData));
			
			buffer.UnmapBuffer(GL_ARRAY_BUFFER);
			
			Syncing fence;
			fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			
			GPUUploadQueueLookup.emplace(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY), GPUupload(offset, (tilemapBuffer.size() - 1), fence));
			GPUUploadQueue.emplace_back(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
		}
		if (tileActionType == 4)
		{
			auto tilemapChunkpointer = tilemapChunkPointerLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (tilemapChunkpointer == tilemapChunkPointerLookup.end()) return;
			auto GPUUpload = GPUUploadQueueLookup.find(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
			if (GPUUpload != GPUUploadQueueLookup.end()) return;

			auto tilemapChunkpointerUpdate = tilemapChunkPointerLookup.find(glm::ivec2(tilemapChunkPointer.back().x, tilemapChunkPointer.back().y));
			tilemapChunkpointerUpdate->second.id = tilemapChunkpointer->second.id;

			tilemapChunkPointer[tilemapChunkpointer->second.id] = tilemapChunkPointer.back();
			tilemapChunkPointer.pop_back();

			tilemapBuffer[tilemapChunkpointer->second.id] = tilemapBuffer.back();
			tilemapBuffer.pop_back();

			textureTileData.freeRegion(tilemapChunkpointer->second.offset);
			tilemapChunkPointerLookup.erase(glm::ivec2(mouseWorldChunkPositionX, mouseWorldChunkPositionY));
		}
	}
}

void TilemapRender::saveTilemap()
{
	if (isInitialized)
	{
		std::ofstream tilemapData;

		tilemapData.open(tilemapFile, std::ios::binary | std::ios::out);
		tilemapData.write(textureAtlasName, 100);
		tilemapData.seekp(100, std::ios::beg);
		tilemapData.write(reinterpret_cast<char*>(tileWidthAndHeight), sizeof(int) * 2);
		for (i32 i = 0; i < tilemapBuffer.size(); i++)
		{
			tilemapData.seekp(100 + sizeof(int) * 2 + i * CHUNK_SIZE_SQUARED * sizeof(TextureData), std::ios::beg);
			tilemapData.write(reinterpret_cast<char*>(tilemapBuffer[i].data()), CHUNK_SIZE_SQUARED * sizeof(TextureData));
		}
		std::vector<glm::ivec2> tempBuffer;
		tempBuffer.resize(tilemapChunkPointer.size());
		for (i32 i = 0; i < tempBuffer.size(); i++)
		{
			tempBuffer[i].x = tilemapChunkPointer[i].x;
			tempBuffer[i].y = tilemapChunkPointer[i].y;
		}
		tilemapData.seekp(100 + sizeof(int) * 2 + tilemapBuffer.size() * CHUNK_SIZE_SQUARED * sizeof(TextureData), std::ios::beg);
		tilemapData.write(reinterpret_cast<char*>(tempBuffer.data()), tempBuffer.size() * sizeof(glm::ivec2));

		tilemapData.close();
	}
}

void TilemapRender::sliceTextureAtlas(char* filename)
{
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int channels;
	std::string path = std::string("resources/") + filename;
	uchar* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

	GLenum format = channels == 4 ? GL_RGBA : GL_RGB;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	shader.LoadShader("resources/vertex.vert", "resources/fragment.frag");
	shader.bind();

	width /= tileWidthAndHeight[0];
	height /= tileWidthAndHeight[1];

	shader.SetVector2i("atlasTileSize", width, height);

	shader.SetInt("TextureAtlasID", 0);
}

void TilemapRender::openTilemapFile(char* filename)
{
	std::ifstream tilemapData;

	std::string tempString;
	tempString = std::string("resources/") + filename + ".tmk";
	tilemapData.open(tempString, std::ios::binary | std::ios::in | std::ios::ate);
	if (!tilemapData) return;

	tilemapFile = tempString;

	std::streamsize size = tilemapData.tellg();
	u32 amountOfChunks = (size - 100 + sizeof(int) * 2) / (CHUNK_SIZE_SQUARED * sizeof(TextureData) + sizeof(glm::ivec2));

	tilemapData.seekg(0, std::ios::beg);

	tilemapBuffer.clear();
	tilemapBuffer.resize(amountOfChunks);
	for (i32 i = 0; i < tilemapBuffer.size(); i++)
	{
		tilemapBuffer[i].resize(CHUNK_SIZE_SQUARED);
	}
	tilemapData.read(textureAtlasName, 100);
	tilemapData.seekg(100, std::ios::beg);
	tilemapData.read(reinterpret_cast<char*>(tileWidthAndHeight), sizeof(int) * 2);
	for (i32 i = 0; i < amountOfChunks; i++)
	{
		tilemapData.seekg(100 + sizeof(int) * 2 + i * CHUNK_SIZE_SQUARED * sizeof(TextureData), std::ios::beg);
		tilemapData.read(reinterpret_cast<char*>(tilemapBuffer[i].data()), CHUNK_SIZE_SQUARED * sizeof(TextureData));
	}
	std::vector<glm::ivec2> tempBuffer;
	tempBuffer.resize(amountOfChunks);

	tilemapData.seekg(100 + sizeof(int) * 2 + tilemapBuffer.size() * CHUNK_SIZE_SQUARED * sizeof(TextureData), std::ios::beg);
	tilemapData.read(reinterpret_cast<char*>(tempBuffer.data()), tempBuffer.size() * sizeof(glm::ivec2));

	tilemapData.close();

	if (!isInitialized)
	{
		tilemapChunkPointer.resize(amountOfChunks);
		tilemapChunkPointerLookup.reserve(amountOfChunks);
		for (i32 i = 0; i < tilemapChunkPointer.size(); i++)
		{
			tilemapChunkPointer[i].offset = 0u-1;
			tilemapChunkPointer[i].x = tempBuffer[i].x;
			tilemapChunkPointer[i].y = tempBuffer[i].y;
			tilemapChunkPointerLookup.emplace(glm::ivec2(tempBuffer[i].x, tempBuffer[i].y), ChunkLookup(0u - 1, i));
		}

		textureVertexArray.bind(0);
		textureTileData.init(amountOfChunks * 2);

		textureVertexArray.EnableVertexAttribArray(0);
		textureVertexArray.VertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(TextureData), (void*)0);
		textureVertexArray.VertexAttribDivisor(0, 1);

		isInitialized = true;
	}
	else
	{
		for (i32 i = 0; i < tilemapChunkPointer.size(); i++)
		{
			textureTileData.freeRegion(tilemapChunkPointer[i].offset);
			tilemapChunkPointer[i].offset = 0u - 1;
		}
		tilemapChunkPointerLookup.clear();
		for (i32 i = 0; i < amountOfChunks; i++)
		{
			tilemapChunkPointerLookup.emplace(glm::ivec2(tempBuffer[i].x, tempBuffer[i].y), ChunkLookup(0u - 1, i));
		}

	}

	auto& buffer = textureTileData.getBuffer();
	u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;

	for (u32 i = 0; i < amountOfChunks; i++)
	{
		u32 offset = textureTileData.alloc();

		void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);
		memcpy(dst, tilemapBuffer[i].data(), CHUNK_SIZE_SQUARED * sizeof(TextureData));
		buffer.UnmapBuffer(GL_ARRAY_BUFFER);

		Syncing fence;
		fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		GPUUploadQueueLookup.emplace(glm::ivec2(tempBuffer[i].x, tempBuffer[i].y), GPUupload(offset, i, fence));
		GPUUploadQueue.emplace_back(glm::ivec2(tempBuffer[i].x, tempBuffer[i].y));
	}
}

void TilemapRender::newTilemapFile(char* filename, u32 sizeX, u32 sizeY)
{
	if (tilemapChunkSizeImGUI[0] <= 0 || tilemapChunkSizeImGUI[1] <= 0) return;
	tilemapFile = std::string("resources/") + filename + ".tmk";
	std::ofstream newFile(tilemapFile, std::ios::binary);

	tilemapBuffer.clear();
	tilemapBuffer.shrink_to_fit();
	tilemapChunkPointer.clear();
	tilemapChunkPointer.shrink_to_fit();
	tilemapChunkPointerLookup.clear();

	tilemapBuffer.resize(sizeX * sizeY);
	for (i32 i = 0; i < tilemapBuffer.size(); i++)
	{
		tilemapBuffer[i].resize(CHUNK_SIZE_SQUARED);
		std::fill(tilemapBuffer[i].data(), tilemapBuffer[i].data() + CHUNK_SIZE_SQUARED, TextureData(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
	}

	tilemapChunkPointer.resize(sizeX * sizeY);
	for (i32 i = 0; i < tilemapChunkPointer.size(); i++)
	{
		i32 x = i % sizeX;
		i32 y = i / sizeX;
		tilemapChunkPointer[i].offset = 0u - 1;
		tilemapChunkPointer[i].x = x;
		tilemapChunkPointer[i].y = y;
	}
	tilemapChunkPointerLookup.reserve(sizeX * sizeY);
	for (i32 i = 0; i < sizeX * sizeY; i++)
	{
		i32 x = i % sizeX;
		i32 y = i / sizeX;

		tilemapChunkPointerLookup.emplace(glm::ivec2(x, y), ChunkLookup(0u - 1, i));
	}

	textureVertexArray.bind(0);
	textureTileData.init((sizeX * sizeY * 2) * CHUNK_SIZE_SQUARED * sizeof(TextureData));

	auto& buffer = textureTileData.getBuffer();
	u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;

	for (u32 i = 0; i < sizeX * sizeY; i++)
	{
		i32 x = i % sizeX;
		i32 y = i / sizeX;

		u32 offset = textureTileData.alloc();
		TextureData* dst = (TextureData*)buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);
		std::fill(dst, dst + CHUNK_SIZE_SQUARED, TextureData(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
		buffer.UnmapBuffer(GL_ARRAY_BUFFER);

		Syncing fence;
		fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		GPUUploadQueueLookup.emplace(glm::ivec2(x, y), GPUupload(offset, i, fence));
		GPUUploadQueue.emplace_back(glm::ivec2(x, y));
	}

	textureVertexArray.EnableVertexAttribArray(0);
	textureVertexArray.VertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(TextureData), (void*)0);
	textureVertexArray.VertexAttribDivisor(0, 1);

	isInitialized = true;
}

void TilemapRender::update()
{
	updateCanvasEdit();
	updateInput();
}

void TilemapRender::draw()
{
	glEnable(GL_CULL_FACE);

	for (u32 i = 0; i < GPUUploadQueue.size(); i++)
	{
		auto GPUUpload = GPUUploadQueueLookup.find(GPUUploadQueue[i]);

		if (GPUUpload->second.fence.IsNotSynced()) continue;
		u32 result = GPUUpload->second.fence.ClientWaitSync(0, 0);

		if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
		{
			u32 index = GPUUpload->second.chunkID;
			if (tilemapChunkPointer[index].offset != 0u - 1)
			{
				textureTileData.freeRegion(tilemapChunkPointer[index].offset);
			}
			tilemapChunkPointer[index].offset = GPUUpload->second.offset;
			GPUUpload->second.fence.DeleteSync();
			GPUUploadQueueLookup.erase(GPUUploadQueue[i]);
			GPUUploadQueue[i] = GPUUploadQueue.back();
			GPUUploadQueue.pop_back();

			auto tilemapChunkpointer = tilemapChunkPointerLookup.find(glm::ivec2(tilemapChunkPointer[index].x, tilemapChunkPointer[index].y));
			if (tilemapChunkpointer == tilemapChunkPointerLookup.end()) return;
			tilemapChunkpointer->second.offset = tilemapChunkPointer[index].offset;
		}
	}

	if (isInitialized)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		shader.bind();

		glDisable(GL_CULL_FACE);
		shader.SetMatrix4("viewAndProjection", camera.getViewAndProjection());

		const Frustum camFrustum = createFrustumFromCamera(camera, window.getWindowSize().x / window.getWindowSize().y, glm::radians(camera.Fov), 0.1f, 100000.0f);
		for (i32 i = 0; i < tilemapChunkPointer.size(); i++)
		{
			if (tilemapChunkPointer[i].offset == 0u - 1) continue;

			i32 x = tilemapChunkPointer[i].x;
			i32 y = tilemapChunkPointer[i].y;

			if (isAABBOnFrustum(setAABB({ x * CHUNK_SIZE, y * CHUNK_SIZE, 0 }, { (x + 1) * CHUNK_SIZE, (y + 1) * CHUNK_SIZE, 0 }), camFrustum))
			{
				textureVertexArray.VertexAttribI1i(1, x);
				textureVertexArray.VertexAttribI1i(2, y);
				textureVertexArray.VertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(TextureData), (void*)(tilemapChunkPointer[i].offset));
				textureVertexArray.DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, CHUNK_SIZE_SQUARED);
			}
		}
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Editor Tools", &my_tool_active, ImGuiWindowFlags_MenuBar);
	if (ImGui::ColorPicker3("Color", colorSelected))
	{
		tileActionType = 3;
	}
	if (ImGui::Button("Rotate", ImVec2(50.f, 25.f)))
	{
		tileActionType = 1;
	}
	if (ImGui::Button("Expand", ImVec2(50.f, 25.f)))
	{
		tileActionType = 2;
	}
	if (ImGui::Button("Remove", ImVec2(50.f, 25.f)))
	{
		tileActionType = 4;
	}

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Open"))
			{
				if (ImGui::InputText("tilemap data name", tilemapDataName, IM_ARRAYSIZE(tilemapDataName)))
				{
					openTilemapFile(tilemapDataName);
					if (isInitialized) sliceTextureAtlas(textureAtlasName);
				}

				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				saveTilemap();
			}
			if (ImGui::BeginMenu("New"))
			{
				ImGui::InputText("Texture Atlas Name", textureAtlasName, IM_ARRAYSIZE(textureAtlasName));
				ImGui::InputText("tilemap Name", tilemapDataName, IM_ARRAYSIZE(tilemapDataName));
				ImGui::InputInt2("tile size", tileWidthAndHeight);
				ImGui::InputInt2("tilemap chunk size", tilemapChunkSizeImGUI);
				if (ImGui::Button("Create"))
				{
					newTilemapFile(tilemapDataName, tilemapChunkSizeImGUI[0], tilemapChunkSizeImGUI[1]);
					sliceTextureAtlas(textureAtlasName);
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	ImGui::End();

	ImGui::Begin("Texture atlas");

	ImVec2 imagePos = ImGui::GetCursorScreenPos();
	ImVec2 imageSize = { width * 32.0f, height * 32.0f };

	if(texture)ImGui::Image((ImTextureID)(intptr_t)texture, imageSize);

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		glm::vec2 mouse = window.getMousePosition();

		glm::ivec2 tile;

		tile.x = (i32)(mouse.x - imagePos.x) / 32.0f;
		tile.y = (i32)(mouse.y - imagePos.y) / 32.0f;

		textureSelect = tile.x + tile.y * width;
		tileActionType = 0;
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}