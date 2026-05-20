#pragma once

#include "types.h"
#include "BufferObject.h"
#include <utility>
#include <GL/glew.h>
#include <vector>
#include <algorithm>
#include "Syncing.h"

namespace kl
{
    template<typename T>
    inline T* alloc(u64 capacity)
    {
        T* data = (T*)::operator new(capacity * sizeof(T));

        if constexpr (!std::is_trivially_default_constructible_v<T>)
        {
            for (u32 i = 0; i < capacity; ++i)
                new (&data[i]) T();
        }

        return data;
    }
    inline void* alloc(u64 bytes)
    {
        void* data = (u8*)::operator new(bytes);
        return data;
    }

    template<typename T>
    inline T* allocate_zeroed(u64 capacity)
    {
        T* data = (T*)::operator new(capacity * sizeof(T));

        if constexpr (std::is_trivial_v<T>)
        {
            std::memset(data, 0, capacity * sizeof(T));
        }
        else
        {
            for (u32 i = 0; i < capacity; ++i)
                new (&data[i]) T();
        }

        return data;
    }

    template<typename T>
    inline void dealloc(T* data, u64 size)
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (u32 i = 0; i < size; ++i)
                data[i].~T();
        }

        ::operator delete(data);
    }
    template<typename T>
    inline void dealloc(T* data)
    {
        ::operator delete(data);
    }

    struct Block {
        u32 m_Offset;
        u32 m_Size;

        Block(u32 offset, u32 size)
        {
            m_Offset = offset;
            m_Size = size;
        }
    };

    class ChunkBufferAllocator {
    public:
        inline void init(u32 bufferSize)
        {
            m_BufferSize = bufferSize;
            m_Buffer.BindBuffer(GL_ARRAY_BUFFER, 0);
            m_Buffer.BufferData(GL_ARRAY_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_COPY);
    
            m_FreeBlocks.clear();
            m_FreeBlocks.shrink_to_fit();
            m_FreeBlocks.push_back({ 0, m_BufferSize });
        }

        inline Buffer<1>& getBuffer()
        {
            return m_Buffer;
        }
        
        inline u32 alloc(u32 size) {
            for (u32 i = 0; i < m_FreeBlocks.size(); ++i) {
                Block& b = m_FreeBlocks[i];
                if (b.m_Size >= size) {
                    u32 offset = b.m_Offset;
                    b.m_Offset += size;
                    b.m_Size -= size;
    
                    if (b.m_Size == 0) {
                        m_FreeBlocks.erase(m_FreeBlocks.begin() + i);
                    }
    
                    return offset;
                }
            }

            return growAndAlloc(size);
        }

        inline u32 alloc_init(void* dst, u32 size) {
            for (u32 i = 0; i < m_FreeBlocks.size(); ++i) {
                Block& b = m_FreeBlocks[i];
                if (b.m_Size >= size) {
                    u32 offset = b.m_Offset;
                    b.m_Offset += size;
                    b.m_Size -= size;

                    if (b.m_Size == 0) {
                        m_FreeBlocks.erase(m_FreeBlocks.begin() + i);
                    }

                    memset((u8*)dst + offset, 0, size);

                    return offset;
                }
            }
        }

        inline void freeRegion(Block& block) {
            if (block.m_Offset == 0u - 1) { return; }
            m_FreeBlocks.push_back({ block.m_Offset, block.m_Size });
            mergeFreeBlocks();
        }


    private:
        Buffer<1> m_Buffer;
        u32 m_BufferSize;
        std::vector<Block> m_FreeBlocks;
    
        void mergeFreeBlocks() {
            std::sort(m_FreeBlocks.begin(), m_FreeBlocks.end(),
                [](auto& a, auto& b) { return a.m_Offset < b.m_Offset; });
    
            for (u32 i = 0; i + 1 < m_FreeBlocks.size();) {
                if (m_FreeBlocks[i].m_Offset + m_FreeBlocks[i].m_Size
                    == m_FreeBlocks[i + 1].m_Offset) {
                    m_FreeBlocks[i].m_Size += m_FreeBlocks[i + 1].m_Size;
                    m_FreeBlocks.erase(m_FreeBlocks.begin() + i + 1);
                }
                else {
                    ++i;
                }
            }
        }

        u32 growAndAlloc(u32 size) {
            u32 oldSize = m_BufferSize;
            u32 newSize = m_BufferSize * 2;
    
            while (newSize - oldSize < size) {
                newSize *= 2;
            }

            m_Buffer.BindBuffer(GL_ARRAY_BUFFER, 0);

            void* temp = malloc(oldSize);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, oldSize, temp);
    
            glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_COPY);
            glBufferSubData(GL_ARRAY_BUFFER, 0, oldSize, temp);
            free(temp);

            m_FreeBlocks.push_back({ oldSize, newSize - oldSize });
            m_BufferSize = newSize;
    
            mergeFreeBlocks();
    
            return alloc(size);
        }
    };

    template<u32 size>
    class BucketGPUMemory
    {
    private:
        Buffer<1> m_Buffer;
        u32 m_BufferSize;
        std::vector<u32> m_FreeBlocks;

    public:
        inline void init(u32 BucketAmount)
        {
            m_BufferSize = BucketAmount * size;
            m_Buffer.BindBuffer(GL_ARRAY_BUFFER, 0);
            m_Buffer.BufferData(GL_ARRAY_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_COPY);

            m_FreeBlocks.clear();
            for (i32 i = 0; i < BucketAmount; i++)
            {
                m_FreeBlocks.emplace_back(i * size);
            }
        }

        inline Buffer<1>& getBuffer()
        {
            return m_Buffer;
        }

        inline u32 alloc()
        {
            if (m_FreeBlocks.size())
            {
                u32 offset = m_FreeBlocks[0];
                m_FreeBlocks[0] = m_FreeBlocks.back();
                m_FreeBlocks.pop_back();
                return offset;
            }
        }

        inline u32 alloc_init(void* dst)
        {
            if (m_FreeBlocks.size())
            {
                u32 offset = m_FreeBlocks[0];
                m_FreeBlocks[0] = m_FreeBlocks.back();

                memset((u8*)dst + offset, 0, size);

                return offset;
            }
            return growAndAlloc();
        }

        inline void freeRegion(u32 offset) {
            if (offset == 0u - 1) { return; }
            m_FreeBlocks.emplace_back(offset);
        }

        u32 growAndAlloc() {
            u32 oldSize = m_BufferSize;
            u32 newSize = m_BufferSize * 2;

            while (newSize - oldSize < size) {
                newSize *= 2;
            }

            Buffer<1> newBuffer;
            newBuffer.BindBuffer(GL_ARRAY_BUFFER, 0);
            newBuffer.BufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_COPY);

            m_Buffer.BindBuffer(GL_COPY_READ_BUFFER, 0);
            newBuffer.BindBuffer(GL_COPY_WRITE_BUFFER, 0);

            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, oldSize);

            i32 start = oldSize / size;
            i32 end = newSize / size;

            for (i32 i = start; i < end; i++)
            {
                m_FreeBlocks.emplace_back(i * size);
            }

            return alloc();
        }
    };
}