#include "TilemapRender.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TilemapRender::TilemapRender()
{
	std::ifstream tilemapData;

	tilemapData.open("resources/tilemap.tmk", std::ios::binary | std::ios::in | std::ios::ate);

	std::streamsize size = tilemapData.tellg();

	if (size == -1)
	{
		tilemapData.close();
		std::ofstream newFile("resources/tilemap.tmk", std::ios::binary);
		tilemapData.open("resources/tilemap.tmk", std::ios::binary | std::ios::in);
		size = tilemapData.tellg();
	}

	if (size == 0)
	{
		tilemapChunkSize.x = 4;
		tilemapChunkSize.y = 4;

		tilemapBuffer.resize(tilemapChunkSize.x * tilemapChunkSize.y * CHUNK_SIZE_SQUARED);
		std::fill(tilemapBuffer.data(), tilemapBuffer.data() + tilemapBuffer.size(), TextureData(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
	}
	else
	{
		tilemapData.seekg(0, std::ios::beg);
		tilemapData.read(reinterpret_cast<char*>(&tilemapChunkSize), sizeof(glm::ivec2));
		tilemapBuffer.resize(tilemapChunkSize.x * tilemapChunkSize.y * CHUNK_SIZE_SQUARED);
		tilemapData.seekg(sizeof(glm::ivec2), std::ios::beg);
		tilemapData.read(reinterpret_cast<char*>(tilemapBuffer.data()), tilemapChunkSize.x * tilemapChunkSize.y * CHUNK_SIZE_SQUARED * sizeof(TextureData));
	}
	tilemapData.close();

	tilemapChunkPointer.resize(tilemapChunkSize.x * tilemapChunkSize.y);
	memset(tilemapChunkPointer.data(), 255, tilemapChunkPointer.size() * sizeof(u32));

	textureVertexArray.bind(0);
	textureTileData.init((tilemapChunkSize.x * tilemapChunkSize.y * 2) * CHUNK_SIZE_SQUARED * sizeof(TextureData));
	GPUUploadQueue.resize(tilemapChunkSize.x * tilemapChunkSize.y);

	auto& buffer = textureTileData.getBuffer();
	u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;


	if (size == 0)
	{
		for (u32 i = 0; i < tilemapChunkSize.x * tilemapChunkSize.y; i++)
		{
			GPUUploadQueue[i].offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(TextureData));
			TextureData* dst = (TextureData*)buffer.MapBufferRange(GL_ARRAY_BUFFER, GPUUploadQueue[i].offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);
			std::fill(dst, dst + CHUNK_SIZE_SQUARED, TextureData(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF));
			GPUUploadQueue[i].fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			buffer.UnmapBuffer(GL_ARRAY_BUFFER);
		}
	}
	else
	{
		for (u32 i = 0; i < tilemapChunkSize.x * tilemapChunkSize.y; i++)
		{
			GPUUploadQueue[i].offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(TextureData));
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, GPUUploadQueue[i].offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);
			memcpy(dst, tilemapBuffer.data() + GPUUploadQueue[i].offset / sizeof(TextureData), CHUNK_SIZE_SQUARED * sizeof(TextureData));
			GPUUploadQueue[i].fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			buffer.UnmapBuffer(GL_ARRAY_BUFFER);

		}
	}

	textureVertexArray.EnableVertexAttribArray(0);
	textureVertexArray.VertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(TextureData), (void*)0);
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
	shader.SetVector2i("tilemapChunkSize", tilemapChunkSize);

	shader.SetInt("TextureAtlasID", 0);
}

