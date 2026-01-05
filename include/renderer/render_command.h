#pragma once

#include "renderer/renderer_api.h"

namespace penumbra
{
	class CRenderCommand
	{
	public:
		static void Create()
		{
			s_spRendererAPI = CreateScope<CRendererAPI>();
		}

		static void Init()
		{
			s_spRendererAPI->Init();
		}

		static void SetViewport(uint32_t nX, uint32_t nY, uint32_t nWidth, uint32_t nHeight)
		{
			s_spRendererAPI->SetViewport(nX, nY, nWidth, nHeight);
		}
		static void SetRasterizerState(const RasterizerSpecification_t& spec)
		{
			s_spRendererAPI->SetRasterizerState(spec);
		}
		static void SetDepth(bool bEnabled)
		{
			s_spRendererAPI->SetDepth(bEnabled);
		}

		static void SetClearColor(const glm::vec4& color)
		{
			s_spRendererAPI->SetClearColor(color);
		}
		static void Clear()
		{
			s_spRendererAPI->Clear();
		}

		static void Bind()
		{
			s_spRendererAPI->Bind();
		}
		static void DrawIndexed(const Ref<CVertexArray>& spVertexArray, uint32_t nIndexCount = 0)
		{
			s_spRendererAPI->DrawIndexed(spVertexArray, nIndexCount);
		}
		static void DrawIndexedInstanced(const Ref<CVertexArray>& spVertexArray, uint32_t nInstanceCount, uint32_t nIndexCount = 0)
		{
			s_spRendererAPI->DrawIndexedInstanced(spVertexArray, nInstanceCount, nIndexCount);
		}
		static void DrawIndexedInstanced(const Ref<CVertexArray>& spVertexArray, uint32_t nInstanceCount, CSubmesh& submesh)
		{
			s_spRendererAPI->DrawIndexedInstanced(spVertexArray, nInstanceCount, submesh);
		}
		static void DrawLines(const Ref<CVertexArray>& spVertexArray, uint32_t nVertexCount = 0)
		{
			s_spRendererAPI->DrawLines(spVertexArray, nVertexCount);
		}
		static void DrawMesh(const Ref<CMesh>& spMesh, uint32_t nInstanceCount)
		{
			s_spRendererAPI->DrawMesh(spMesh, nInstanceCount);
		}

		static const RasterizerSpecification_t& GetRasterizerSpecification()
		{
			return s_spRendererAPI->GetRasterizerSpecification();
		}
	private:
		static Scope<CRendererAPI> s_spRendererAPI;
	};
}