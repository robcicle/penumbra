#pragma once

#include "core/base.h"
#include "events/event.h"

#include "core/time.h"

namespace penumbra
{
	class CLayer
	{
	public:
		CLayer(const std::string& debugName = "Layer");
		virtual ~CLayer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(CTime time) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(CEvent& event) {}

		const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}