#include "ppch.h"
#include "renderer/scene_renderer.h"

#include "renderer/mesh.h"
#include "renderer/material.h"
#include "renderer/render_pass.h"
#include "renderer/renderer.h"
#include "renderer/shader.h"

namespace penumbra
{
	namespace Utils
	{
		static RasterizerSpecification_t::CullMode RenderFaceToCullMode(RenderFace face)
		{
			switch (face)
			{
			case RenderFace::Front: return RasterizerSpecification_t::CullMode::Back;
			case RenderFace::Back: return RasterizerSpecification_t::CullMode::Front;
			case RenderFace::Both: return RasterizerSpecification_t::CullMode::None; // No culling
			}
			PENUMBRA_CORE_ASSERT(false, "Unknown RenderFace!");
			return RasterizerSpecification_t::CullMode::None;
		}
	}

	struct SceneRendererData_t
	{
		// OLDER
		static const uint32_t m_nMaxLights = 16;
		static const uint32_t m_nMaxInstances = 2048;

		static const uint32_t m_nShadowMapSlot = 16;

		// Buffer data structs

		struct CameraData_t
		{
			alignas(16)
			glm::mat4 m_matViewProjectionMatrix;
			glm::mat4 m_matInverseViewProjectionMatrix;
			glm::mat4 m_matViewMatrix;
			glm::mat4 m_matInverseViewMatrix;
			glm::mat4 m_matProjectionMatrix;
			glm::mat4 m_matInverseProjectionMatrix;
			glm::vec2 m_ViewportSize;
		};

		struct PerLightData_t
		{
			alignas(16)glm::vec3 m_Position;
			alignas(4)int m_nType;

			alignas(16)glm::vec3 m_Direction; // Normalized
			float m_flRange;

			alignas(16)glm::vec3 m_Color;
			float m_flPower;

			alignas(16)float m_flSpotInner; // Spot Specific
			float m_flSpotOuter; // Spot Specific

			float _Padding0;
			float _Padding1;
		};

		// Uniform/Constant buffer structs

		struct GlobalData_t
		{
			CameraData_t m_CameraData;

			glm::mat4 m_matLightSpaceMatrix = glm::mat4(1.0); // For shadow mapping
		};

		struct LightingData_t
		{
			PerLightData_t m_Lights[SceneRendererData_t::m_nMaxLights];
			int m_nActiveLights;
		};


		LightingData_t m_LightingBuffer;
		Ref<CConstantBuffer> m_spLightingUniformBuffer;

		GlobalData_t m_GlobalBuffer;
		Ref<CConstantBuffer> m_spGlobalUniformBuffer;

		struct InstanceData_t
		{
			glm::mat4 m_matModelTransform;
		};

		std::vector<InstanceData_t> m_vecInstanceDataBuffers;
		uint32_t m_nInstanceDataBufferSize;
		uint32_t m_nInstanceDataCount;
		Ref<CVertexBuffer> m_spInstanceBuffer;

		// NEWER
		Ref<CShader> m_spShadowMapDepthShader;

		const CScene* m_pActiveScene = nullptr;
		struct SceneInfo_t
		{
			SceneRendererCamera_t m_SceneCamera;
		} m_SceneData;

		Ref<CShader> m_spCompositeShader;

		SceneRendererPasses_t m_RendererPasses;

		struct DrawCommand_t
		{
			Ref<CMesh> m_spMesh;
			Ref<CMaterial> m_spMaterial;
			std::vector<glm::mat4> m_vecTransforms;
			int m_nInstanceCount = 0;
		};
		std::vector<DrawCommand_t> m_vecDrawList;

		RasterizerSpecification_t m_DefaultRasterSpec;
	};

	static SceneRendererData_t* s_SceneRendererData;

