#pragma once

#include "core/base.h"

#include "renderer/mesh.h"
#include "events/application_event.h"
#include "renderer/state_cache.h"

namespace penumbra
{
	struct RasterizerSpecification_t
	{
		enum class CullMode
		{
			None = 0,
			Front = 1,
			Back = 2,
		};

		enum class FillMode
		{
			Solid = 0,
			Wireframe = 1
		};

		CullMode m_Cull = CullMode::Back;
		FillMode m_Fill = FillMode::Solid;

		bool m_bFrontCounterClockwise = false;
		bool m_bDepthClipEnable = true;
		bool m_bScissorEnable = false;
		bool m_bMultisampleEnable = false;
		bool m_bAntialiasedLineEnable = false;

		int32_t m_nDepthBias = 0;
		float m_flDepthBiasClamp = 0.0f;
		float m_flSlopeScaledDepthBias = 0.0f;
	};

	class CRendererAPI
	{
	public:
		CRendererAPI();
		~CRendererAPI() = default;

		void Init();

		void SetViewport(uint32_t nX, uint32_t nY, uint32_t nWidth, uint32_t nHeight);
		void SetRasterizerState(const RasterizerSpecification_t& spec);
		void SetDepth(bool bEnabled);

		void SetClearColor(const glm::vec4& color);
		void Clear();

		void Bind();
		void DrawIndexed(const Ref<CVertexArray>& spVertexArray, uint32_t nIndexCount = 0);
		void DrawIndexedInstanced(const Ref<CVertexArray>& spVertexArray, uint32_t nInstanceCount, uint32_t nIndexCount = 0);
		void DrawIndexedInstanced(const Ref<CVertexArray>& spVertexArray, uint32_t nInstanceCount, CSubmesh& submesh);
		void DrawLines(const Ref<CVertexArray>& spVertexArray, uint32_t nVertexCount = 0);
		void DrawMesh(const Ref<CMesh>& spMesh, uint32_t nInstanceCount);

		const RasterizerSpecification_t& GetRasterizerSpecification() const { return m_Specification; }
	private:
		void CreateRenderTarget();
		void SetViewport();
		void RequestTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		const uint32_t PrepareIndexedDraw(const Ref<CVertexArray>& spVAO, uint32_t count);

		void PollDebugMessages();
	private:
		RasterizerSpecification_t m_Specification;
		CStateCache<D3D11_RASTERIZER_DESC, ID3D11RasterizerState> m_RasterizerStateCache;
		D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		ComPtr<ID3D11InfoQueue> m_spInfoQueue;

		ComPtr<ID3D11DepthStencilState> m_spDepthStencilState;

		ComPtr<IDXGISwapChain1> m_spSwapChain;
		ComPtr<ID3D11RenderTargetView> m_spRenderTargetView;
		ComPtr<ID3D11DepthStencilView> m_spDepthDSV;
		ComPtr<ID3D11BlendState> m_spBlendState;

		ComPtr<ID3D11DeviceContext> m_spContext;
		ComPtr<ID3D11Device> m_spDevice;

		FLOAT m_flBackBufferColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		uint32_t m_nTopLeftX = 0, m_nTopLeftY = 0;
		uint32_t m_nWindowWidth = 0, m_nWindowHeight = 0;
	};
}