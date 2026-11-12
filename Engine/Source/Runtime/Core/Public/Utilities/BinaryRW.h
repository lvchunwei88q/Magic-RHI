#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

/*
* In future development, we need to handle a lot of byte data (which are often the smallest units of data),
* so to make it easier and clearer to work with this data, we use these classes to manage it efficiently.
*/

// Binary write buffer Responsible for writing data to a binary buffer
class BinaryWriter {
private:
    std::vector<uint8_t> m_Data;
    size_t m_WriteOffset;      // Current Write Position
    size_t m_CurrentSize;      // Currently allocated size

public:
    explicit BinaryWriter(size_t initialSize = 0)
        : m_WriteOffset(0)
        , m_CurrentSize(initialSize) {
        if (initialSize > 0) {
            m_Data.resize(initialSize);
        }
    }

    explicit BinaryWriter(const std::vector<uint8_t>& data)
        : m_Data(data)
        , m_WriteOffset(data.size())
        , m_CurrentSize(data.size()) {}

    explicit BinaryWriter(std::vector<uint8_t>&& data) noexcept
        : m_Data(std::move(data))
        , m_WriteOffset(m_Data.size())
        , m_CurrentSize(m_Data.size()) {}

    // copy construction
    BinaryWriter(const BinaryWriter& other)
        : m_Data(other.m_Data)
        , m_WriteOffset(other.m_WriteOffset)
        , m_CurrentSize(other.m_CurrentSize) {}

    // Note that 'other' can no longer be used afterwards
    BinaryWriter(BinaryWriter&& other) noexcept
        : m_Data(std::move(other.m_Data))
        , m_WriteOffset(other.m_WriteOffset)
        , m_CurrentSize(other.m_CurrentSize) {
        other.m_WriteOffset = 0;
        other.m_CurrentSize = 0;
    }

    BinaryWriter& operator=(const BinaryWriter& other) {
        if (this != &other) {
            m_Data = other.m_Data;
            m_WriteOffset = other.m_WriteOffset;
            m_CurrentSize = other.m_CurrentSize;
        }
        return *this;
    }

    BinaryWriter& operator=(BinaryWriter&& other) noexcept {
        if (this != &other) {
            m_Data = std::move(other.m_Data);
            m_WriteOffset = other.m_WriteOffset;
            m_CurrentSize = other.m_CurrentSize;
            other.m_WriteOffset = 0;
            other.m_CurrentSize = 0;
        }
        return *this;
    }

    // Allocate space and return the write pointer.
    // Any pointer previously returned by Allocate/GetCurrentPtr is invalidated.
    uint8_t* Allocate(size_t size) {
        if (size > SIZE_MAX - m_WriteOffset) {
            throw std::length_error("BinaryWrite::Allocate overflow");
        }
        size_t newSize = m_WriteOffset + size;
        if (newSize > m_CurrentSize) {
            size_t reserved = newSize + newSize / 2 + 1024;
            if (reserved < newSize) {
                reserved = newSize;
            }
            m_Data.resize(reserved);
            m_CurrentSize = m_Data.size();
        }

        uint8_t* ptr = m_Data.data() + m_WriteOffset;
        m_WriteOffset = newSize;
        return ptr;
    }

    template<typename T>
    void Write(const T& value) {
        static_assert(std::is_trivially_copyable<T>::value,
            "T must be trivially copyable");
        uint8_t* ptr = Allocate(sizeof(T));
        std::memcpy(ptr, &value, sizeof(T));
    }

    void Write(const void* data, size_t size) {
        if (size == 0) {
            return;
        }
        if (data == nullptr) {
            throw std::invalid_argument("BinaryWrite::Write null data");
        }
        uint8_t* ptr = Allocate(size);
        std::memcpy(ptr, data, size);
    }

    void Write(const std::vector<uint8_t>& data) {
        Write(data.data(), data.size());
    }

