#pragma once

#include "core/base.h"

#include "core/window.h"

#include "core/layer_stack.h"
#include "imgui/imgui_layer.h"

#include "events/application_event.h"
#include "renderer/graphics_context.h"
#include "renderer/shader.h"

#include "core/time.h"

int main(int argc, char** argv);

namespace penumbra
{
	struct ApplicationCommandLineArgs_t
	{
		int m_nCount = 0;
		char** m_ppArgs = nullptr;

		const char* operator[](int index) const
		{
			PENUMBRA_CORE_ASSERT(index < m_nCount);
			return m_ppArgs[index];
		}
	};

	struct ApplicationSpecification_t
	{
		std::string m_Name = "Application";
		std::string m_WorkingDirectory;
		ApplicationCommandLineArgs_t m_CommandLineArgs;

		uint32_t m_nTargetFPS = 0; // <30 means no limit
		WindowProps_t m_WinProps;
	};

	class CApplication
	{
	public:
		CApplication(const ApplicationSpecification_t specification = ApplicationSpecification_t());
		virtual ~CApplication();

		void OnEvent(CEvent& e);
		
		void PushLayer(CLayer* pLayer);
		void PopLayer(CLayer* pLayer);
		void PushOverlay(CLayer* overlay);

		const CWindow& GetWindow() const { return *m_spWindow; }
		CWindow& GetWindow() { return *m_spWindow; }

		void Run();
		void Close();
		
		CImGuiLayer* GetImGuiLayer() { return m_pImGuiLayer; }

		static CApplication& Get() { return *s_pInstance; }

		const ApplicationSpecification_t& GetSpecification() const { return m_Specification; }
		
		void SubmitToMainThread(const std::function<void()>& function);

		void SetTargetFPS(float targetFPS)
		{
			m_Specification.m_nTargetFPS = targetFPS;
			if (targetFPS < 30.0f)
				m_flTargetFrameTime = 0.0f;
			else
				m_flTargetFrameTime = 1.0f / targetFPS;
		}
	public:
		struct Statistics_t
		{
			float m_flCpuTime = 0.0f;
			float m_flGpuTime = 0.0f;
		};

		Statistics_t& GetStatistics() { return m_Statistics; }
	private:
		bool OnWindowClose(CWindowCloseEvent& e);
		bool OnWindowResize(CWindowResizeEvent& e);

		void RenderImGui();

		void ExecuteMainThreadQueue();
	private:
		ApplicationSpecification_t m_Specification;
		Scope<CWindow> m_spWindow;
		CImGuiLayer* m_pImGuiLayer;
		bool m_bRunning = true;
		bool m_bMinimized = false;
		CLayerStack m_LayerStack;
		float m_flLastFrameTime = 0.0f;
		float m_flTargetFrameTime = 0.0f;
		Statistics_t m_Statistics;

		std::vector<std::function<void()>> m_vecfnMainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
	private:
		static CApplication* s_pInstance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in CLIENT
	CApplication* CreateApplication(ApplicationCommandLineArgs_t args);
}