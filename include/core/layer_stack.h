#pragma once

#include "core/base.h"
#include "core/layer.h"

namespace penumbra
{
	class CLayerStack
	{
	public:
		CLayerStack() = default;
		~CLayerStack();

		void PushLayer(CLayer* pLayer);
		void PushOverlay(CLayer* pOverlay);
		void PopLayer(CLayer* pLayer);
		void PopOverlay(CLayer* pOverlay);

		std::vector<CLayer*>::iterator begin() { return m_vecpLayers.begin(); }
		std::vector<CLayer*>::iterator end() { return m_vecpLayers.end(); }
		std::vector<CLayer*>::reverse_iterator rbegin() { return m_vecpLayers.rbegin(); }
		std::vector<CLayer*>::reverse_iterator rend() { return m_vecpLayers.rend(); }

		std::vector<CLayer*>::const_iterator begin() const { return m_vecpLayers.begin(); }
		std::vector<CLayer*>::const_iterator end()	const { return m_vecpLayers.end(); }
		std::vector<CLayer*>::const_reverse_iterator rbegin() const { return m_vecpLayers.rbegin(); }
		std::vector<CLayer*>::const_reverse_iterator rend() const { return m_vecpLayers.rend(); }
	private:
		std::vector<CLayer*> m_vecpLayers;
		uint32_t m_nLayerInsertIndex = 0;
	};
}