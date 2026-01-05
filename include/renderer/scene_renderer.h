#pragma once

#include "renderer/camera.h"

namespace penumbra
{
	class CMesh;
	class CMaterial;
	class CRenderPass;
	class CVertexBuffer;
	class CFramebuffer;

	class CScene;
	struct LightComponent_t;

	struct SceneRendererCamera_t
	{
		CCamera Camera;
		glm::mat4 ViewMatrix;
		float Near, Far;
		float FOV;
	};

	struct SceneRendererPasses_t
	{
		Ref<CRenderPass> m_spShadowPass;
		Ref<CRenderPass> m_spGeometryPass;
		Ref<CRenderPass> m_spCompositePass;
	};

	class CSceneRenderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void SetViewportSize(uint32_t width, uint32_t height);

		static void BeginScene(const CScene* scene, const SceneRendererCamera_t& camera, const glm::vec2& viewportSize);
		static void EndScene();

		static void SubmitLight(const glm::mat4& matTransform, const LightComponent_t& lc);
		static void SubmitMesh(const Ref<CMesh>& spMesh, const glm::mat4& transform, const Ref<CMaterial>& spMaterial, int nEntityId);
	
		static int ReadGeometryPassPixel(int nAttachment, int x, int y);
		static void RenderFinalPassToBuffer(const Ref<CFramebuffer>& spFramebuffer);

		static Ref<CRenderPass> GetFinalRenderPass();
		static const Ref<CVertexBuffer>& GetInstanceBuffer();

		static SceneRendererPasses_t& GetSceneRendererPasses();
		static void SetSceneRendererPasses(SceneRendererPasses_t passes);
	private:
		static void FlushDrawList();

		static void ShadowPass();
		static void GeometryPass();
		static void CompositePass();
	};
}