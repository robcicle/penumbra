#pragma once

namespace penumbra
{
	class CUUID
	{
	public:
		CUUID();
		CUUID(uint64_t nUUID);
		CUUID(const CUUID&) = default;

		operator uint64_t() const { return m_nUUID; }
	private:
		uint64_t m_nUUID;
	};
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<penumbra::CUUID>
	{
		std::size_t operator()(const penumbra::CUUID& nUuid) const
		{
			return(uint64_t)nUuid;
		}
	};
}