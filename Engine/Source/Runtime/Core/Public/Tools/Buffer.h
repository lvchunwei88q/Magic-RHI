#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <stdexcept>

/*
* In future development, we need to handle a lot of byte data (which are often the smallest units of data),
* so to make it easier and clearer to work with this data, we use these classes to manage it efficiently.
*/

class BinaryWrite {
private:
    std::vector<uint8_t> m_Data;
    size_t m_WriteOffset;      // Current Write Postion
    size_t m_CurrentSize;      // 当前分配的大小

public:
    explicit BinaryWrite(size_t initialSize = 0)
        : m_WriteOffset(0)
        , m_CurrentSize(initialSize) {
        if (initialSize > 0) {
            m_Data.resize(initialSize);
        }
    }

    explicit BinaryWrite(const std::vector<uint8_t>& data)
        : m_WriteOffset(0)
        , m_CurrentSize(data.size()) {
        if (data.size() > 0) {
            m_Data = data;
        }
    }

    BinaryWrite(const BinaryWrite& other)
        : m_Data(other.m_Data)      
        , m_WriteOffset(other.m_WriteOffset)
        , m_CurrentSize(other.m_CurrentSize) {
    }

    BinaryWrite& operator=(const BinaryWrite& other) {
        if (this != &other) {  // 防止自赋值
            m_Data = other.m_Data;
            m_WriteOffset = other.m_WriteOffset;
            m_CurrentSize = other.m_CurrentSize;
        }
        return *this;
    }

    // 分配空间并返回写入指针
    uint8_t* Allocate(size_t size) {
        // 检查是否需要扩展
        size_t newSize = m_WriteOffset + size;
        if (newSize > m_CurrentSize) {
            // 扩展空间
            size_t reserved = static_cast<size_t>(newSize * 1.5 + 1024);
            m_Data.resize(reserved);
            m_CurrentSize = m_Data.size();
        }

        // 获取当前写入位置的指针
        uint8_t* ptr = m_Data.data() + m_WriteOffset;
        m_WriteOffset += size;

        return ptr;
    }

    // 写入数据
    template<typename T>
    void Write(const T& value) {
        uint8_t* ptr = Allocate(sizeof(T));
        memcpy(ptr, &value, sizeof(T));
    }

    void Write(const void* data, size_t size) {
        uint8_t* ptr = Allocate(size);
        memcpy(ptr, data, size);
    }

    void Write(const std::vector<uint8_t>& data) {Write(data.data(), data.size());}
    // 获取当前写入指针
    const uint8_t* GetCurrentPtr()  const {return m_Data.data() + m_WriteOffset;}
    // 获取数据指针
    const uint8_t* DataU8()         const {return m_Data.data();}
    const char* DataChar()          const {return reinterpret_cast<const char*>(m_Data.data());}
    // 获取当前已写入的大小
    size_t GetWrittenSize()         const {return m_WriteOffset;}
    // 获取当前分配的总大小
    size_t GetTotalSize()           const {return m_CurrentSize;}
    // 获取实际数据大小
    size_t GetUsedSize()            const {return m_WriteOffset;}

    // 手动设置写入位置
    void Seek(size_t position)      { if (position <= m_CurrentSize)m_WriteOffset = position; }
    // 获取当前偏移
    size_t Tell()                   const { return m_WriteOffset; }

    // 收缩到实际使用大小
    void ShrinkToFit() {
        if (m_Data.size() > m_WriteOffset) {
            m_Data.resize(m_WriteOffset);
            m_CurrentSize = m_WriteOffset;
        }
    }

    // 获取底层 vector
    std::vector<uint8_t> MoveData() {
        ShrinkToFit();
        return std::move(m_Data);
    }

    // 重置
    void Reset() {
        m_Data.clear();
        m_WriteOffset = 0;
        m_CurrentSize = 0;
    }

    // 重置并重新分配
    void Reset(size_t newSize) {
        m_Data.clear();
        m_Data.resize(newSize);
        m_WriteOffset = 0;
        m_CurrentSize = newSize;
    }
};

class BinaryReader {
private:
    std::vector<uint8_t> m_Data;                // 指向外部数据
    size_t m_ReadOffset;                        // 当前读取位置
    size_t m_DataSize;                          // 数据总大小

public:
    // 从 vector 构造
    explicit BinaryReader(const std::vector<uint8_t>& data)
        : m_Data(data)
        , m_ReadOffset(0)
        , m_DataSize(data.size()) {}

