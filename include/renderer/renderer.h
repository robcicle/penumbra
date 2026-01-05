#pragma once

#include "renderer/render_command.h"
#include "renderer/render_command_queue.h"
#include "renderer/render_pass.h"

#include "renderer/orthographic_camera.h"
#include "renderer/camera.h"
#include "renderer/texture.h"

namespace penumbra
{
	class CMaterial;
	class CMesh;

	class CRenderer
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		static void Init();
		static void Shutdown();
		static void OnWindowResize(uint32_t nWidth, uint32_t nHeight);

		template<typename FuncT>
		static void Submit(FuncT&& func)
		{
			auto& renderCommandQueue = GetRenderCommandQueue();
			if (!renderCommandQueue.IsActive()) {
				func();
				return;
			}

			auto renderCmd = [](void* ptr) {
				auto pFunc = (FuncT*)ptr;
				(*pFunc)();

				// NOTE: Instead of destroying we could try and enforce all items to be trivally destructible
				// however some items like uniforms which contain std::strings still exist for now
				// static_assert(std::is_trivially_destructible_v<FuncT>, "FuncT must be trivially destructible");
				pFunc->~FuncT();
				};
			auto storageBuffer = renderCommandQueue.Allocate(renderCmd, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}

		static void WaitAndRender();

		static void BeginRenderPass(Ref<CRenderPass> renderPass, bool clear = true);
		static void EndRenderPass();

		static void SubmitFullscreenQuad(const Ref<CMaterial>& spMaterial);

		static const Ref<CTexture2D>& GetColorTexture(const glm::vec3& vecColor, const TextureSpecification_t& textureSpec) { 
			return CRenderer::GetColorTexture(glm::vec4(vecColor, 1.0f), textureSpec); 
		};
		static const Ref<CTexture2D>& GetColorTexture(const glm::vec4& vecColor, const TextureSpecification_t& textureSpec);
		static const Ref<CShader> GetMeshShader();
	public:
		struct Statistics_t
		{
			uint32_t m_nDrawCalls = 0;
			uint32_t m_nVertexCount = 0;
			uint32_t m_nIndexCount = 0;
		};

		static void ResetStats();
		static Statistics_t& GetStats();
	private:
		static CRenderCommandQueue& GetRenderCommandQueue();
	};

}