#include "ppch.h"
#include "renderer/renderer.h"

#include "core/application.h"

#include "renderer/scene_renderer.h"
#include "renderer/constant_buffer.h"
#include "renderer/mesh.h"
#include "renderer/material.h"

namespace penumbra
{
	// Key for color textures
	struct ColorKey_t
	{
		glm::vec4 Color;
		TextureSpecification_t TextureSpec;

		// Equals operator
		bool operator==(const ColorKey_t& other) const {
			// Use epsilon comparison for floating-point color values
			return glm::all(glm::epsilonEqual(Color, other.Color, 0.001f)) &&
				TextureSpec == other.TextureSpec;
		}
	};
}

namespace std {
	template<>
	// Specialization of std::hash for ColorKey_t
	struct hash<penumbra::ColorKey_t>
	{
		// Hash function
		size_t operator()(const penumbra::ColorKey_t& key) const {
			// Simple quantization to reduce floating-point precision issues
			auto quantize = [](float f) { return static_cast<int>(f * 1000.0f); };
			// Combine hashes of color components and texture format
			size_t h1 = std::hash<int>()(quantize(key.Color.x)) ^
				std::hash<int>()(quantize(key.Color.y) << 1) ^
				std::hash<int>()(quantize(key.Color.z) << 2) ^
				std::hash<int>()(quantize(key.Color.w) << 3);
			size_t h2 = std::hash<int>()(static_cast<int>(key.TextureSpec.m_Format));
			return h1 ^ (h2 << 3);
		}
	};
}

namespace penumbra
{
	struct RendererData_t
	{
		const std::string m_FSQuadShader = R"(
		#type vertex
		struct VSInput
		{
			float3 position : POSITION;
			float2 texCoord : TEXCOORD0;
		};
		float4 main(VSInput input) : SV_Position
		{
			return float4(input.position, 1.0f);
		}
	)";

		const std::string m_MeshShader = R"(
		#type vertex
		struct VSInput
		{
			// Mesh Vertex data
			float3 position : POSITION;
			float3 normal   : NORMAL;
			float3 tangent  : TANGENT;
			float2 texCoord : TEXCOORD0;