TilemapRender::~TilemapRender()
{
	std::ofstream tilemapData;

	tilemapData.open("resources/tilemap.tmk", std::ios::binary | std::ios::out);
	tilemapData.write(reinterpret_cast<char*>(&tilemapChunkSize), sizeof(glm::ivec2));
	tilemapData.seekp(sizeof(glm::ivec2), std::ios::beg);
	tilemapData.write(reinterpret_cast<char*>(tilemapBuffer.data()), tilemapChunkSize.x * tilemapChunkSize.y * CHUNK_SIZE_SQUARED * sizeof(TextureData));
	tilemapData.close();
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
		if (camera.Position.z < 0.1f)
		{
			camera.Position.z = 0.1f;
		}
	}
	if (input.isActionActive(MappedInput::E))
	{
		camera.Position.z += Time.dt * 5;
	}

	if (input.isActionActiveDigital(MappedInput::Z, GLFW_PRESS))
	{
		textureSelect--;
		if (textureSelect < 0)
		{
			textureSelect = height * width;
		}
	}
	if (input.isActionActiveDigital(MappedInput::X, GLFW_PRESS))
	{
		textureSelect++;
		if (textureSelect > height * width)
		{
			textureSelect = 0;
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
	if (input.isActionActiveDigital(MappedInput::C, GLFW_PRESS))
	{
		tileActionType = 3;
	}
}

void TilemapRender::updateCanvasEdit()
{
	if (input.getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_REPEAT)
	{
		glm::ivec3 mouseWorldPosition = window.getWorldMousePosition(camera);
		glm::ivec3 floatMouseWorldPosition = (glm::ivec3)(window.getWorldMousePosition(camera) * glm::vec3(2.f, 2.f, 2.f));
		u32 mouseWorldPositionX = mouseWorldPosition.x;
		u32 mouseWorldPositionY = mouseWorldPosition.y;

		u32 mouseWorldChunkPositionX = mouseWorldPositionX / CHUNK_SIZE;
		u32 mouseWorldChunkPositionY = mouseWorldPositionY / CHUNK_SIZE;

		u32 mouseWorldChunkPositionIndexX = mouseWorldPositionX % CHUNK_SIZE;
		u32 mouseWorldChunkPositionIndexY = mouseWorldPositionY % CHUNK_SIZE;

		u32 mouseWorldChunkIndex = mouseWorldChunkPositionX + mouseWorldChunkPositionY * tilemapChunkSize.x;
		u32 mouseWorldIndex = mouseWorldChunkPositionIndexX + mouseWorldChunkPositionIndexY * CHUNK_SIZE;

		u32 quadVertexIDX = floatMouseWorldPosition.x % 2;
		u32 quadVertexIDY = floatMouseWorldPosition.y % 2;
		u32 quadVertexID = quadVertexIDX + quadVertexIDY * 2;

		if (tileActionType == 0)
		{
			if (mouseWorldChunkPositionX > tilemapChunkSize.x - 1) return;
			if (mouseWorldChunkPositionY > tilemapChunkSize.y - 1) return;
			if (mouseWorldChunkPositionX < 0) return;
			if (mouseWorldChunkPositionY < 0) return;
			if (tilemapBuffer[mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED].textureData == textureSelect) return;

			if (!GPUUploadQueue[mouseWorldChunkIndex].fence.IsNotSynced()) return;

			u32 TileIndex = mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED;
			tilemapBuffer[TileIndex].textureData = textureSelect;

			u32 offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(TextureData));

			auto& buffer = textureTileData.getBuffer();

			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);

			memcpy(dst, tilemapBuffer.data() + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED, CHUNK_SIZE_SQUARED * sizeof(TextureData));

			buffer.UnmapBuffer(GL_ARRAY_BUFFER);

			GPUUploadQueue[mouseWorldChunkIndex].fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			GPUUploadQueue[mouseWorldChunkIndex].offset = offset;
		}
		if (tileActionType == 3)
		{
			if (mouseWorldChunkPositionX > tilemapChunkSize.x - 1) return;
			if (mouseWorldChunkPositionY > tilemapChunkSize.y - 1) return;
			if (mouseWorldChunkPositionX < 0) return;
			if (mouseWorldChunkPositionY < 0) return;

			u32 colorR = (u32)(colorSelected[0] * 255);
			u32 colorG = (u32)(colorSelected[1] * 255);
			u32 colorB = (u32)(colorSelected[2] * 255);

			int shift = quadVertexID * 8;
			u32 TileIndex = mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED;

			u32 R = (tilemapBuffer[TileIndex].R >> shift) & 255u;
			u32 G = (tilemapBuffer[TileIndex].G >> shift) & 255u;
			u32 B = (tilemapBuffer[TileIndex].B >> shift) & 255u;
			if (R == colorR && G == colorG && B == colorB) return;

			if (!GPUUploadQueue[mouseWorldChunkIndex].fence.IsNotSynced()) return;

			tilemapBuffer[TileIndex].R =
				(tilemapBuffer[TileIndex].R & ~(0xFFu << shift)) |
				((colorR) << shift);

			tilemapBuffer[TileIndex].G =
				(tilemapBuffer[TileIndex].G & ~(0xFFu << shift)) |
				((colorG) << shift);

			tilemapBuffer[TileIndex].B =
				(tilemapBuffer[TileIndex].B & ~(0xFFu << shift)) |
				((colorB) << shift);

			u32 red = tilemapBuffer[TileIndex].R >> shift & 255u;
			u32 green = tilemapBuffer[TileIndex].G >> shift & 255u;
			u32 blue = tilemapBuffer[TileIndex].B >> shift & 255u;

			u32 offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(TextureData));

			auto& buffer = textureTileData.getBuffer();

			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);

			memcpy(dst, tilemapBuffer.data() + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED, CHUNK_SIZE_SQUARED * sizeof(TextureData));

			GPUUploadQueue[mouseWorldChunkIndex].fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			GPUUploadQueue[mouseWorldChunkIndex].offset = offset;

			buffer.UnmapBuffer(GL_ARRAY_BUFFER);
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
		
		u32 mouseWorldChunkIndex = mouseWorldChunkPositionX + mouseWorldChunkPositionY * tilemapChunkSize.x;
		u32 mouseWorldIndex = mouseWorldChunkPositionIndexX + mouseWorldChunkPositionIndexY * CHUNK_SIZE;
		
		if (tileActionType == 1)
		{
			if (mouseWorldChunkPositionX > tilemapChunkSize.x - 1) return;
			if (mouseWorldChunkPositionY > tilemapChunkSize.y - 1) return;
			if (mouseWorldChunkPositionX < 0) return;
			if (mouseWorldChunkPositionY < 0) return;

			if (!GPUUploadQueue[mouseWorldChunkIndex].fence.IsNotSynced()) return;
		
			u32 TileIndex = mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED;
			u32 rotation = ((((tilemapBuffer[TileIndex].textureData >> 8) & 3u) + 1) % 4);
			u32 bits = (rotation << 8);
			u32 mask = bits;
			mask |= 255;
			mask |= (1u << 21u) << 10u;
			tilemapBuffer[TileIndex].textureData &= mask;
			tilemapBuffer[TileIndex].textureData |= bits;
		
			u32 offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(TextureData));
		
			auto& buffer = textureTileData.getBuffer();
		
			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);
		
			memcpy(dst, tilemapBuffer.data() + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED, CHUNK_SIZE_SQUARED * sizeof(TextureData));

			GPUUploadQueue[mouseWorldChunkIndex].fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			GPUUploadQueue[mouseWorldChunkIndex].offset = offset;
		
			buffer.UnmapBuffer(GL_ARRAY_BUFFER);
		}
		if (tileActionType == 2)
		{
			if (mouseWorldChunkPositionX > tilemapChunkSize.x - 1) return;
			if (mouseWorldChunkPositionY > tilemapChunkSize.y - 1) return;
			if (mouseWorldChunkPositionX < 0) return;
			if (mouseWorldChunkPositionY < 0) return;

			if (!GPUUploadQueue[mouseWorldChunkIndex].fence.IsNotSynced()) return;
		
			u32 TileIndex = mouseWorldIndex + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED;
		
			u32 flip = ((((tilemapBuffer[TileIndex].textureData >> 10) & 1u) + 1) % 2);
			u32 bits = (flip << 10);
			u32 mask = bits;
			mask |= 1023;
			mask |= (1u << 19u) << 8u;
			tilemapBuffer[TileIndex].textureData &= mask;
			tilemapBuffer[TileIndex].textureData |= bits;
		
			u32 offset = textureTileData.alloc(CHUNK_SIZE_SQUARED * sizeof(TextureData));
		
			auto& buffer = textureTileData.getBuffer();
		
			u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			void* dst = buffer.MapBufferRange(GL_ARRAY_BUFFER, offset, CHUNK_SIZE_SQUARED * sizeof(TextureData), flags);
		
			memcpy(dst, tilemapBuffer.data() + mouseWorldChunkIndex * CHUNK_SIZE_SQUARED, CHUNK_SIZE_SQUARED * sizeof(TextureData));

			GPUUploadQueue[mouseWorldChunkIndex].fence.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			GPUUploadQueue[mouseWorldChunkIndex].offset = offset;
		
			buffer.UnmapBuffer(GL_ARRAY_BUFFER);
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
	for (u32 i = 0; i < GPUUploadQueue.size(); i++)
	{
		if (GPUUploadQueue[i].fence.IsNotSynced()) continue;
		u32 result = GPUUploadQueue[i].fence.ClientWaitSync(0, 0);

		if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
		{
			if (tilemapChunkPointer[i] != 0u - 1)
			{
				kl::Block b = kl::Block(tilemapChunkPointer[i], CHUNK_SIZE_SQUARED * sizeof(TextureData));
				textureTileData.freeRegion(b);
			}
			tilemapChunkPointer[i] = GPUUploadQueue[i].offset;
			GPUUploadQueue[i].fence.DeleteSync();
		}
	}
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
	
	ImGui::Begin("My First Tool", &my_tool_active, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
			if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
			if (ImGui::MenuItem("Close", "Ctrl+W")) { my_tool_active = false; }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	
	ImGui::ColorPicker3("Color", colorSelected);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	shader.bind();

	glDisable(GL_CULL_FACE);
	shader.SetMatrix4("viewAndProjection", camera.getViewAndProjection());

	const Frustum camFrustum = createFrustumFromCamera(camera, window.getWindowSize().x / window.getWindowSize().y, glm::radians(camera.Fov), 0.1f, 100000.0f);
	for (i32 i = 0; i < tilemapChunkPointer.size(); i++)
	{
		i32 x = i % tilemapChunkSize.x;
		i32 y = i / tilemapChunkSize.y;

		if (isAABBOnFrustum(setAABB({ x * CHUNK_SIZE, y * CHUNK_SIZE, 0 }, { (x + 1) * CHUNK_SIZE, (y + 1) * CHUNK_SIZE, 0 }), camFrustum))
		{
			textureVertexArray.VertexAttribI1i(1, x);
			textureVertexArray.VertexAttribI1i(2, y);
			textureVertexArray.VertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(TextureData), (void*)(tilemapChunkPointer[i]));
			textureVertexArray.DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, CHUNK_SIZE_SQUARED);
		}
	}
	glEnable(GL_CULL_FACE);
}