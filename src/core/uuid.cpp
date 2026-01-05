#include "ppch.h"
#include "core/uuid.h"

namespace penumbra
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	CUUID::CUUID()
		: m_nUUID(s_UniformDistribution(s_Engine))
	{
	}

	CUUID::CUUID(uint64_t nUuid)
		: m_nUUID(nUuid)
	{
	}
}