#pragma once

#include "core/base.h"

#include "renderer/framebuffer.h"

namespace penumbra
{
	struct RenderPassSpecification_t
	{
		Ref<CFramebuffer> m_spTargetFramebuffer;
	};

	class CRenderPass
	{
	public:
		CRenderPass(const RenderPassSpecification_t& spec);
		~CRenderPass() = default;

		RenderPassSpecification_t& GetSpecification() { return m_Specification; }
		const RenderPassSpecification_t& GetSpecification() const { return m_Specification; }
	private:
		RenderPassSpecification_t m_Specification;
	};
}