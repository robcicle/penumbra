#include "ppch.h"
#include "core/layer_stack.h"

namespace penumbra
{
	CLayerStack::~CLayerStack()
	{
		PENUMBRA_PROFILE_FUNC();

		for (CLayer* pLayer : m_vecpLayers) {
			pLayer->OnDetach();
			delete pLayer;
		}
	}

	void CLayerStack::PushLayer(CLayer* pLayer)
	{
		PENUMBRA_PROFILE_FUNC();

		m_vecpLayers.emplace(m_vecpLayers.begin() + m_nLayerInsertIndex, pLayer);
		pLayer->OnAttach();
		m_nLayerInsertIndex++;
	}

	void CLayerStack::PushOverlay(CLayer* pOverlay)
	{
		PENUMBRA_PROFILE_FUNC();

		m_vecpLayers.emplace_back(pOverlay);
		pOverlay->OnAttach();
	}

	void CLayerStack::PopLayer(CLayer* pLayer)
	{
		PENUMBRA_PROFILE_FUNC();

		auto it = std::find(m_vecpLayers.begin(), m_vecpLayers.end(), pLayer);
		if (it != m_vecpLayers.end()) {
			m_vecpLayers.erase(it);
			pLayer->OnDetach();
			if (m_nLayerInsertIndex > 0) {
				m_nLayerInsertIndex--;
			}
		}
	}

	void CLayerStack::PopOverlay(CLayer* pOverlay)
	{
		PENUMBRA_PROFILE_FUNC();

		auto it = std::find(m_vecpLayers.begin(), m_vecpLayers.end(), pOverlay);
		if (it != m_vecpLayers.end()) {
			m_vecpLayers.erase(it);
			pOverlay->OnDetach();
		}
	}
}