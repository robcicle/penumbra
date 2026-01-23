#pragma once

namespace penumbra 
{
#ifndef PENUMBRA_ENABLE_PROFILER
	#define PENUMBRA_ENABLE_PROFILER 0
#endif

#if PENUMBRA_ENABLE_PROFILER
	#define TRACY_ENABLE

	#define PENUMBRA_PROFILE_MARK_FRAME				FrameMark;
	// Use PENUMBRA_PROFILE_FUNC ONLY at the top of a function
	// Use PENUMBRA_PROFILE_SCOPE / PENUMBRA_PROFILE_SCOPE_DYNAMIC for an inner scope
	#define PENUMBRA_PROFILE_FUNC(...)				ZoneScoped __VA_OPT__(; ZoneName(__VA_ARGS__, strlen(__VA_ARGS__)))
	#define PENUMBRA_PROFILE_SCOPE(...)				PENUMBRA_PROFILE_FUNC(__VA_ARGS__)
	#define PENUMBRA_PROFILE_SCOPE_DYNAMIC(NAME)	ZoneScoped; ZoneName(NAME, strlen(NAME))
#else
	#undef TRACY_ENABLE

	#define PENUMBRA_PROFILE_MARK_FRAME
	#define PENUMBRA_PROFILE_FUNC(...)
	#define PENUMBRA_PROFILE_SCOPE(...)
	#define PENUMBRA_PROFILE_SCOPE_DYNAMIC(NAME)
#endif
}

#if defined(TRACY_ENABLE)
#include <tracy/Tracy.hpp>
#include <tracy/TracyD3D11.hpp>
#endif