	void CSceneRenderer::Init()
	{
		PENUMBRA_PROFILE_FUNC();

		s_SceneRendererData = new SceneRendererData_t();

		FramebufferSpecification_t shadowFramebufferSpec;
		shadowFramebufferSpec.m_Attachments = { FramebufferTextureFormat::Depth };
		shadowFramebufferSpec.m_nWidth = 1024;
		shadowFramebufferSpec.m_nHeight = 1024;

		RenderPassSpecification_t shadowPassSpec;
		shadowPassSpec.m_spTargetFramebuffer = CreateRef<CFramebuffer>(shadowFramebufferSpec);
		s_SceneRendererData->m_RendererPasses.m_spShadowPass = CreateRef<CRenderPass>(shadowPassSpec);

		FramebufferSpecification_t geoFramebufferSpec;
		geoFramebufferSpec.m_Attachments = { FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		geoFramebufferSpec.m_ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		geoFramebufferSpec.m_nWidth = 1920;
		geoFramebufferSpec.m_nHeight = 1080;

		RenderPassSpecification_t geoPassSpec;
		geoPassSpec.m_spTargetFramebuffer = CreateRef<CFramebuffer>(geoFramebufferSpec);
		s_SceneRendererData->m_RendererPasses.m_spGeometryPass = CreateRef<CRenderPass>(geoPassSpec);

		FramebufferSpecification_t compFramebufferSpec;
		compFramebufferSpec.m_Attachments = { FramebufferTextureFormat::RGBA8 };
		compFramebufferSpec.m_ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		compFramebufferSpec.m_nWidth = 1920;
		compFramebufferSpec.m_nHeight = 1080;

		RenderPassSpecification_t compPassSpec;
		compPassSpec.m_spTargetFramebuffer = CreateRef<CFramebuffer>(compFramebufferSpec);
		s_SceneRendererData->m_RendererPasses.m_spCompositePass = CreateRef<CRenderPass>(compPassSpec);

		//s_SceneRendererData->m_spCompositeShader = CreateRef<CShader>("assets\\shaders\\Engine\\SceneComposite.hlsl");
		//s_SceneRendererData->m_spShadowMapDepthShader = CreateRef<CShader>("assets\\shaders\\Engine\\ShadowMap.hlsl");

		// OLDER
		s_SceneRendererData->m_spGlobalUniformBuffer = CreateRef<CConstantBuffer>(static_cast<uint32_t>(sizeof(SceneRendererData_t::GlobalData_t)), 0, ShaderType::SHADER_TYPE_VERTEX);

		s_SceneRendererData->m_spLightingUniformBuffer = CreateRef<CConstantBuffer>(static_cast<uint32_t>(sizeof(SceneRendererData_t::LightingData_t)), 1, ShaderType::SHADER_TYPE_PIXEL);

		s_SceneRendererData->m_nInstanceDataBufferSize = SceneRendererData_t::m_nMaxInstances * static_cast<uint32_t>(sizeof(SceneRendererData_t::InstanceData_t));
		s_SceneRendererData->m_spInstanceBuffer = CreateRef<CVertexBuffer>(s_SceneRendererData->m_nInstanceDataBufferSize);
		s_SceneRendererData->m_spInstanceBuffer->SetLayout({
			{ ShaderDataType::Int, "ENTITYID", true },
			{ ShaderDataType::Mat4, "MODELMATRIX", true }
		});

		s_SceneRendererData->m_DefaultRasterSpec = CRenderCommand::GetRasterizerSpecification();
	}

	void CSceneRenderer::Shutdown()
	{
		PENUMBRA_PROFILE_FUNC();

		delete s_SceneRendererData;
	}

	void CSceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		PENUMBRA_PROFILE_FUNC();

		s_SceneRendererData->m_RendererPasses.m_spGeometryPass->GetSpecification().m_spTargetFramebuffer->Resize(width, height);
		s_SceneRendererData->m_RendererPasses.m_spCompositePass->GetSpecification().m_spTargetFramebuffer->Resize(width, height);
	}

