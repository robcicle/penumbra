#include "ppch.h"
#include "core/application.h"

#include "core/input.h"
#include "core/key_codes.h"

#include "events/event.h"
#include "events/key_event.h"

#include "renderer/render_command.h"

#include "core/window.h"
#include "utility/platform_utils.h"
#include "core/timer.h"

namespace penumbra
{
    static void SleepPrecise(float seconds)
    {
        if (seconds <= 0.0f)
            return;

        float endTime = CPlatformUtils::GetTime() + seconds;

        // Sleep most of the time
        while (CPlatformUtils::GetTime() + kSpinThreshold < endTime) {
            std::this_thread::sleep_for(std::chrono::microseconds(kSpinSleepMicroseconds));
        }

        // Spin-wait remaining time
        while (CPlatformUtils::GetTime() < endTime) {
            std::this_thread::yield();
        }
    }

    CApplication* CApplication::s_pInstance = nullptr;

    CApplication::CApplication(const ApplicationSpecification_t specification)
        : m_Specification(specification)
    {
        PENUMBRA_PROFILE_FUNC();
		
        PENUMBRA_CORE_ASSERT(!s_pInstance, "Application already exists!");
        s_pInstance = this;

        if (!m_Specification.m_WorkingDirectory.empty()) {
            std::filesystem::current_path(m_Specification.m_WorkingDirectory);
        }
        else {
			m_Specification.m_WorkingDirectory = std::filesystem::current_path().string();
        }

        CRenderCommand::Create();

		m_spWindow = CreateScope<CWindow>(m_Specification.m_WinProps);
        m_spWindow->SetEventCallback(PENUMBRA_BIND_EVENT_FN(CApplication::OnEvent));

        CRenderer::Init();
        CRenderer::WaitAndRender();
        
        m_pImGuiLayer = new CImGuiLayer();
        PushOverlay(m_pImGuiLayer);

		SetTargetFPS(m_Specification.m_nTargetFPS);
    }

    CApplication::~CApplication()
    {
        PENUMBRA_PROFILE_FUNC();

        CRenderer::Shutdown();
    }

    void CApplication::SubmitToMainThread(const std::function<void()>& function)
    {
        PENUMBRA_PROFILE_FUNC();

        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        m_vecfnMainThreadQueue.emplace_back(function);
    }

    void CApplication::Run()
    {;
		PENUMBRA_PROFILE_FUNC();  // Main application loop

		// Initialize timing
        double nextFrameTime = CPlatformUtils::GetTime();

        while (m_bRunning) {
            PENUMBRA_PROFILE_SCOPE(kFrameProfileName);

			// Mark frame start time
            const double frameStartTime = CPlatformUtils::GetTime();

			// Calculate delta time
            const double currentTime = CPlatformUtils::GetTime();
            const float deltaTime = static_cast<float>(currentTime - m_flLastFrameTime);
			// Update last frame time
            m_flLastFrameTime = currentTime;

			// Execute any functions queued for the main thread
            ExecuteMainThreadQueue();

            if (!m_bMinimized) {
				// Bind and clear the default framebuffer
                CRenderCommand::Bind();
                CRenderCommand::Clear();
                {
					// Update all layers
                    PENUMBRA_PROFILE_SCOPE(kLayerStackUpdateName);
                    CTimer timer;

                    for (CLayer* layer : m_LayerStack) {
                        layer->OnUpdate(deltaTime);
                    }

					m_Statistics.m_flCPUUpdateTime = timer.ElapsedMillis();
                }

				// Render ImGui on top of everything
                // The actual rendering commands will be submitted to the
				// RenderQueue and executed later
                RenderImGui();

                {
					// Render all submitted render commands
                    CTimer timer;
                    CRenderer::WaitAndRender();
					m_Statistics.m_flGPURenderTime = timer.ElapsedMillis();
                }
            }

			// Update the window (swap buffers and poll events)
            m_spWindow->OnUpdate();

			// Frame rate capping
            if (m_flTargetFrameTime > 0.0f) {
				// Calculate next frame time
                nextFrameTime += m_flTargetFrameTime;

				// Sleep until the next frame time
			    const double now = CPlatformUtils::GetTime();
                const double sleepDuration = nextFrameTime - now;
                
				// If we're running behind, skip sleeping
                if (sleepDuration > 0.0f) 
                    SleepPrecise(static_cast<float>(sleepDuration));
                else
					nextFrameTime = now;
            }

			// Calculate frame time
            const double frameEndTime = CPlatformUtils::GetTime();
            m_Statistics.m_flFrameTime = static_cast<float>((frameEndTime - frameStartTime) * 1000.0);

            PENUMBRA_PROFILE_MARK_FRAME;  // Marks frame end for profiler
        }
    }

    void CApplication::OnEvent(CEvent& e)
    {
        PENUMBRA_PROFILE_FUNC();

        CEventDispatcher dispatcher(e);
        dispatcher.Dispatch<CWindowCloseEvent>(PENUMBRA_BIND_EVENT_FN(CApplication::OnWindowClose));
        dispatcher.Dispatch<CWindowResizeEvent>(PENUMBRA_BIND_EVENT_FN(CApplication::OnWindowResize));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); it++) {
            if (e.m_bHandled) {
                break;
            }
            (*it)->OnEvent(e);
        }
    }

    void CApplication::PushLayer(CLayer* layer)
    {
        PENUMBRA_PROFILE_FUNC();

        m_LayerStack.PushLayer(layer);
    }

    void CApplication::PopLayer(CLayer* layer)
    {
        PENUMBRA_PROFILE_FUNC();

		m_LayerStack.PopLayer(layer);
    }

    void CApplication::PushOverlay(CLayer* overlay)
    {
        PENUMBRA_PROFILE_FUNC();

        m_LayerStack.PushOverlay(overlay);
    }

    void CApplication::Close()
    {
        PENUMBRA_PROFILE_FUNC();

        m_bRunning = false;
    }

    bool CApplication::OnWindowClose(CWindowCloseEvent& e)
    {
        PENUMBRA_PROFILE_FUNC();

        m_bRunning = false;
        return true;
    }

    bool CApplication::OnWindowResize(CWindowResizeEvent& e)
    {
        PENUMBRA_PROFILE_FUNC();

        if (e.GetWidth() == 0 || e.GetHeight() == 0) {
            m_bMinimized = true;
            return false;
        }

        m_bMinimized = false;
        CRenderer::OnWindowResize(e.GetWidth(), e.GetHeight());
        return false;
    }

    void CApplication::RenderImGui()
    {
        PENUMBRA_PROFILE_FUNC();

        RasterizerSpecification_t spec = CRenderCommand::GetRasterizerSpecification();

        CRenderer::Submit([this]() 
        {
            m_pImGuiLayer->Begin();
            {
                PENUMBRA_PROFILE_SCOPE(kLayerStackImGuiRenderName);

                for (CLayer* layer : m_LayerStack) {
                    layer->OnImGuiRender();
                }
            }
            m_pImGuiLayer->End();
        });
        
		CRenderCommand::SetRasterizerState(spec);
    }

    void CApplication::ExecuteMainThreadQueue()
    {
        PENUMBRA_PROFILE_FUNC();

        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (auto& func : m_vecfnMainThreadQueue) {
            func();
        }

        m_vecfnMainThreadQueue.clear();
    }
}