    // 从 BinaryBuffer 读取
    explicit BinaryReader(const BinaryWrite& buffer)
        : m_ReadOffset(0)
        , m_DataSize(buffer.GetUsedSize()) {
        // copy data to this
        const uint8_t* src = buffer.DataU8();
        m_Data = std::vector<uint8_t>(src, src + m_DataSize);
    }

    // 拷贝构造
    BinaryReader(const BinaryReader& other)
        : m_Data(other.m_Data)  // 共享数据
        , m_ReadOffset(other.m_ReadOffset)
        , m_DataSize(other.m_DataSize) {}

    ~BinaryReader() {}

    // 读取基础类型
    template<typename T>
    T Read() {
        static_assert(std::is_trivially_copyable<T>::value,
            "T must be trivially copyable");

        if (m_ReadOffset + sizeof(T) > m_DataSize) {
            throw std::out_of_range("Not enough data to read");
        }

        T value;
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        memcpy(&value, src, sizeof(T));
        m_ReadOffset += sizeof(T);
        return value;
    }

    // read to vector input vector count
    std::vector<uint8_t> ReadToVector(size_t count) {
        if (m_ReadOffset + count > m_DataSize) {
            throw std::out_of_range("Not enough data to read");
        }

        const uint8_t* src = m_Data.data() + m_ReadOffset;
        std::vector<uint8_t> result(src, src + count);
        m_ReadOffset += count;
        return result;
    }

    // read to vector add
    void ReadToVector(std::vector<uint8_t>& out, size_t count) {
        if (m_ReadOffset + count > m_DataSize) {
            throw std::out_of_range("Not enough data to read");
        }

        const uint8_t* src = m_Data.data() + m_ReadOffset;
        out.insert(out.end(), src, src + count);
        m_ReadOffset += count;
    }

    // 读取指定数量的字节到缓冲区
    void ReadBytes(void* buffer, size_t count) {
        if (m_ReadOffset + count > m_DataSize) {
            throw std::out_of_range("Not enough data to read");
        }

        const uint8_t* src = m_Data.data() + m_ReadOffset;
        memcpy(buffer, src, count);
        m_ReadOffset += count;
    }

    // 读取字符串
    std::string ReadString(size_t length) {
        if (m_ReadOffset + length > m_DataSize) {
            throw std::out_of_range("String data truncated");
        }

        const char* src = reinterpret_cast<const char*>(m_Data.data() + m_ReadOffset);
        std::string result(src, length);
        m_ReadOffset += length;
        return result;
    }

    // 查看数据
    template<typename T>
    T Peek() const {
        static_assert(std::is_trivially_copyable<T>::value,
            "T must be trivially copyable");

        if (m_ReadOffset + sizeof(T) > m_DataSize)
            throw std::out_of_range("Not enough data to peek");

        T value;
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        memcpy(&value, src, sizeof(T));
        return value;
    }

    // 跳过指定字节数
    void Skip(size_t count) {
        if (m_ReadOffset + count > m_DataSize)
            throw std::out_of_range("Skip beyond data end");
        m_ReadOffset += count;
    }

    // 获取当前读取位置
    size_t Tell() const {return m_ReadOffset;}

    // 设置读取位置
    void Seek(size_t position) {
        if (position > m_DataSize)
            throw std::out_of_range("Seek position out of range");
        m_ReadOffset = position;
    }

    // 获取剩余可读字节数
    size_t Remaining()          const {return m_DataSize - m_ReadOffset;}
    // 获取数据总大小
    size_t Size()               const {return m_DataSize;}
    // 判断是否已读完
    bool IsEOF()                const {return m_ReadOffset >= m_DataSize;}
    // 获取数据指针
    const uint8_t* Data()       const {return m_Data.data();}
    // 获取当前指针位置
    const uint8_t* CurrentPtr() const {return m_Data.data() + m_ReadOffset;}
    // 重置读取位置
    void Reset()                {m_ReadOffset = 0;}

    // 重置并设置新数据
    void Reset(const std::vector<uint8_t>& data) {
        m_Data = data;
        m_ReadOffset = 0;
        m_DataSize = data.size();
    }
};