			// Instance data
			int entityID : ENTITYID;
			row_major matrix modelMatrix : MODELMATRIX;
		};
		float4 main(VSInput input) : SV_Position
		{
			return float4(input.position, 1.0f);
		}
	)";

		CRenderer::Statistics_t m_Stats;

		CRenderCommandQueue m_CommandQueue;

		Ref<CConstantBuffer> m_spMaterialUniformBuffer;

		// RenderQueue and Commands

		enum class RenderQueue {
			Opaque,
			Transparent,
			Overlay,
		};

		// Defaults
		std::unordered_map<ColorKey_t, Ref<CTexture2D>> m_TextureColorMap;	// Map of color keys to color textures
		Ref<CShader> m_spMeshShader;

		Ref<CRenderPass> m_spActiveRenderPass;

		Ref<CVertexArray> m_spFullscreenQuadVAO;
		Ref<CShader> m_spFullscreenQuadLayoutShader;
	};

	static RendererData_t* s_RendererData;

	void CRenderer::Init()
	{
		PENUMBRA_PROFILE_FUNC();

		s_RendererData = new RendererData_t();

		s_RendererData->m_CommandQueue.Enable();

		s_RendererData->m_spMaterialUniformBuffer = CreateRef<CConstantBuffer>(static_cast<uint32_t>(sizeof(glm::vec4)), 2, ShaderType::SHADER_TYPE_PIXEL);

		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};
		float x = -1, y = -1, width = 2, height = 2;
		QuadVertex* data = new QuadVertex[4]
		{
			{ {x,          y,          0.0f}, {0, 1} },
			{ {x + width,  y,          0.0f}, {1, 1} },
			{ {x + width,  y + height, 0.0f}, {1, 0} },
			{ {x,          y + height, 0.0f}, {0, 0} }
		};

		s_RendererData->m_spFullscreenQuadVAO = CreateRef<CVertexArray>();
		Ref<CVertexBuffer> fullscreenQuadVB = CreateRef<CVertexBuffer>((float*)data, 4u * static_cast<uint32_t>(sizeof(QuadVertex)));
		fullscreenQuadVB->SetLayout({
			{ ShaderDataType::Float3, kLayoutPositionSemantic },
			{ ShaderDataType::Float2, kLayoutTexCoordSemantic }
		});
		s_RendererData->m_spFullscreenQuadVAO->AddVertexBuffer(fullscreenQuadVB);
		s_RendererData->m_spFullscreenQuadLayoutShader = CreateRef<CShader>("VSFullscreenQuadLayout.hlsl", s_RendererData->m_FSQuadShader);
		s_RendererData->m_spFullscreenQuadVAO->CreateInputLayout(s_RendererData->m_spFullscreenQuadLayoutShader);
		uint32_t indices[6] = { 0, 2, 1, 2, 0, 3 };
		s_RendererData->m_spFullscreenQuadVAO->SetIndexBuffer(CreateRef<CIndexBuffer>(indices, 6u * static_cast<uint32_t>(sizeof(uint32_t))));

		s_RendererData->m_spMeshShader = CreateRef<CShader>(kMeshShaderName, s_RendererData->m_MeshShader);

		delete[] data;

		CRenderCommand::Init();
		CSceneRenderer::Init();
	}

	void CRenderer::Shutdown()
	{
		PENUMBRA_PROFILE_FUNC();

		s_RendererData->m_CommandQueue.Disable();
		delete s_RendererData;
		
		CSceneRenderer::Shutdown();
	}

	void CRenderer::OnWindowResize(uint32_t nWidth, uint32_t nHeight)
	{
		PENUMBRA_PROFILE_FUNC();

		CRenderCommand::SetViewport(0, 0, nWidth, nHeight);
	}

	void CRenderer::WaitAndRender()
	{
		PENUMBRA_PROFILE_FUNC();

		s_RendererData->m_CommandQueue.Execute();
	}

	void CRenderer::BeginRenderPass(Ref<CRenderPass> renderPass, bool clear)
	{
		PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(renderPass, "Renderer::BeginRenderPass - Render Pass cannot be null!");

		s_RendererData->m_spActiveRenderPass = renderPass;

		renderPass->GetSpecification().m_spTargetFramebuffer->Bind();
		if (clear) {
			const glm::vec4& clearColor = renderPass->GetSpecification().m_spTargetFramebuffer->GetSpecification().m_ClearColor;
			CRenderCommand::SetClearColor(clearColor);
			CRenderCommand::Clear();
		}
	}

	void CRenderer::EndRenderPass()
	{
		PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(s_RendererData->m_spActiveRenderPass, "Renderer::EndRenderPass - No active Render Pass!");

		s_RendererData->m_spActiveRenderPass->GetSpecification().m_spTargetFramebuffer->Unbind();
		s_RendererData->m_spActiveRenderPass = nullptr;
	}

	void CRenderer::SubmitFullscreenQuad(const Ref<CMaterial>& spMaterial)
	{
		PENUMBRA_PROFILE_FUNC();

		bool depthTest = true;
		bool cullFace = true;
		if (spMaterial)
		{
			Ref<CShader> shader = spMaterial->m_spShader;
			if (shader) {
				shader->Bind();
				spMaterial->UploadDataToGPU();
			}

			depthTest = spMaterial->m_bDepthTest;
		}

		if (!depthTest)
			CRenderCommand::SetDepth(false);

		s_RendererData->m_spFullscreenQuadVAO->Bind();

		CRenderCommand::DrawIndexed(s_RendererData->m_spFullscreenQuadVAO);
		
		if (!depthTest)
			CRenderCommand::SetDepth(true);
	}

	const Ref<CTexture2D>& CRenderer::GetColorTexture(const glm::vec4& vecColor, const TextureSpecification_t& textureSpec)
	{
		PENUMBRA_PROFILE_FUNC();

		// Create a key for the color texture
		ColorKey_t key = { vecColor, textureSpec };

		auto& textureMap = s_RendererData->m_TextureColorMap;

		// Check if the texture already exists
		auto it = textureMap.find(key);
		if (it != textureMap.end())
			return it->second;

		// Create a new texture
		Ref<CTexture2D> texture = CreateRef<CTexture2D>(textureSpec);
		// Set the texture data based on the color and format
		Buffer_t pixelData;

		switch (textureSpec.m_Format)
		{
		case ImageFormat::R8:
		{
			// Single channel red
			uint8_t r = static_cast<uint8_t>(vecColor.x * 255.0f);
			pixelData = Buffer_t::Copy(&r, sizeof(uint8_t));
			break;
		}
		case ImageFormat::RGBA8:
		default:
		{
			// 4 channel RGBA
			uint8_t rgba[4] = {
				static_cast<uint8_t>(vecColor.x * 255.0f),
				static_cast<uint8_t>(vecColor.y * 255.0f),
				static_cast<uint8_t>(vecColor.z * 255.0f),
				static_cast<uint8_t>(vecColor.w * 255.0f)
			};
			pixelData = Buffer_t::Copy(rgba, sizeof(rgba));
			break;
		}
		case ImageFormat::RGBA16F:
		{
			// 16-bit float RGBA
			// Not sure this is good enough precision conversion
			float rgba[4] = { vecColor.x, vecColor.y, vecColor.z, vecColor.w };
			pixelData = Buffer_t::Copy(rgba, sizeof(rgba));
			break;
		}
		case ImageFormat::RGB32F:
		{
			// 32-bit float RGB
			float rgb[3] = { vecColor.x, vecColor.y, vecColor.z };
			pixelData = Buffer_t::Copy(rgb, sizeof(rgb));
			break;
		}
		case ImageFormat::RGBA32F:
		{
			// 32-bit float RGBA
			float rgba[4] = { vecColor.x, vecColor.y, vecColor.z, vecColor.w };
			pixelData = Buffer_t::Copy(rgba, sizeof(rgba));
			break;
		}
		}

		// Upload the pixel data to the texture
		texture->SetData(pixelData);
		pixelData.Release();
		// Store the texture in the map
		textureMap[key] = texture;

		// Finally return the texture
		return textureMap[key];
	}

	const Ref<CShader> CRenderer::GetMeshShader()
	{
		return s_RendererData->m_spMeshShader;
	}

	void CRenderer::ResetStats()
	{
		memset(&s_RendererData->m_Stats, 0, static_cast<uint32_t>(sizeof(Statistics_t)));
	}

	CRenderer::Statistics_t& CRenderer::GetStats()
	{
		return s_RendererData->m_Stats;
	}

	CRenderCommandQueue& CRenderer::GetRenderCommandQueue()
	{
		return s_RendererData->m_CommandQueue;
	}
}