    // Get current write pointer
    const uint8_t* GetCurrentPtr()  const { return m_Data.data() + m_WriteOffset; }
    // Get data pointer
    const uint8_t* DataU8()         const { return m_Data.data(); }
    const char*    DataChar()       const { return reinterpret_cast<const char*>(m_Data.data()); }
    // Get the size written so far
    size_t GetWrittenSize()         const { return m_WriteOffset; }
    // Get the total size currently allocated
    size_t GetTotalSize()           const { return m_CurrentSize; }

    // Manually set write position (throws if out of allocated range)
    void Seek(size_t position) {
        if (position > m_CurrentSize) {
            throw std::out_of_range("BinaryWrite::Seek out of range");
        }
        m_WriteOffset = position;
    }
    // Get Current Offset
    size_t Tell() const { return m_WriteOffset; }

    // Shrink to actual size
    void ShrinkToFit() {
        if (m_Data.size() > m_WriteOffset) {
            m_Data.resize(m_WriteOffset);
            m_Data.shrink_to_fit();
            m_CurrentSize = m_WriteOffset;
        }
    }

    // Get vector
    std::vector<uint8_t> MoveData() {
        ShrinkToFit();
        std::vector<uint8_t> out = std::move(m_Data);
        m_Data.clear();
        m_WriteOffset = 0;
        m_CurrentSize = 0;
        return out;
    }

    // Reset
    void Reset() {
        m_Data.clear();
        m_WriteOffset = 0;
        m_CurrentSize = 0;
    }

    // Reset and reassign
    void Reset(size_t newSize) {
        m_Data.assign(newSize, 0);
        m_WriteOffset = 0;
        m_CurrentSize = newSize;
    }
};

// Binary read buffer Responsible for reading data from a binary buffer
class BinaryReader {
private:
    std::vector<uint8_t> m_Data;    // Owned byte storage (deep copied)
    size_t m_ReadOffset;            // Current read position
    size_t m_DataSize;              // data size

public:
    // construct from vector (copy)
    explicit BinaryReader(const std::vector<uint8_t>& data)
        : m_Data(data)
        , m_ReadOffset(0)
        , m_DataSize(m_Data.size()) {}

    // construct from vector (move)
    explicit BinaryReader(std::vector<uint8_t>&& data) noexcept
        : m_Data(std::move(data))
        , m_ReadOffset(0)
        , m_DataSize(m_Data.size()) {}

    // construct from BinaryWriter (copy written bytes only)
    explicit BinaryReader(const BinaryWriter& buffer)
        : m_ReadOffset(0)
        , m_DataSize(buffer.GetWrittenSize()) {
        if (m_DataSize > 0) {
            const uint8_t* src = buffer.DataU8();
            m_Data.assign(src, src + m_DataSize);
        }
    }

    // copy construction (deep copy)
    BinaryReader(const BinaryReader& other)
        : m_Data(other.m_Data)
        , m_ReadOffset(other.m_ReadOffset)
        , m_DataSize(other.m_DataSize) {}

    // Note that 'other' can no longer be used afterwards
    BinaryReader(BinaryReader&& other) noexcept
        : m_Data(std::move(other.m_Data))
        , m_ReadOffset(other.m_ReadOffset)
        , m_DataSize(other.m_DataSize) {
        other.m_ReadOffset = 0;
        other.m_DataSize = 0;
    }

    BinaryReader& operator=(const BinaryReader& other) {
        if (this != &other) {
            m_Data = other.m_Data;
            m_ReadOffset = other.m_ReadOffset;
            m_DataSize = other.m_DataSize;
        }
        return *this;
    }

    BinaryReader& operator=(BinaryReader&& other) noexcept {
        if (this != &other) {
            m_Data = std::move(other.m_Data);
            m_ReadOffset = other.m_ReadOffset;
            m_DataSize = other.m_DataSize;
            other.m_ReadOffset = 0;
            other.m_DataSize = 0;
        }
        return *this;
    }

