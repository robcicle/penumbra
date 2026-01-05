#include "ppch.h"
#include "renderer/renderer_api.h"

#include "core/application.h"

namespace penumbra
{
	namespace Utils
	{
		static D3D11_CULL_MODE ToD3D11(RasterizerSpecification_t::CullMode mode)
		{
			switch (mode)
			{
			case RasterizerSpecification_t::CullMode::None:  return D3D11_CULL_NONE;
			case RasterizerSpecification_t::CullMode::Front: return D3D11_CULL_FRONT;
			case RasterizerSpecification_t::CullMode::Back:  return D3D11_CULL_BACK;
			}
			PENUMBRA_CORE_ASSERT(false, "RendererAPI: Unknown CullMode!");
			return D3D11_CULL_NONE;
		}

		static D3D11_FILL_MODE ToD3D11(RasterizerSpecification_t::FillMode mode)
		{
			switch (mode)
			{
			case RasterizerSpecification_t::FillMode::Solid:     return D3D11_FILL_SOLID;
			case RasterizerSpecification_t::FillMode::Wireframe: return D3D11_FILL_WIREFRAME;
			}
			PENUMBRA_CORE_ASSERT(false, "RendererAPI: Unknown FillMode!");
			return D3D11_FILL_SOLID;
		}
	}

