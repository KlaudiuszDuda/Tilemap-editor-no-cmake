#pragma once

#include "types.h"
#include "BufferObject.h"
#include <utility>
#include <GL/glew.h>
#include <vector>
#include <algorithm>

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
    inline T* allocate_initialized(u64 capacity, const T& initializedTo)
    {
        T* data = (T*)::operator new(capacity * sizeof(T));

        if constexpr (std::is_trivial_v<T>)
        {
            std::memset(data, initializedTo, capacity * sizeof(T));
        }
        else
        {
            for (u32 i = 0; i < capacity; ++i)
                new (&data[i]) T(initializedTo);
        }

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

    class ChunkBufferAllocator {
    public:
        struct Block {
            u32 offset;
            u32 size;
        };

        inline void init(u32 bufferSize)
        {
            m_BufferSize = bufferSize;
            m_Buffer.BindBuffer(GL_ARRAY_BUFFER, 0);
            m_Buffer.BufferData(GL_ARRAY_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
    
            m_FreeBlocks.push_back({ 0, m_BufferSize });
        }

        inline Buffer<1>& getBuffer()
        {
            return m_Buffer;
        }
        
        // allocate a region of `size` bytes
        // returns offset or (size_t)-1 on failure
        inline u32 alloc(void* dst, void const* src, u32 size) {
            for (u32 i = 0; i < m_FreeBlocks.size(); ++i) {
                Block& b = m_FreeBlocks[i];
                if (b.size >= size) {
                    u32 offset = b.offset;
                    b.offset += size;
                    b.size -= size;
    
                    if (b.size == 0) {
                        m_FreeBlocks.erase(m_FreeBlocks.begin() + i);
                    }

                    memcpy((u8*)dst + offset, src , size);
    
                    return offset;
                }
            }

            return growAndAlloc(dst, src, size);
        }

        inline void freeRegion(Block& block) {
            if (block.offset == 0u - 1) { return; }
            m_FreeBlocks.push_back({ block.offset, block.size });
            mergeFreeBlocks();
        }


    private:
        Buffer<1> m_Buffer;
        u32 m_BufferSize;
        std::vector<Block> m_FreeBlocks;
    
        // merge adjacent free blocks
        void mergeFreeBlocks() {
            std::sort(m_FreeBlocks.begin(), m_FreeBlocks.end(),
                [](auto& a, auto& b) { return a.offset < b.offset; });
    
            for (u32 i = 0; i + 1 < m_FreeBlocks.size();) {
                if (m_FreeBlocks[i].offset + m_FreeBlocks[i].size
                    == m_FreeBlocks[i + 1].offset) {
                    m_FreeBlocks[i].size += m_FreeBlocks[i + 1].size;
                    m_FreeBlocks.erase(m_FreeBlocks.begin() + i + 1);
                }
                else {
                    ++i;
                }
            }
        }

        u32 growAndAlloc(void* dst, void const* src, u32 size) {
            u32 oldSize = m_BufferSize;
            u32 newSize = m_BufferSize * 2;
    
            while (newSize - oldSize < size) {
                newSize *= 2;
            }

            m_Buffer.BindBuffer(GL_ARRAY_BUFFER, 0);

            void* temp = malloc(oldSize);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, oldSize, temp);
    
            glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, oldSize, temp);
            free(temp);

            m_FreeBlocks.push_back({ oldSize, newSize - oldSize });
            m_BufferSize = newSize;
    
            mergeFreeBlocks();
    
            return alloc(dst, src, size);
        }
    };
}