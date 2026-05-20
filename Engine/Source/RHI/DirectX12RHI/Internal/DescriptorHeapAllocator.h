#pragma once
#include <cstdint>
#include <mutex>
#include <array>
#include <stdexcept>

namespace RHI
{
    template<uint32_t MaxDescriptors>
    class RHIDescriptorHeapAllocator
    {
    public:
        void Init()
        {
            m_NextIndex = 0;
            m_FreeCount = 0;
            // std::array 自动清零
        }
        
        uint32_t Allocate()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            if (m_FreeCount > 0)
            {
                return m_FreeIndices[--m_FreeCount];
            }
            
            if (m_NextIndex >= MaxDescriptors)
            {
                throw std::runtime_error("Descriptor heap full!");
            }
            return m_NextIndex++;
        }
        
        void Free(uint32_t index)
        {
            if (index >= MaxDescriptors) return; 
            
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_FreeCount < MaxDescriptors)
            {
                m_FreeIndices[m_FreeCount++] = index;
            }
        }
        
        uint32_t GetUsedCount() const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_NextIndex - m_FreeCount;
        }
        
        uint32_t GetFreeCount() const
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_FreeCount;
        }
        
        void Reset()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_NextIndex = 0;
            m_FreeCount = 0;
        }
        
    private:
        mutable std::mutex m_Mutex;
        uint32_t m_NextIndex = 0;
        uint32_t m_FreeCount = 0;
        std::array<uint32_t, MaxDescriptors> m_FreeIndices{};  // 存储空闲索引
    };
}
