#pragma once

#include "core/layer.h"

namespace penumbra
{
	class CImGuiLayer : public CLayer
	{
	public:
		CImGuiLayer();
		~CImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(CEvent& e) override;

		void Begin();
		void End();

		void BlockEvents(bool bBlock) { m_bBlockEvents = bBlock; }

		uint32_t GetActiveWidgetID() const;
	private:
		bool m_bBlockEvents = true;
	};
}