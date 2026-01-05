#include "ppch.h"
#include "renderer/mesh.h"

#include "renderer/scene_renderer.h"
#include "renderer/renderer.h"

namespace penumbra
{
	CMesh::CMesh(std::vector<Vertex_t> vecVertices, std::vector<Index_t> vecIndices, std::vector<CSubmesh> vecSubmeshes)
		: m_vecSubmeshes(std::move(vecSubmeshes))
	{
		PENUMBRA_PROFILE_FUNC();

		m_spVertexArray = CreateRef<CVertexArray>();

		// Create Vertex Array
		Ref<CVertexBuffer> vb = CreateRef<CVertexBuffer>(vecVertices.data(), static_cast<uint32_t>(vecVertices.size() * sizeof(Vertex_t)));

		CBufferLayout layout = {
			{ ShaderDataType::Float3, kLayoutPositionSemantic },
			{ ShaderDataType::Float3, kLayoutNormalSemantic   },
			{ ShaderDataType::Float3, kLayoutTangentSemantic  },
			{ ShaderDataType::Float2, kLayoutTexCoordSemantic }
		};
		vb->SetLayout(layout);

		Ref<CIndexBuffer> ib = CreateRef<CIndexBuffer>(vecIndices.data(), static_cast<uint32_t>(vecIndices.size() * sizeof(Index_t)));

		m_spVertexArray->AddVertexBuffer(vb);
		m_spVertexArray->SetIndexBuffer(ib);
		m_spVertexArray->AddInstanceBuffer(CSceneRenderer::GetInstanceBuffer());
		m_spVertexArray->CreateInputLayout(CRenderer::GetMeshShader());
	}
}