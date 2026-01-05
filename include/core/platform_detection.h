#pragma once

// Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define PENUMBRA_PLATFORM_WINDOWS
		
		#ifdef PENUMBRA_RENDERER_WANTS_OPENGL
			#define PENUMBRA_RENDERER_SUPPORTS_OPENGL
		#endif
		#ifdef PENUMBRA_RENDERER_WANTS_DIRECTX11
			#define PENUMBRA_RENDERER_SUPPORTS_DIRECTX11
		#endif
		#ifdef PENUMBRA_RENDERER_WANTS_VULKAN
			#define PENUMBRA_RENDERER_SUPPORTS_VULKAN
		#endif

	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	 * so we must check all of them (in this order)
	 * to ensure that we're running on MAC
	 * and not some other Apple platform */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define PENUMBRA_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define PENUMBRA_PLATFORM_MACOS

		#ifdef PENUMBRA_RENDERER_WANTS_METAL
			#define PENUMBRA_RENDERER_SUPPORTS_METAL
		#endif

		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define PENUMBRA_PLATFORM_ANDROID

	#ifdef PENUMBRA_RENDERER_WANTS_OPENGL
		#define PENUMBRA_RENDERER_SUPPORTS_OPENGL
	#endif
	#ifdef PENUMBRA_RENDERER_WANTS_VULKAN
		#define PENUMBRA_RENDERER_SUPPORTS_VULKAN
	#endif

	#error "Android is not supported!"
#elif defined(__linux__)
	#define PENUMBRA_PLATFORM_LINUX

	#ifdef PENUMBRA_RENDERER_WANTS_OPENGL
		#define PENUMBRA_RENDERER_SUPPORTS_OPENGL
	#endif
	#ifdef PENUMBRA_RENDERER_WANTS_VULKAN
		#define PENUMBRA_RENDERER_SUPPORTS_VULKAN
	#endif

#else
	/* Unknown compiler/platform */
	#error "Unknown platform! Please submit a request at https://github.com/robcicle/AntiLight/issues/4 if you want this platform to be supported!"
#endif // End of platform detection