	void CSceneRenderer::BeginScene(const CScene* scene, const SceneRendererCamera_t& camera, const glm::vec2& viewportSize)
	{
		PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(!s_SceneRendererData->m_pActiveScene, "SceneRenderer::BeginScene - Scene already started!");

		auto& matProjection = camera.Camera.GetProjection();
		auto& matView = camera.ViewMatrix;
		auto matViewProjection = matProjection * matView;
		auto matInverseView = glm::inverse(matView);

		s_SceneRendererData->m_pActiveScene = scene;

		s_SceneRendererData->m_SceneData.m_SceneCamera = camera;

		s_SceneRendererData->m_GlobalBuffer.m_CameraData.m_matViewProjectionMatrix = matViewProjection;
		s_SceneRendererData->m_GlobalBuffer.m_CameraData.m_matInverseViewProjectionMatrix = glm::inverse(matViewProjection);
		s_SceneRendererData->m_GlobalBuffer.m_CameraData.m_matViewMatrix = matView;
		s_SceneRendererData->m_GlobalBuffer.m_CameraData.m_matInverseViewMatrix = matInverseView;
		s_SceneRendererData->m_GlobalBuffer.m_CameraData.m_matProjectionMatrix = matProjection;
		s_SceneRendererData->m_GlobalBuffer.m_CameraData.m_matInverseProjectionMatrix = glm::inverse(matProjection);
		s_SceneRendererData->m_GlobalBuffer.m_CameraData.m_ViewportSize = viewportSize;
		s_SceneRendererData->m_spGlobalUniformBuffer->SetData(&s_SceneRendererData->m_GlobalBuffer, static_cast<uint32_t>(sizeof(SceneRendererData_t::GlobalData_t)));

		s_SceneRendererData->m_LightingBuffer.m_nActiveLights = 0;
		s_SceneRendererData->m_spLightingUniformBuffer->SetData(&s_SceneRendererData->m_LightingBuffer, static_cast<uint32_t>(sizeof(SceneRendererData_t::LightingData_t)));
	
		s_SceneRendererData->m_vecInstanceDataBuffers.clear();
		s_SceneRendererData->m_vecInstanceDataBuffers.resize(SceneRendererData_t::m_nMaxInstances);
		s_SceneRendererData->m_nInstanceDataCount = 0;
	}

	void CSceneRenderer::EndScene()
	{
		PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(s_SceneRendererData->m_pActiveScene, "SceneRenderer::EndScene - No active scene!");

		s_SceneRendererData->m_pActiveScene = nullptr;

		FlushDrawList();
	}

	void CSceneRenderer::SubmitLight(const glm::mat4& matTransform, const LightComponent_t& lc)
	{
		/*if (s_SceneRendererData->m_LightingBuffer.m_nActiveLights >= SceneRendererData_t::m_nMaxLights) {
			return;
		}

		SceneRendererData_t::PerLightData_t& lightData = s_SceneRendererData->m_LightingBuffer.m_Lights[s_SceneRendererData->m_LightingBuffer.m_nActiveLights];

		lightData.m_Position = matTransform[3];
		lightData.m_nType = (int)lc.m_Type;

		glm::vec3 pos, rot, scale;
		Math::DecomposeTransform(matTransform, pos, rot, scale);
		glm::vec3 forward = glm::rotate(glm::quat(rot), glm::vec3(0.0f, 0.0f, -1.0f));
		lightData.m_Direction = forward;

		lightData.m_Color = lc.m_Color;
		lightData.m_flPower = lc.m_flPower;

		switch (lc.m_Type)
		{
		case LightType::Directional:
			lightData.m_flRange = 0.0f;
			lightData.m_flSpotInner = 0.0f;
			lightData.m_flSpotOuter = 0.0f;
			break;

		case LightType::Point:
			lightData.m_flRange = lc.m_flRange;
			lightData.m_flSpotInner = 0.0f;
			lightData.m_flSpotOuter = 0.0f;
			break;

		case LightType::Spot:
			lightData.m_flRange = lc.m_flRange;
			lightData.m_flSpotInner = std::cos(glm::radians(lc.m_flSpotInner));
			lightData.m_flSpotOuter = std::cos(glm::radians(lc.m_flSpotOuter));
			break;
		}

		s_SceneRendererData->m_LightingBuffer.m_nActiveLights++;
		s_SceneRendererData->m_spLightingUniformBuffer->SetData(&s_SceneRendererData->m_LightingBuffer, static_cast<uint32_t>(sizeof(SceneRendererData_t::m_LightingBuffer)));*/
	}

	void CSceneRenderer::SubmitMesh(const Ref<CMesh>& spMesh, const glm::mat4& transform, const Ref<CMaterial>& spMaterial, int nEntityId)
	{
		PENUMBRA_PROFILE_FUNC();

		for (auto& dc : s_SceneRendererData->m_vecDrawList)
		{
			if (dc.m_spMesh == spMesh && dc.m_spMaterial == spMaterial)
			{
				// Same mesh and material found
				dc.m_vecTransforms.push_back(transform);
				dc.m_nInstanceCount++;
				return;
			}
		}

		// No existing command found
		SceneRendererData_t::DrawCommand_t newCommand;
		newCommand.m_spMesh = spMesh;
		newCommand.m_spMaterial = spMaterial;
		newCommand.m_vecTransforms.push_back(transform);
		newCommand.m_nInstanceCount = 1;

		s_SceneRendererData->m_vecDrawList.push_back(std::move(newCommand));
	}

