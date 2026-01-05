#pragma once

namespace penumbra
{
	class CTime
	{
	public:
		CTime(float flTime = 0.0f)
			: m_flTime(flTime)
		{

		}

		operator float() const { return m_flTime; }

		float GetSeconds() const { return m_flTime; }
		float GetMilliseconds() const { return m_flTime * 1000.0f; }
	private:
		float m_flTime;
	};
}