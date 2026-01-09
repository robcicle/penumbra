#pragma once

namespace penumbra
{
	// To-do: Split into namespaces per system/class

	// Application constants
	constexpr float kSpinThreshold = 0.001f;
	constexpr uint32_t kSpinSleepMicroseconds = 500;
	constexpr const char* kFrameProfileName = "Frame";
	constexpr const char* kLayerStackUpdateName = "LayerStack::OnUpdate";
	constexpr const char* kLayerStackImGuiRenderName = "LayerStack::OnImGuiRender";

	// Log constants
	constexpr const char* kLogSinkColorPattern = "%^[%T] %n: %v%$";
	constexpr const char* kLogSinkFilePattern = "[%T] [%l] %n: %v";
	constexpr const char* kCoreLoggerName = "PENUMBRA";
	constexpr const char* kClientLoggerName = "APP";

	// Window constants
	constexpr const char* kWindowClassName = "PenumbraWindowClass";

	// EditorCamera constants
	constexpr float kPanViewportDivisor = 1000.0f;
	constexpr float kPanSpeedMax = 2.4f;
	constexpr float kPanCoeffA = 0.0366f;
	constexpr float kPanCoeffB = -0.1778f;
	constexpr float kPanCoeffC = 0.3021f;
	constexpr float kMouseSensitivity = 0.003f;
	constexpr float kMoveSpeedMin = 1.0f;
	constexpr float kMoveSpeedBoostMultiplier = 3.0f;
	constexpr float kRotationSpeed = 0.6f;
	constexpr float kPitchLimitDegrees = 89.0f;
	constexpr float kZoomSpeed = 1.0f;

	// Framebuffer constants
	constexpr uint32_t kMaxFramebufferSize = 8192;
	constexpr uint32_t kDefaultFramebufferMipLevels = 1;
	constexpr uint32_t kDefaultFramebufferArraySize = 1;
	constexpr uint32_t kDefaultFramebufferMipSlice = 0;
	constexpr uint32_t kDefaultFramebufferMostDetailedMip = 0;
	constexpr uint32_t kDepthStencilClearValue = 0xffffffff;

	// GraphicsContext constants
	constexpr const char* kGraphicsCreateDXGIFactoryProfileName = "CreateDXGIFactory";
	constexpr uint32_t kVendorIdNVIDIA	= 0x10DE;	// NVIDIA Vendor ID
	constexpr uint32_t kVendorIdAMD		= 0x1002;	// AMD Vendor ID
	constexpr uint32_t kVendorIdIntel	= 0x8086;	// Intel Vendor ID
	constexpr const char* kVendorNVIDIA		= "NVIDIA Corporation";
	constexpr const char* kVendorAMD		= "AMD";
	constexpr const char* kVendorIntel		= "Intel";
	constexpr const char* kVendorUnknown	= "UnknownVendor";
	constexpr size_t kOneMiB = 1024ull * 1024ull;	// One MiB in bytes
	constexpr uint32_t kQPCIntervalDivisor = 10;	// Divisor for converting QPC to milliseconds

	// Mesh constants
	constexpr const char* kLayoutPositionSemantic = "POSITION";
	constexpr const char* kLayoutNormalSemantic = "NORMAL";
	constexpr const char* kLayoutTangentSemantic = "TANGENT";
	constexpr const char* kLayoutTexCoordSemantic = "TEXCOORD";

	// OrthographicCamera constants
	constexpr float kZNear = 0.0f;
	constexpr float kZFar = 2048.0f;
	constexpr float kTranslationSpeedFactor = 0.00186f;
	constexpr float kMinZoomLevel = 0.1f;

	// RenderCommandQueue constants
	constexpr size_t kRenderCommandQueueBufferSize = 10 * 1024 * 1024; // 10mb buffer

	// Renderer constants
	constexpr uint32_t kMaxInstances = 2048;
	constexpr const char* kMeshShaderName = "VSMesh.hlsl";
	
	// RendererAPI constants
	constexpr uint32_t kDefaultSwapChainBufferCount = 2;
	constexpr uint8_t kClearStencil = 0;
	constexpr float kDefaultClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr uint32_t kSampleMask = 0xffffffff;

	// Shader constants
	constexpr const char* kShaderDefineDirectX = "#define DIRECTX\n";
	constexpr const char* kShaderCacheDirectory = "assets/cache/shader/d3d11";

	constexpr const char* kShaderCacheExtVertex = ".cached_d3d11.vertex";
	constexpr const char* kShaderCacheExtPixel = ".cached_d3d11.pixel";
	constexpr const char* kShaderCacheExtGeometry = ".cached_d3d11.geo";
	constexpr const char* kShaderCacheExtCompute = ".cached_d3d11.compute";

	constexpr const char* kShaderModelVertex = "vs_5_0";
	constexpr const char* kShaderModelPixel = "ps_5_0";
	constexpr const char* kShaderModelGeometry = "gs_5_0";
	constexpr const char* kShaderModelCompute = "cs_5_0";

	constexpr const char* kShaderTypeToken = "#type";
	constexpr uint32_t kShaderMaterialCBufferSlot = 2;

	// Texture constants
	constexpr uint32_t kTextureMipLevels = 1;
	constexpr uint32_t kTextureArraySize = 1;
	constexpr uint32_t kTextureSampleCount = 1;
	constexpr uint32_t kTextureCPUAccessFlags = 0;
	constexpr uint32_t kTextureMiscFlags = 0;

	constexpr uint32_t kBPPR8		= 1;
	constexpr uint32_t kBPPRG8		= 2;
	constexpr uint32_t kBPPRGBA8	= 4;
	constexpr uint32_t kBPPRGBA16F	= 8;
	constexpr uint32_t kBPPRGB32F	= 12;
	constexpr uint32_t kBPPRGBA32F	= 16;

	constexpr uint32_t kChannelR	= 1;
	constexpr uint32_t kChannelRG	= 2;
	constexpr uint32_t kChannelRGB	= 3;
	constexpr uint32_t kChannelRGBA	= 4;

	constexpr uint8_t kDefaultAlpha		= 255;
	constexpr uint32_t kSrcChannelsRGB	= 3;
	constexpr uint32_t kDstChannelsRGBA	= 4;

	constexpr uint32_t kFloat32Bytes	= 4;
	constexpr uint32_t kRGBAChannels	= 4;
	constexpr uint32_t kRGBChannels		= 3;

	constexpr uint32_t kBoxLeft		= 0;
	constexpr uint32_t kBoxTop		= 0;
	constexpr uint32_t kBoxFront	= 0;
	constexpr uint32_t kBoxBack		= 1;

	// Reusable constants
	constexpr float kMinDepth = 0.0f;
	constexpr float kMaxDepth = 1.0f;
}