	int CSceneRenderer::ReadGeometryPassPixel(int nAttachment, int x, int y)
	{
		PENUMBRA_PROFILE_FUNC();

		return s_SceneRendererData->m_RendererPasses.m_spGeometryPass->GetSpecification().m_spTargetFramebuffer->ReadPixel(nAttachment, x, y);
	}

	void CSceneRenderer::RenderFinalPassToBuffer(const Ref<CFramebuffer>& spFramebuffer)
	{
		PENUMBRA_PROFILE_FUNC();

		auto& renderBuffer = GetFinalRenderPass()->GetSpecification().m_spTargetFramebuffer;
		
		spFramebuffer->Bind();

		renderBuffer->BindTexture(0, 0);
		renderBuffer->BindTexture(1, 1);
		s_SceneRendererData->m_spCompositeShader->Bind();
		CRenderer::SubmitFullscreenQuad(nullptr);
		renderBuffer->UnbindTexture(0);
		renderBuffer->UnbindTexture(1);

		spFramebuffer->Unbind();
	}

	Ref<CRenderPass> CSceneRenderer::GetFinalRenderPass()
	{
		return s_SceneRendererData->m_RendererPasses.m_spCompositePass;
	}

	const Ref<CVertexBuffer>& CSceneRenderer::GetInstanceBuffer()
	{
		return s_SceneRendererData->m_spInstanceBuffer;
	}

	SceneRendererPasses_t& CSceneRenderer::GetSceneRendererPasses()
	{
		return s_SceneRendererData->m_RendererPasses;
	}

	void CSceneRenderer::SetSceneRendererPasses(SceneRendererPasses_t passes)
	{
		s_SceneRendererData->m_RendererPasses = passes;
	}

	void CSceneRenderer::FlushDrawList()
	{
		PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(!s_SceneRendererData->m_pActiveScene, "SceneRenderer::FlushDrawList - Scene hasn't ended!");

		ShadowPass();
		GeometryPass();
		CompositePass();

		s_SceneRendererData->m_vecDrawList.clear();
		s_SceneRendererData->m_SceneData = {};
	}

	void CSceneRenderer::ShadowPass()
	{
		PENUMBRA_PROFILE_FUNC();

		//bool found = false;
		//glm::vec3 lightDir; // direction vector stored in light

		//for (int i = 0; i < s_SceneRendererData->m_LightingBuffer.m_nActiveLights; i++) {
		//	auto& light = s_SceneRendererData->m_LightingBuffer.m_Lights[i];
		//	if (light.m_nType == (int)LightType::Directional) {
		//		found = true;
		//		lightDir = light.m_Direction;
		//		break;
		//	}
		//}

		//if (!found)
		//	return;

		//glm::mat4 lightProjection, lightView;
		//glm::mat4 lightSpaceMatrix;
		//static const float nearPlane = 1.0f, farPlane = 7.5f;
		////lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
		//lightProjection = glm::orthoLH(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
		//lightView = glm::lookAtLH(lightDir, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		//lightSpaceMatrix = lightProjection * lightView;

		//s_SceneRendererData->m_GlobalBuffer.m_matLightSpaceMatrix = lightSpaceMatrix;
		//
		//s_SceneRendererData->m_spGlobalUniformBuffer->SetData(&s_SceneRendererData->m_GlobalBuffer, static_cast<uint32_t>(sizeof(SceneRendererData_t::GlobalData_t)));
		//s_SceneRendererData->m_spGlobalUniformBuffer->Bind();

		//auto& shadowPass = s_SceneRendererData->m_RendererPasses.m_spShadowPass;

		//CRenderer::BeginRenderPass(shadowPass);
		//s_SceneRendererData->m_spShadowMapDepthShader->Unbind();
		//s_SceneRendererData->m_spShadowMapDepthShader->Bind();
		//for (auto& dc : s_SceneRendererData->m_vecDrawList) {
		//	for (size_t i = 0; i < dc.m_vecTransforms.size(); i++) {
		//		auto& data = s_SceneRendererData->m_vecInstanceDataBuffers[i];
		//		data.m_matModelTransform = dc.m_vecTransforms[i];
		//		data.m_nEntityID = dc.m_vecEntityIDs[i];
		//	}

		//	s_SceneRendererData->m_spInstanceBuffer->SetData(
		//		s_SceneRendererData->m_vecInstanceDataBuffers.data(),
		//		s_SceneRendererData->m_nInstanceDataBufferSize
		//	);

		//	CRenderCommand::DrawMesh(dc.m_spMesh, dc.m_nInstanceCount);
		//}
		//CRenderer::EndRenderPass();
	}

