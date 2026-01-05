#pragma once

#include "core/base.h"
#include "core/log.h"

#ifdef PENUMBRA_ENABLE_ASSERTS
	#define PENUMBRA_INTERNPENUMBRA_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { PENUMBRA##type##ERROR(msg, __VA_ARGS__); PENUMBRA_DEBUGBREAK(); } }
	#define PENUMBRA_INTERNPENUMBRA_ASSERT_WITH_MSG(type, check, ...) PENUMBRA_INTERNPENUMBRA_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define PENUMBRA_INTERNPENUMBRA_ASSERT_NO_MSG(type, check) PENUMBRA_INTERNPENUMBRA_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", PENUMBRA_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define PENUMBRA_INTERNPENUMBRA_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define PENUMBRA_INTERNPENUMBRA_ASSERT_GET_MACRO(...) PENUMBRA_EXPAND_MACRO( PENUMBRA_INTERNPENUMBRA_ASSERT_GET_MACRO_NAME(__VA_ARGS__, PENUMBRA_INTERNPENUMBRA_ASSERT_WITH_MSG, PENUMBRA_INTERNPENUMBRA_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define PENUMBRA_ASSERT(...) PENUMBRA_EXPAND_MACRO( PENUMBRA_INTERNPENUMBRA_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define PENUMBRA_CORE_ASSERT(...) PENUMBRA_EXPAND_MACRO( PENUMBRA_INTERNPENUMBRA_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define PENUMBRA_ASSERT(...)
	#define PENUMBRA_CORE_ASSERT(...)
#endif

#ifdef PENUMBRA_ENABLE_VERIFY

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define PENUMBRA_INTERNPENUMBRA_VERIFY_IMPL(type, check, msg, ...) { if(!(check)) { PENUMBRA##type##ERROR(msg, __VA_ARGS__); PENUMBRA_DEBUGBREAK(); } }
#define PENUMBRA_INTERNPENUMBRA_VERIFY_WITH_MSG(type, check, ...) PENUMBRA_INTERNPENUMBRA_VERIFY_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define PENUMBRA_INTERNPENUMBRA_VERIFY_NO_MSG(type, check) PENUMBRA_INTERNPENUMBRA_VERIFY_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", PENUMBRA_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define PENUMBRA_INTERNPENUMBRA_VERIFY_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define PENUMBRA_INTERNPENUMBRA_VERIFY_GET_MACRO(...) PENUMBRA_EXPAND_MACRO( PENUMBRA_INTERNPENUMBRA_VERIFY_GET_MACRO_NAME(__VA_ARGS__, PENUMBRA_INTERNPENUMBRA_VERIFY_WITH_MSG, PENUMBRA_INTERNPENUMBRA_VERIFY_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define PENUMBRA_VERIFY(...) PENUMBRA_EXPAND_MACRO( PENUMBRA_INTERNPENUMBRA_VERIFY_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define PENUMBRA_CORE_VERIFY(...) PENUMBRA_EXPAND_MACRO( PENUMBRA_INTERNPENUMBRA_VERIFY_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define PENUMBRA_VERIFY(...)
#define PENUMBRA_CORE_VERIFY(...)
#endif