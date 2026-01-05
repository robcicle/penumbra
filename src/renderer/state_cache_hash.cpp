#include "ppch.h"
#include "renderer/state_cache_hash.h"

namespace penumbra
{
    size_t HashMemory(const void* pData, size_t nSize)
    {
        return 0;
        //return static_cast<size_t>(XXH3_64bits(pData, nSize));
    }
}