	CRendererAPI::CRendererAPI()
		: m_RasterizerStateCache(
			[this](const D3D11_RASTERIZER_DESC& desc)
			{
				ComPtr<ID3D11RasterizerState> spState = nullptr;
				PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateRasterizerState");
				HRESULT hr = m_spDevice->CreateRasterizerState(&desc, spState.GetAddressOf());
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "RendererAPI: Failed to create rasterizer state!");
				return spState;
			})
	{
	}

	void CRendererAPI::Init()
	{
		PENUMBRA_PROFILE_FUNC();

		// Get the application window and graphics context
		CWindow& window = CApplication::Get().GetWindow();
		CGraphicsContext* pContext = window.GetGraphicsContext();

		// Device and Context
		m_spDevice = pContext->GetDevice();
		m_spContext = pContext->GetContext();

#ifdef PENUMBRA_DEBUG
		// Debug Info Queue
		CRenderer::Submit([this]()
			{
				ComPtr<ID3D11InfoQueue> info;

				if (SUCCEEDED(m_spDevice->QueryInterface(IID_PPV_ARGS(&info)))) {
					m_spInfoQueue = info;

					// Apply an empty filter; user may customize later
					D3D11_INFO_QUEUE_FILTER filter{};
					info->AddStorageFilterEntries(&filter);

					// Break on warnings and errors
					info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
					info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
					info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
				}
			});
#endif

		// Swap Chain
		auto [width, height] = window.GetWidthHeight();
		HWND hwnd = window.GetHWND();

		bool allowTearing = pContext->GetTearing();

		// Swap Chain description
		DXGI_SWAP_CHAIN_DESC1 scDesc{};
		scDesc.Width = width;	// Use window width
		scDesc.Height = height;	// Use window height
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 8-bit RGBA
		scDesc.SampleDesc = { 1, 0 };				// No multi-sampling
		scDesc.BufferCount = kDefaultSwapChainBufferCount;		// Double buffering
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// Render target output
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// Flip discard
		scDesc.Scaling = DXGI_SCALING_STRETCH;	// Stretch to fit
		scDesc.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		// Fullscreen description
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc{};
		fsDesc.Windowed = !window.GetWindowData().m_bFullscreen;

		// Create Swap Chain
		ComPtr<IDXGIFactory2> spFactory = pContext->GetDXGIFactory();

		CRenderer::Submit([this, spFactory, hwnd, scDesc, fsDesc, pContext]()
			{
				PENUMBRA_PROFILE_SCOPE("IDXGIFactory2::CreateSwapChainForHwnd");

				// Create the swap chain
				HRESULT hr = spFactory->CreateSwapChainForHwnd(
					m_spDevice.Get(),	// Associated device
					hwnd,				// Window handle
					&scDesc,			// Swap chain description
					&fsDesc,			// Fullscreen description
					nullptr,			// No restrict to output
					m_spSwapChain.GetAddressOf()	// Resulting swap chain
				);

				// Ensure swap chain creation succeeded
				PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "RendererAPI: Failed to create swap chain!");
				// Set the swap chain to the context
				pContext->SetCurrentSwapchain(m_spSwapChain);
			});

		// Must set before viewport submission
		SetRasterizerState(m_Specification);

		// Sampler State
		D3D11_SAMPLER_DESC sampler{};
		sampler.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;	// Linear filtering for minification and magnification, point filtering for mipmaps
		// Wrap addressing mode for U, V, and W coordinates
		sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler.ComparisonFunc = D3D11_COMPARISON_ALWAYS;	// Comparison function always passes
		sampler.MaxAnisotropy = 1;							// No anisotropic filtering
		sampler.MaxLOD = D3D11_FLOAT32_MAX;					// Use the maximum level of detail

		CRenderer::Submit([this, sampler]()
			{
				// Create the sampler state
				ComPtr<ID3D11SamplerState> spState;
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateSamplerState");
					HRESULT hr = m_spDevice->CreateSamplerState(&sampler, spState.GetAddressOf());
					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to create sampler state!");
				}

				// Bind the sampler state to the pixel shader stage
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetSamplers");
					m_spContext->PSSetSamplers(0, 1, spState.GetAddressOf());
				}

				// Set default primitive topology
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::IASetPrimitiveTopology");
					m_spContext->IASetPrimitiveTopology(m_PrimitiveTopology);
				}
			});

		// Set the viewport
		SetViewport(0, 0, width, height);
		// Enable depth testing by default
		SetDepth(true);
	}

	void CRendererAPI::SetViewport(uint32_t nX, uint32_t nY, uint32_t nWidth, uint32_t nHeight)
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this, nWidth, nHeight]()
			{
				// Force a present to avoid issues with resizing
				{
					PENUMBRA_PROFILE_SCOPE("IDXGISwapChain1::Present");
					auto& windowData = CApplication::Get().GetWindow().GetWindowData();
					m_spSwapChain->Present(windowData.m_bVSync == true ? 1 : 0, 0);
				}

				// Before resizing, unbind render targets and release references
				ID3D11RenderTargetView* pNullRTV[] = { nullptr };

				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::OMSetRenderTargets");
					m_spContext->OMSetRenderTargets(1, pNullRTV, nullptr);
				}
				m_spRenderTargetView.Reset();
				m_spDepthDSV.Reset();
				m_spBlendState.Reset();

				// Flush any commands that might be referencing the swapchain buffers
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Flush");
					m_spContext->Flush();
				}

				// Resize the swap chain buffers
				{
					auto* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
					UINT resizeFlags = pContext->GetTearing() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
					PENUMBRA_PROFILE_SCOPE("IDXGISwapChain1::ResizeBuffers");
					HRESULT hr = m_spSwapChain->ResizeBuffers(
						0,			// Preserve the existing buffer count
						nWidth,		// New width
						nHeight,	// New height
						DXGI_FORMAT_R8G8B8A8_UNORM,	// Format
						resizeFlags	// Allow screen tearing if enabled
					);

					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to resize swapchain buffers!");
				}
			});

		// Store the new dimensions
		m_nTopLeftX = nX;
		m_nTopLeftY = nY;
		m_nWindowWidth = nWidth;
		m_nWindowHeight = nHeight;

		// Recreate the render target views and depth stencil view
		CreateRenderTarget();
	}

	void CRendererAPI::SetRasterizerState(const RasterizerSpecification_t& spec)
	{
		PENUMBRA_PROFILE_FUNC();
		m_Specification = spec;

		// Create D3D11 rasterizer description from specification
		D3D11_RASTERIZER_DESC desc{};
		desc.FillMode = Utils::ToD3D11(spec.m_Fill);
		desc.CullMode = Utils::ToD3D11(spec.m_Cull);
		desc.FrontCounterClockwise = spec.m_bFrontCounterClockwise;
		desc.DepthClipEnable = spec.m_bDepthClipEnable;
		desc.ScissorEnable = spec.m_bScissorEnable;
		desc.MultisampleEnable = spec.m_bMultisampleEnable;
		desc.AntialiasedLineEnable = spec.m_bAntialiasedLineEnable;
		desc.DepthBias = spec.m_nDepthBias;
		desc.DepthBiasClamp = spec.m_flDepthBiasClamp;
		desc.SlopeScaledDepthBias = spec.m_flSlopeScaledDepthBias;

		CRenderer::Submit([this, desc]()
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::RSSetState");

				// Get or create the rasterizer state from the cache
				ComPtr<ID3D11RasterizerState> spState = m_RasterizerStateCache.GetOrCreate(desc);
				// Set the rasterizer state to the device context
				m_spContext->RSSetState(spState.Get());
			});
	}

	void CRendererAPI::SetDepth(bool bEnabled)
	{
		PENUMBRA_PROFILE_FUNC();

		D3D11_DEPTH_STENCIL_DESC desc{};
		desc.DepthEnable = bEnabled;	// Enable or disable depth testing
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;	// Write to all depth bits
		desc.DepthFunc = D3D11_COMPARISON_LESS;				// Less depth test function
		desc.StencilEnable = FALSE;	// Disable stencil testing

		CRenderer::Submit([this, desc]()
			{
				ComPtr<ID3D11DepthStencilState> spState = nullptr;

				// Create depth-stencil state
				{
					PENUMBRA_PROFILE_SCOPE("CreateDepthStencilState");
					HRESULT hr = m_spDevice->CreateDepthStencilState(&desc, spState.GetAddressOf());
					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Failed to create DepthStencilState!");
				}

				// Bind depth-stencil state
				{
					PENUMBRA_PROFILE_SCOPE("OMSetDepthStencilState");
					m_spContext->OMSetDepthStencilState(spState.Get(), 1);
				}

				// Store after creation and binding
				m_spDepthStencilState = spState;
			});
	}

	void CRendererAPI::SetClearColor(const glm::vec4& color)
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this, color]()
			{
				// Update the clear color
				memcpy(m_flBackBufferColor, &color[0], sizeof(float) * 4);
			});
	}

	void CRendererAPI::Clear()
	{
		PENUMBRA_PROFILE_FUNC();

#ifdef PENUMBRA_DEBUG
		// Debug message polling
		// We put it here as we expect Clear to be called once per frame
		PollDebugMessages();
#endif

		CRenderer::Submit([this]()
			{
				ComPtr<ID3D11DepthStencilView> spDSV = nullptr;
				ComPtr<ID3D11RenderTargetView> spRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];

				// Get current render targets
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::OMGetRenderTargets");

					ID3D11RenderTargetView* rawRtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
					ID3D11DepthStencilView* rawDsv = nullptr;

					m_spContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rawRtvs, &rawDsv);

					// Wrap raw pointers in ComPtr for automatic reference counting
					for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
						spRTVs[i].Attach(rawRtvs[i]);

					spDSV.Attach(rawDsv);
				}

				// Clear render targets
				for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
					if (spRTVs[i]) {
						PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::ClearRenderTargetView");
						m_spContext->ClearRenderTargetView(spRTVs[i].Get(), m_flBackBufferColor);
					}
				}

				// Clear depth-stencil view
				if (spDSV) {
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::ClearDepthStencilView");
					m_spContext->ClearDepthStencilView(
						spDSV.Get(),	// Depth-stencil view to clear
						D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,	// Clear both depth and stencil
						kMaxDepth,	// Clear depth to maximum value
						kDepthStencilClearValue	// Clear stencil to default value
					);
				}
			});
	}

	void CRendererAPI::Bind()
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderer::Submit([this]()
			{
				// Bind the render target and depth-stencil view
				PENUMBRA_PROFILE_SCOPE("OMSetRenderTargets");
				m_spContext->OMSetRenderTargets(1, m_spRenderTargetView.GetAddressOf(), m_spDepthDSV.Get());
			});

		// Set the viewport
		SetViewport();
	}

	void CRendererAPI::DrawIndexed(const Ref<CVertexArray>& spVertexArray, uint32_t nIndexCount)
	{
		PENUMBRA_PROFILE_FUNC();

		// Prepare for indexed draw
		const uint32_t count = PrepareIndexedDraw(spVertexArray, nIndexCount);

		CRenderer::Submit([this, count]()
			{
				// Draw the indexed geometry
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::DrawIndexed");
				m_spContext->DrawIndexed(count, 0, 0);
			});
	}

	void CRendererAPI::DrawIndexedInstanced(const Ref<CVertexArray>& spVertexArray, uint32_t nInstanceCount, uint32_t nIndexCount)
	{
		PENUMBRA_PROFILE_FUNC();

		// Prepare for indexed draw
		const uint32_t count = PrepareIndexedDraw(spVertexArray, nIndexCount);

		CRenderer::Submit([this, count, nInstanceCount]()
			{
				// Draw the indexed geometry with instancing
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::DrawIndexedInstanced");
				m_spContext->DrawIndexedInstanced(count, nInstanceCount, 0, 0, 0);
			});
	}

	void CRendererAPI::DrawIndexedInstanced(const Ref<CVertexArray>& spVertexArray, uint32_t nInstanceCount, CSubmesh& submesh)
	{
		PENUMBRA_PROFILE_FUNC();

		// Prepare for indexed draw
		const uint32_t nIndexCount = PrepareIndexedDraw(spVertexArray, submesh.m_nIndexCount);
		const uint32_t nBaseIndex = submesh.m_nBaseIndex;
		const uint32_t nBaseVertex = submesh.m_nBaseVertex;

		CRenderer::Submit([this, nInstanceCount, nIndexCount, nBaseIndex, nBaseVertex]()
			{
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::DrawIndexedInstanced");
				m_spContext->DrawIndexedInstanced(
					nIndexCount,	// Index count per instance
					nInstanceCount,	// Number of instances
					nBaseIndex,		// Start index location
					nBaseVertex,	// Base vertex location
					0	// Start instance location
				);
			});
	}

	void CRendererAPI::DrawLines(const Ref<CVertexArray>& spVertexArray, uint32_t nVertexCount)
	{
		PENUMBRA_PROFILE_FUNC();

		// Set line list topology
		RequestTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		// Bind the vertex array
		spVertexArray->Bind();

		// Disable culling
		const RasterizerSpecification_t oldSpec = m_Specification;
		auto newSpec = oldSpec;
		newSpec.m_Cull = RasterizerSpecification_t::CullMode::None;
		SetRasterizerState(newSpec);

		CRenderer::Submit([this, nVertexCount]()
			{
				// Draw the lines
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::DrawLines");
				m_spContext->Draw(nVertexCount, 0);
			});

		// Restore culling
		SetRasterizerState(oldSpec);
	}

	void CRendererAPI::DrawMesh(const Ref<CMesh>& spMesh, uint32_t nInstanceCount)
	{
		PENUMBRA_PROFILE_FUNC();

		RequestTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		spMesh->GetVertexArray()->Bind();

		// Draw each submesh
		for (const auto& submesh : spMesh->GetSubmeshes())
		{
			// Extract submesh parameters
			const uint32_t indexCount = submesh.m_nIndexCount;
			const uint32_t baseIndex = submesh.m_nBaseIndex;
			const uint32_t baseVertex = submesh.m_nBaseVertex;

			CRenderer::Submit([this, nInstanceCount, indexCount, baseIndex, baseVertex]()
				{
					// Draw the indexed geometry with instancing
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::DrawIndexedInstanced");
					m_spContext->DrawIndexedInstanced(
						indexCount, 
						nInstanceCount,
						baseIndex, 
						baseVertex, 
						0
					);
				});
		}
	}

	void CRendererAPI::CreateRenderTarget()
	{
		PENUMBRA_PROFILE_FUNC();

		const uint32_t nWidth = m_nWindowWidth;
		const uint32_t nHeight = m_nWindowHeight;

		// Create Render Target View
		CRenderer::Submit([this, nWidth, nHeight]()
			{
				// Get the back buffer from the swap chain
				ComPtr<ID3D11Texture2D> backBuffer;
				{
					PENUMBRA_PROFILE_SCOPE("IDXGISwapChain1::GetBuffer");
					HRESULT hr = m_spSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
						reinterpret_cast<void**>(backBuffer.GetAddressOf()));
					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr) && backBuffer, "RendererAPI: Failed to get swapchain back buffer!");
				}

				// Create the render target view
				{
					D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
					rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			// 8-bit RGBA format
					rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;	// 2D texture

					PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateRenderTargetView");
					HRESULT hr = m_spDevice->CreateRenderTargetView(backBuffer.Get(), &rtvDesc, m_spRenderTargetView.GetAddressOf());
					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr) && m_spRenderTargetView, "RendererAPI: Failed to create RenderTargetView!");
				}

				// Create the depth stencil buffer
				ComPtr<ID3D11Texture2D> depthStencilBuffer;
				{
					D3D11_TEXTURE2D_DESC depthDesc{};
					depthDesc.Width = nWidth;	// Window width
					depthDesc.Height = nHeight;	// Window height
					depthDesc.MipLevels = 1;	// No mipmaps
					depthDesc.ArraySize = 1;	// Single texture
					depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24 bits for depth, 8 bits for stencil
					depthDesc.SampleDesc.Count = 1;						// No multi-sampling
					depthDesc.SampleDesc.Quality = 0;					// Standard quality level
					depthDesc.Usage = D3D11_USAGE_DEFAULT;				// GPU read/write access
					depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;		// Bind as depth-stencil buffer

					PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateTexture2D(Depth)");
					HRESULT hr = m_spDevice->CreateTexture2D(&depthDesc, nullptr, depthStencilBuffer.GetAddressOf());
					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr) && depthStencilBuffer, "RendererAPI: Failed to create DepthStencil texture!");
				}

				// Create the depth stencil view
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateDepthStencilView");
					HRESULT hr = m_spDevice->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, m_spDepthDSV.GetAddressOf());
					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr) && m_spDepthDSV, "RendererAPI: Failed to create DepthStencilView!");
				}

				// Create the blend state for alpha blending
				D3D11_BLEND_DESC blendDesc{};
				blendDesc.IndependentBlendEnable = TRUE;	// Enable independent blending for multiple render targets

				// Alpha blending for the first render target
				D3D11_RENDER_TARGET_BLEND_DESC rtBlend{};
				rtBlend.BlendEnable = TRUE;	// Enable blending
				rtBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;		// Standard alpha blending
				rtBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;	// Standard alpha blending
				rtBlend.BlendOp = D3D11_BLEND_OP_ADD;			// Standard alpha blending
				rtBlend.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;	// Standard alpha blending
				rtBlend.DestBlendAlpha = D3D11_BLEND_ONE;		// Standard alpha blending
				rtBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;		// Standard alpha blending
				rtBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;	// Write to all color channels

				// Disabled blend state for other render targets
				D3D11_RENDER_TARGET_BLEND_DESC disabledBlend{};
				disabledBlend.BlendEnable = FALSE;
				disabledBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

				// Apply to all render targets
				blendDesc.RenderTarget[0] = rtBlend;
				for (int i = 1; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
					blendDesc.RenderTarget[i] = disabledBlend;

				// Create the blend state
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateBlendState");
					HRESULT hr = m_spDevice->CreateBlendState(&blendDesc, m_spBlendState.GetAddressOf());
					PENUMBRA_CORE_ASSERT(SUCCEEDED(hr) && m_spBlendState, "RendererAPI: Failed to create BlendState!");
				}

				// Bind the blend state
				{
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::OMSetBlendState");
					m_spContext->OMSetBlendState(m_spBlendState.Get(), nullptr, kSampleMask);
				}
			});

		CRenderer::Submit([this]()
			{
				// Clear the render target view
				if (m_spRenderTargetView) {
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::ClearRenderTargetView");
					m_spContext->ClearRenderTargetView(m_spRenderTargetView.Get(), kDefaultClearColor);
				}

				// Clear the depth stencil view
				if (m_spDepthDSV) {
					PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::ClearDepthStencilView");
					m_spContext->ClearDepthStencilView(
						m_spDepthDSV.Get(),		// Depth-stencil view to clear
						D3D11_CLEAR_DEPTH,		// Clear depth only
						kMaxDepth,				// Clear depth to maximum value
						kDepthStencilClearValue	// Clear stencil to default value
					);
				}
			});

		// Set the viewport to match the new render target size
		SetViewport();

		// Bind the created render target
		Bind();
	}

	void CRendererAPI::SetViewport()
	{
		PENUMBRA_PROFILE_FUNC();

		// Define the viewport
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = static_cast<FLOAT>(m_nTopLeftX);
		viewport.TopLeftY = static_cast<FLOAT>(m_nTopLeftY);
		viewport.Width = static_cast<FLOAT>(m_nWindowWidth);
		viewport.Height = static_cast<FLOAT>(m_nWindowHeight);
		viewport.MinDepth = kMinDepth;
		viewport.MaxDepth = kMaxDepth;

		CRenderer::Submit([this, viewport]()
			{
				// Set the viewport
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::RSSetViewports");
				m_spContext->RSSetViewports(1, &viewport);
			});
	}

	void CRendererAPI::RequestTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		PENUMBRA_PROFILE_FUNC();

		// Avoid unneeded state changes
		if (m_PrimitiveTopology == topology)
			return;

		// Update current topology
		m_PrimitiveTopology = topology;

		CRenderer::Submit([this, topology]()
			{
				// Set the primitive topology
				PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::IASetPrimitiveTopology");
				m_spContext->IASetPrimitiveTopology(topology);
			});
	}

	const uint32_t CRendererAPI::PrepareIndexedDraw(const Ref<CVertexArray>& spVAO, uint32_t count)
	{
		RequestTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		spVAO->Bind();
		return (count != 0) ? count : spVAO->GetIndexBuffer()->GetCount();
	}

	void CRendererAPI::PollDebugMessages()
	{
#ifdef PENUMBRA_DEBUG
		PENUMBRA_PROFILE_FUNC();

		// Check if the info queue is available
		if (!m_spInfoQueue)
			return;

		// Retrieve and log all stored messages
		const UINT64 num = m_spInfoQueue->GetNumStoredMessages();
		for (UINT64 i = 0; i < num; ++i) {
			// Get the size of the message
			SIZE_T messageLength = 0;
			m_spInfoQueue->GetMessage(i, nullptr, &messageLength);

			// Allocate space for the message
			std::vector<char> storage(messageLength);
			// Retrieve the message
			D3D11_MESSAGE* msg = reinterpret_cast<D3D11_MESSAGE*>(storage.data());

			// Log the message based on its severity
			if (SUCCEEDED(m_spInfoQueue->GetMessage(i, msg, &messageLength)) && msg && msg->pDescription) {
				switch (msg->Severity)
				{
				case D3D11_MESSAGE_SEVERITY_CORRUPTION:
					PENUMBRA_CORE_CRITICAL("[D3D11]: {0}", msg->pDescription);
					break;
				case D3D11_MESSAGE_SEVERITY_ERROR:
					PENUMBRA_CORE_ERROR("[D3D11]: {0}", msg->pDescription);
					break;
				case D3D11_MESSAGE_SEVERITY_WARNING:
					PENUMBRA_CORE_WARN("[D3D11]: {0}", msg->pDescription);
					break;
				case D3D11_MESSAGE_SEVERITY_INFO:
					PENUMBRA_CORE_INFO("[D3D11]: {0}", msg->pDescription);
					break;
				case D3D11_MESSAGE_SEVERITY_MESSAGE:
				default:
					PENUMBRA_CORE_TRACE("[D3D11]: {0}", msg->pDescription);
					break;
				}
			}
		}

		m_spInfoQueue->ClearStoredMessages();
#endif
	}
}