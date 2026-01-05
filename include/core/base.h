#pragma once

#define PENUMBRA_NAME "penumbra"
#define PENUMBRA_VERSION "0.0.1a"
#define PENUMBRA_COMMIT_HASH "indev"

#include "core/platform_detection.h"

#if defined(PENUMBRA_PLATFORM_WINDOWS)
	#define PENUMBRA_DEBUGBREAK() __debugbreak()
#elif defined(PENUMBRA_PLATFORM_LINUX)
	#include <signal.h>
	#define PENUMBRA_DEBUGBREAK() raise(SIGTRAP)
#else
	#error "Platform doesn't support debugbreak yet!"
#endif

#ifdef PENUMBRA_DEBUG
	#define PENUMBRA_ENABLE_ASSERTS
#endif

#ifndef PENUMBRA_DIST
	#define PENUMBRA_ENABLE_VERIFY
#endif

#define PENUMBRA_EXPAND_MACRO(x) x
#define PENUMBRA_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define PENUMBRA_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace penumbra
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
	
	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#include "core/log.h"
#include "core/assert.h"
#include "core/constants.h"