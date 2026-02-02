#include "ppch.h"
#include "imgui/imgui_layer.h"

#include "core/application.h"
#include "utility/platform_utils.h"

namespace penumbra
{
	CImGuiLayer::CImGuiLayer()
		: CLayer("ImGuiLayer")
	{
	}

	void CImGuiLayer::OnAttach()
	{
		PENUMBRA_PROFILE_FUNC();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		HWND hwnd = CApplication::Get().GetWindow().GetHWND();
		ImGui_ImplWin32_Init(hwnd);

		CGraphicsContext* pGraphicsContext = CApplication::Get().GetWindow().GetGraphicsContext();
		ComPtr<ID3D11DeviceContext> spContext = pGraphicsContext->GetContext();
		ComPtr<ID3D11Device> spDevice = pGraphicsContext->GetDevice();

		ImGui_ImplDX11_Init(spDevice.Get(), spContext.Get());
	}

	void CImGuiLayer::OnDetach()
	{
		PENUMBRA_PROFILE_FUNC();

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();

		ImGui::DestroyContext();
	}

	void CImGuiLayer::OnEvent(CEvent& e)
	{
		PENUMBRA_PROFILE_FUNC();

		if (m_bBlockEvents) {
			ImGuiIO& io = ImGui::GetIO();
			e.m_bHandled |= e.IsInCategory(EVENT_CATEGORY_MOUSE) & io.WantCaptureMouse;
			e.m_bHandled |= e.IsInCategory(EVENT_CATEGORY_KEYBOARD) & io.WantCaptureKeyboard;
		}
	}

	void CImGuiLayer::Begin()
	{
		PENUMBRA_PROFILE_FUNC();

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();
	}

	void CImGuiLayer::End()
	{
		PENUMBRA_PROFILE_FUNC();

		ImGuiIO& io = ImGui::GetIO();
		CApplication& app = CApplication::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
		
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	uint32_t CImGuiLayer::GetActiveWidgetID() const
	{
		return GImGui->ActiveId;
	}
}