	void CSceneRenderer::GeometryPass()
	{
		PENUMBRA_PROFILE_FUNC();

		auto& geoPass = s_SceneRendererData->m_RendererPasses.m_spGeometryPass;

		CRenderer::BeginRenderPass(geoPass);
		geoPass->GetSpecification().m_spTargetFramebuffer->ClearAttachment(1, -1);

		auto currentRasterSpec = CRenderCommand::GetRasterizerSpecification();
		CRenderCommand::SetRasterizerState(s_SceneRendererData->m_DefaultRasterSpec);

		auto& shadowBuffer = s_SceneRendererData->m_RendererPasses.m_spShadowPass->GetSpecification().m_spTargetFramebuffer;

		auto& sceneCamera = s_SceneRendererData->m_SceneData.m_SceneCamera;
		for (auto& dc : s_SceneRendererData->m_vecDrawList) {
			Ref<CShader> shader = dc.m_spMaterial->m_spShader;
			if (!shader) {
				return;
			}

			shader->Bind();
			dc.m_spMaterial->UploadDataToGPU();

			shadowBuffer->BindDepthTexture(SceneRendererData_t::m_nShadowMapSlot);

			RasterizerSpecification_t materialRasterSpec = currentRasterSpec;
			materialRasterSpec.m_Cull = Utils::RenderFaceToCullMode(dc.m_spMaterial->m_RenderFace);
			CRenderCommand::SetRasterizerState(materialRasterSpec);

			for (size_t i = 0; i < dc.m_vecTransforms.size(); i++) {
				auto& data = s_SceneRendererData->m_vecInstanceDataBuffers[i];
				data.m_matModelTransform = dc.m_vecTransforms[i];
			}

			s_SceneRendererData->m_spInstanceBuffer->SetData(
				s_SceneRendererData->m_vecInstanceDataBuffers.data(),
				s_SceneRendererData->m_nInstanceDataBufferSize
			);

			CRenderCommand::DrawMesh(dc.m_spMesh, dc.m_nInstanceCount);
			/*for (const auto& vao : dc.m_spMesh->GetVertexArrays()) {
				CRenderCommand::DrawIndexedInstanced(vao, dc.m_nInstanceCount);
			}*/

			shadowBuffer->UnbindTexture(SceneRendererData_t::m_nShadowMapSlot);
		}

		CRenderCommand::SetRasterizerState(currentRasterSpec);

		CRenderer::EndRenderPass();
	}

	void CSceneRenderer::CompositePass()
	{
		PENUMBRA_PROFILE_FUNC();

		auto& compositePass = s_SceneRendererData->m_RendererPasses.m_spCompositePass;
		auto& compositeBuffer = compositePass->GetSpecification().m_spTargetFramebuffer;

		CRenderer::BeginRenderPass(compositePass);
		
		s_SceneRendererData->m_spCompositeShader->Bind();
		auto& geoBuffer = s_SceneRendererData->m_RendererPasses.m_spGeometryPass->GetSpecification().m_spTargetFramebuffer;

		auto& shadowBuffer = s_SceneRendererData->m_RendererPasses.m_spShadowPass->GetSpecification().m_spTargetFramebuffer;

		auto currentRasterSpec = CRenderCommand::GetRasterizerSpecification();
		CRenderCommand::SetRasterizerState(s_SceneRendererData->m_DefaultRasterSpec);
		
		geoBuffer->BindTexture(0, 0);
		geoBuffer->BindTexture(1, 1);
		shadowBuffer->BindDepthTexture(SceneRendererData_t::m_nShadowMapSlot);
		
		CRenderer::SubmitFullscreenQuad(nullptr);

		geoBuffer->UnbindTexture(0);
		geoBuffer->UnbindTexture(1);
		shadowBuffer->UnbindTexture(SceneRendererData_t::m_nShadowMapSlot);

		CRenderCommand::SetRasterizerState(currentRasterSpec);

		CRenderer::EndRenderPass();
	}
}