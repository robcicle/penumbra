#pragma once

#include "renderer/state_cache_hash.h"

namespace penumbra
{
    template<typename StateDesc, typename StateObject>
    class CStateCache
    {
    public:
        using CreateFunc = std::function<ComPtr<StateObject>(const StateDesc&)>;

        CStateCache(CreateFunc createFunction)
            : m_fnCreateFunction(createFunction) {
        }
        ~CStateCache() = default;

        ComPtr<StateObject> GetOrCreate(const StateDesc& desc)
        {
            size_t hash = Hash(desc);
            auto it = m_Cache.find(hash);
            if (it != m_Cache.end()) {
                return it->second;
            }

            auto newState = m_fnCreateFunction(desc);
            m_Cache[hash] = newState;
            return newState;
        }

        void Clear()
        {
            m_Cache.clear();
        }

    private:
        std::unordered_map<size_t, ComPtr<StateObject>> m_Cache;
        CreateFunc m_fnCreateFunction;

        size_t Hash(const StateDesc& desc) const
        {
            static_assert(std::is_trivially_copyable<StateDesc>::value, "StateDesc must be trivially copyable");

            return HashMemory(&desc, sizeof(StateDesc));
        }
    };
}
