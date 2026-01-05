#include "ppch.h"
#include "renderer/render_command_queue.h"

namespace penumbra
{
	CRenderCommandQueue::CRenderCommandQueue()
	{
		PENUMBRA_PROFILE_FUNC();

		m_CommandBuffer = new uint8_t[kRenderCommandQueueBufferSize];
		m_CommandBufferPtr = m_CommandBuffer;
		memset(m_CommandBuffer, 0, kRenderCommandQueueBufferSize);
	}

	CRenderCommandQueue::~CRenderCommandQueue()
	{
		PENUMBRA_PROFILE_FUNC();

		delete[] m_CommandBuffer;
	}

	void* CRenderCommandQueue::Allocate(RenderCommandFn fn, uint32_t size)
	{
		PENUMBRA_PROFILE_FUNC();

		// TODO: alignment
		*(RenderCommandFn*)m_CommandBufferPtr = fn;
		m_CommandBufferPtr += sizeof(RenderCommandFn);

		*(uint32_t*)m_CommandBufferPtr = size;
		m_CommandBufferPtr += sizeof(uint32_t);

		void* memory = m_CommandBufferPtr;
		m_CommandBufferPtr += size;

		m_CommandCount++;
		return memory;
	}

	void CRenderCommandQueue::Execute()
	{
		PENUMBRA_PROFILE_FUNC();

		std::byte* buffer = reinterpret_cast<std::byte*>(m_CommandBuffer);

		for (uint32_t i = 0; i < m_CommandCount; i++)
		{
			RenderCommandFn function = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);

			uint32_t size = *(uint32_t*)buffer;
			buffer += sizeof(uint32_t);
			function(buffer);
			buffer += size;
		}

		m_CommandBufferPtr = m_CommandBuffer;
		m_CommandCount = 0;
	}

}