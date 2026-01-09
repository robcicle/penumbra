#pragma once

#include "core/platform_detection.h"

#ifdef PENUMBRA_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <filesystem>
#include <iostream>
#include <utility>
#include <algorithm>
#include <map>
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <ostream>
#include <future>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <cstdint>
#include <random>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "math/math.h"

#include "core/base.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/bundled/std.h>
#pragma warning(pop)

#include "debug/profiler.h"
#include "core/log.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;
#include <d3d11_2.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>