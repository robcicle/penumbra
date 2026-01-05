#pragma once

#include "renderer/shader_type.h"
#include "renderer/constant_buffer.h"

namespace penumbra
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	struct ShaderUVariable_t 
	{
		std::string m_Name = "Unknown";
		ShaderDataType m_Type = ShaderDataType::None;
		uint32_t m_nOffset = 0;
		uint32_t m_nSize = 0;
	};

	struct ShaderTexture_t 
	{
		std::string m_Name = "Unknown";
		uint32_t m_nBinding = 0;
		// TO-DO: Add type (2D, Cube, etc.)
	};

	struct ShaderUBuffer_t 
	{
		std::string m_Name = "Unknown";
		uint32_t m_nBinding = 0;
		uint32_t m_nSize = 0;
		std::vector<ShaderUVariable_t> m_vecVariables;
		std::vector<ShaderTexture_t> m_vecTextures;
		bool m_bIsValid = false;
	};

	class CShader
	{
	public:
		CShader(const std::string& filePath);
		CShader(const std::string& name, const std::string& fileSource);
		~CShader() = default;

		void Bind() const;
		void Unbind() const;

		void DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const;

		const std::string& GetName() const { return m_Name; }
		const ShaderUBuffer_t& GetMaterialBuffer() const { return m_MaterialBuffer; }
		ShaderUBuffer_t& GetMaterialBuffer() { return m_MaterialBuffer; }

		ID3DBlob* GetVertexBlob() const;
	public:
		static void AddGlobalDefine(ShaderType stage, const std::string& name, const std::string& value);
		static void ClearGlobalDefines(ShaderType stage);
		static void OutputShaderInfo(ShaderUBuffer_t& matBuffer);
	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<ShaderType, std::string> PreProcess(const std::string& shaderSource);

		void CompileOrGetD3D11Binaries(const std::unordered_map<ShaderType, std::string>& shaderSources);
		void CreateResources();
		void Reflect(ShaderType stage, const ComPtr<ID3DBlob>& spShaderBlob);
	private:
		std::string m_FilePath;
		std::string m_Name;

		ShaderUBuffer_t m_MaterialBuffer;

		std::unordered_map<ShaderType, ComPtr<ID3DBlob>> m_D3D11Blobs;
		std::unordered_map<ShaderType, ComPtr<ID3D11DeviceChild>> m_Shaders;

		ComPtr<ID3D11DeviceContext> m_spContext;
	};

	class CShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<CShader>& spShader);
		void Add(const Ref<CShader>& spShader);
		Ref<CShader> Load(const std::string& filePath);
		Ref<CShader> Load(const std::string& name, const std::string& filePath);

		Ref<CShader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<CShader>> m_Shaders;
	};
}