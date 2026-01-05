#pragma once

#include "renderer/shader_type.h"

namespace penumbra
{
	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		R8,
		RGBA8,
		RGBA16F,
		RGBA32F,
		RED_INTEGER,

		// Depth/Stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureSpecification_t
	{
		FramebufferTextureSpecification_t() = default;
		FramebufferTextureSpecification_t(FramebufferTextureFormat format)
			: m_TextureFormat(format) {
		}

		FramebufferTextureFormat m_TextureFormat = FramebufferTextureFormat::None;
		// TO-DO: Filtering/Wrap
	};

	struct FramebufferAttachmentSpecification_t
	{
		FramebufferAttachmentSpecification_t() = default;
		FramebufferAttachmentSpecification_t(const std::initializer_list<FramebufferTextureSpecification_t> attachments)
			: m_vAttachments(attachments) {
		}

		std::vector<FramebufferTextureSpecification_t> m_vAttachments;
	};

	struct FramebufferSpecification_t
	{
		uint32_t m_nWidth = 0, m_nHeight = 0;
		glm::vec4 m_ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		FramebufferAttachmentSpecification_t m_Attachments;
		uint32_t m_nSamples = 1;

		// TO-DO: Temp, needs scale
		bool m_bNoResize = false;

		bool m_bSwapChainTarget = false;
	};

	class CFramebuffer
	{
	public:
		CFramebuffer(const FramebufferSpecification_t& spec);
		~CFramebuffer() = default;

		void Bind() const;
		void Unbind() const;

		void Resize(uint32_t nWidth, uint32_t nHeight);
		int ReadPixel(uint32_t nAttachmentIndex, int nX, int nY) const;

		void BindTexture(uint32_t nAttachmentIndex, uint32_t nSlot = 0) const;
		void BindDepthTexture(uint32_t nSlot = 0, ShaderType stage = ShaderType::SHADER_TYPE_PIXEL) const;
		void UnbindTexture(uint32_t nSlot = 0, ShaderType stage = ShaderType::SHADER_TYPE_PIXEL) const;

		uint32_t GetWidth() const { return m_Specification.m_nWidth; }
		uint32_t GetHeight() const { return m_Specification.m_nHeight; }

		void ClearAttachment(uint32_t nAttachmentIndex, int nValue) const;

		intptr_t GetColorAttachmentRendererID(uint32_t nIndex = 0) const { return reinterpret_cast<intptr_t>(m_vspSRVs[nIndex].Get()); }
		intptr_t GetDepthAttachmentRendererID() const { return reinterpret_cast<intptr_t>(m_spDepthTexture.Get()); }

		const FramebufferSpecification_t& GetSpecification() const { return m_Specification; };
		FramebufferSpecification_t& GetSpecification() { return m_Specification; };
	private:
		void Invalidate();
		void ReleaseResources();
	private:
		FramebufferSpecification_t m_Specification;

		ComPtr<ID3D11Device> m_spDevice;
		ComPtr<ID3D11DeviceContext> m_spContext;

		std::vector<ComPtr<ID3D11Texture2D>> m_vspTextures;
		std::vector<ComPtr<ID3D11RenderTargetView>> m_vspRTVs;
		std::vector<ComPtr<ID3D11ShaderResourceView>> m_vspSRVs;

		ComPtr<ID3D11Texture2D> m_spDepthTexture;
		ComPtr<ID3D11ShaderResourceView> m_spDepthSRV;
		ComPtr<ID3D11DepthStencilView> m_spDepthDSV;
	};
}