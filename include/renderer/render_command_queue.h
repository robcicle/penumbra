#pragma once

namespace penumbra
{
	class CRenderCommandQueue
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		CRenderCommandQueue();
		~CRenderCommandQueue();

		void* Allocate(RenderCommandFn func, uint32_t size);

		void Execute();

		bool IsActive() { return m_bIsActive; }
		void Enable () { m_bIsActive = true; }
		void Disable() { m_bIsActive = false; }
		
	private:
		uint8_t* m_CommandBuffer;
		uint8_t* m_CommandBufferPtr;
		uint32_t m_CommandCount = 0;

		bool m_bIsActive = true;
	};
}