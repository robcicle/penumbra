#pragma once

namespace penumbra
{
	// Non-owning raw buffer class
	struct Buffer_t
	{
		uint8_t* m_pData = nullptr;
		uint64_t m_nSize = 0;

		Buffer_t() = default;

		Buffer_t(uint64_t nSize)
		{
			Allocate(nSize);
		}

		Buffer_t(const void* pData, uint64_t nSize)
			: m_pData((uint8_t*)pData), m_nSize(nSize)
		{
		}

		Buffer_t(const Buffer_t&) = default;

		static Buffer_t Copy(Buffer_t other)
		{
			Buffer_t result(other.m_nSize);
			memcpy(result.m_pData, other.m_pData, other.m_nSize);
			return result;
		}

		static Buffer_t Copy(const void* data, uint32_t size)
		{
			Buffer_t buffer;
			buffer.Allocate(size);
			memcpy(buffer.m_pData, data, size);
			return buffer;
		}

		static Buffer_t Copy(void* data, uint32_t size)
		{
			Buffer_t buffer;
			buffer.Allocate(size);
			memcpy(buffer.m_pData, data, size);
			return buffer;
		}

		void Allocate(uint64_t nSize)
		{
			Release();

			m_pData = (uint8_t*)malloc(nSize);
			m_nSize = nSize;
		}

		void Release()
		{
			free(m_pData);
			m_pData = nullptr;
			m_nSize = 0;
		}

		template<typename T>
		T* As()
		{
			return (T*)m_pData;
		}

		operator bool() const
		{
			return (bool)m_pData;
		}

	};

	struct ScopedBuffer_t
	{
		ScopedBuffer_t(Buffer_t buffer)
			: m_Buffer(buffer)
		{
		}

		ScopedBuffer_t(uint64_t size)
			: m_Buffer(size)
		{
		}

		~ScopedBuffer_t()
		{
			m_Buffer.Release();
		}

		uint8_t* Data() { return m_Buffer.m_pData; }
		uint64_t Size() { return m_Buffer.m_nSize; }

		template<typename T>
		T* As()
		{
			return m_Buffer.As<T>();
		}

		operator bool() const { return m_Buffer; }
	private:
		Buffer_t m_Buffer;
	};
}