    ~BinaryReader() = default;

    // read base type
    template<typename T>
    T Read() {
        static_assert(std::is_trivially_copyable<T>::value,
            "T must be trivially copyable");
        if (sizeof(T) > m_DataSize - m_ReadOffset) {
            throw std::out_of_range("Not enough data to read");
        }
        T value;
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        std::memcpy(&value, src, sizeof(T));
        m_ReadOffset += sizeof(T);
        return value;
    }

    // read to a new vector (byteCount bytes)
    std::vector<uint8_t> ReadToVector(size_t byteCount) {
        if (byteCount > m_DataSize - m_ReadOffset) {
            throw std::out_of_range("Not enough data to read");
        }
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        std::vector<uint8_t> result(src, src + byteCount);
        m_ReadOffset += byteCount;
        return result;
    }

    // read and append to an existing vector (byteCount bytes)
    void ReadToVector(std::vector<uint8_t>& out, size_t byteCount) {
        if (byteCount > m_DataSize - m_ReadOffset) {
            throw std::out_of_range("Not enough data to read");
        }
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        out.insert(out.end(), src, src + byteCount);
        m_ReadOffset += byteCount;
    }

    // Read a specified number of bytes into the buffer
    void ReadBytes(void* buffer, size_t byteCount) {
        if (byteCount == 0) {
            return;
        }
        if (buffer == nullptr) {
            throw std::invalid_argument("BinaryReader::ReadBytes null buffer");
        }
        if (byteCount > m_DataSize - m_ReadOffset) {
            throw std::out_of_range("Not enough data to read");
        }
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        std::memcpy(buffer, src, byteCount);
        m_ReadOffset += byteCount;
    }

    // Read string
    std::string ReadString(size_t length) {
        if (length > m_DataSize - m_ReadOffset) {
            throw std::out_of_range("String data truncated");
        }
        const char* src = reinterpret_cast<const char*>(m_Data.data() + m_ReadOffset);
        std::string result(src, length);
        m_ReadOffset += length;
        return result;
    }

    // Peek data
    template<typename T>
    T Peek() const {
        static_assert(std::is_trivially_copyable<T>::value,
            "T must be trivially copyable");
        if (sizeof(T) > m_DataSize - m_ReadOffset) {
            throw std::out_of_range("Not enough data to peek");
        }
        T value;
        const uint8_t* src = m_Data.data() + m_ReadOffset;
        std::memcpy(&value, src, sizeof(T));
        return value;
    }

    // Skip bytes
    void Skip(size_t byteCount) {
        if (byteCount > m_DataSize - m_ReadOffset) {
            throw std::out_of_range("Skip beyond data end");
        }
        m_ReadOffset += byteCount;
    }

    // get current read position
    size_t Tell() const { return m_ReadOffset; }

    // set read position
    void Seek(size_t position) {
        if (position > m_DataSize) {
            throw std::out_of_range("Seek position out of range");
        }
        m_ReadOffset = position;
    }

    size_t Remaining()          const { return m_DataSize - m_ReadOffset; }
    size_t Size()               const { return m_DataSize; }
    bool   IsEOF()              const { return m_ReadOffset >= m_DataSize; }
    const uint8_t* DataU8()     const { return m_Data.data(); }
    const char*    DataChar()   const { return reinterpret_cast<const char*>(m_Data.data()); }
    const uint8_t* CurrentPtr() const { return m_Data.data() + m_ReadOffset; }

    // Reset read position
    void Reset() { m_ReadOffset = 0; }

    // Reset and set new data
    void Reset(const std::vector<uint8_t>& data) {
        m_Data = data;
        m_ReadOffset = 0;
        m_DataSize = m_Data.size();
    }

    void Reset(std::vector<uint8_t>&& data) noexcept {
        m_Data = std::move(data);
        m_ReadOffset = 0;
        m_DataSize = m_Data.size();
    }
};