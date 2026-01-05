#include "ppch.h"
#include "renderer/shader.h"

#include "core/application.h"
#include "core/timer.h"
#include "renderer/renderer.h"

namespace penumbra
{
	namespace Utils
	{
		// Converts ShaderDataType enum to string
		static std::string DataTypeToString(ShaderDataType type)
		{
			switch (type)
			{
			case ShaderDataType::Float:     return "Float";
			case ShaderDataType::Float2:    return "Float2";
			case ShaderDataType::Float3:    return "Float3";
			case ShaderDataType::Float4:    return "Float4";
			case ShaderDataType::Mat3:      return "Mat3";
			case ShaderDataType::Mat4:      return "Mat4";
			case ShaderDataType::Int:       return "Int";
			case ShaderDataType::Int2:      return "Int2";
			case ShaderDataType::Int3:      return "Int3";
			case ShaderDataType::Int4:      return "Int4";
			case ShaderDataType::Bool:      return "Bool";
			}

            return "None";
		}

		// Resolves #include directives in shader source code
        static std::string ResolveIncludesInternal(
            const std::string& source,
            const std::filesystem::path& parentDir,
            std::unordered_set<std::string>& includedFiles)
        {
            std::stringstream output;
            std::istringstream input(source);
            std::string line;

            while (std::getline(input, line)) {
                if (line.starts_with("#include")) {

                    size_t first = line.find_first_of("\"<");
                    size_t last = line.find_last_of("\">");

                    std::string includePath = line.substr(first + 1, last - first - 1);

                    std::filesystem::path fullPath = parentDir / includePath;
                    std::filesystem::path canonical = std::filesystem::weakly_canonical(fullPath);
                    std::string canonicalStr = canonical.string();

                    // GLOBAL check, not per-call
                    if (includedFiles.contains(canonicalStr))
                        continue;

                    includedFiles.insert(canonicalStr);

                    std::ifstream file(canonical);
                    PENUMBRA_CORE_ASSERT(file.is_open(), "Shader: Failed to open include file!");

                    std::stringstream contents;
                    contents << file.rdbuf();

                    output << "// Begin include: " << canonicalStr << "\n";
                    output << ResolveIncludesInternal(contents.str(), canonical.parent_path(), includedFiles);
                    output << "// End include: " << canonicalStr << "\n";

                }
                else {
                    output << line << "\n";
                }
            }

            return output.str();
        }

        static std::string ResolveIncludes(
            const std::string& source,
            const std::filesystem::path& parentDir)
        {
            std::unordered_set<std::string> includedFiles;
            return ResolveIncludesInternal(source, parentDir, includedFiles);
        }

		// Maps D3D shader type to ShaderDataType enum
        ShaderDataType MapShaderType(const D3D11_SHADER_TYPE_DESC& t)
        {
			// Handle scalar types
            if (t.Class == D3D_SVC_SCALAR) {
                if (t.Type == D3D_SVT_FLOAT) return ShaderDataType::Float;
                if (t.Type == D3D_SVT_INT)   return ShaderDataType::Int;
                if (t.Type == D3D_SVT_BOOL)  return ShaderDataType::Bool;
            }

			// Handle vector types
            if (t.Class == D3D_SVC_VECTOR) {
                if (t.Type == D3D_SVT_FLOAT) {
                    switch (t.Columns)
                    {
                    case 2: return ShaderDataType::Float2;
                    case 3: return ShaderDataType::Float3;
                    case 4: return ShaderDataType::Float4;
                    }
                }
                if (t.Type == D3D_SVT_INT) {
                    switch (t.Columns)
                    {
                    case 2: return ShaderDataType::Int2;
                    case 3: return ShaderDataType::Int3;
                    case 4: return ShaderDataType::Int4;
                    }
                }
            }

			// Handle matrix types
            if ((t.Class == D3D_SVC_MATRIX_COLUMNS ||
                t.Class == D3D_SVC_MATRIX_ROWS) &&
                t.Type == D3D_SVT_FLOAT) {
                if (t.Rows == 3 && t.Columns == 3) return ShaderDataType::Mat3;
                if (t.Rows == 4 && t.Columns == 4) return ShaderDataType::Mat4;
            }

            return ShaderDataType::None;
        }

		// Converts string to ShaderType enum
        static ShaderType ShaderTypeFromString(const std::string& type)
        {
            if (type == "vertex")   return ShaderType::SHADER_TYPE_VERTEX;
            if (type == "pixel" || type == "fragment") return ShaderType::SHADER_TYPE_PIXEL;
            if (type == "geometry") return ShaderType::SHADER_TYPE_GEOMETRY;
			if (type == "compute") return ShaderType::SHADER_TYPE_COMPUTE;

            PENUMBRA_CORE_ASSERT(false, "Shader: Unknown shader type");
            return ShaderType::SHADER_TYPE_NONE;
        }

		// Maps ShaderType enum to D3D shader profile string
        static const char* ShaderStageToString(ShaderType stage)
        {
            switch (stage)
            {
                case ShaderType::SHADER_TYPE_VERTEX:    return kShaderModelVertex;
                case ShaderType::SHADER_TYPE_PIXEL:     return kShaderModelPixel;
                case ShaderType::SHADER_TYPE_GEOMETRY:  return kShaderModelGeometry;
                case ShaderType::SHADER_TYPE_COMPUTE:   return kShaderModelCompute;
            }
            PENUMBRA_CORE_ASSERT(false);
            return "";
        }

		// Gets the shader cache directory path
        static const char* GetCacheDirectory()
        {
            // TO-DO: make sure the assets directory is valid
            return kShaderCacheDirectory;
        }

		// Hashes the shader source code to create a unique identifier
        static std::string HashShaderSource(const std::string& src)
        {
			// Use std::hash to generate a hash of the source code
            std::hash<std::string> hasher;
            size_t hash = hasher(src);
			// Convert hash to hexadecimal string
            std::stringstream ss;
            ss << std::hex << hash;

            return ss.str();
        }

		// Creates the shader cache directory if it doesn't exist
        static void CreateCacheDirectoryIfNeeded()
        {
            std::string dir = GetCacheDirectory();
            if (!std::filesystem::exists(dir))
                std::filesystem::create_directories(dir);
        }

		// Gets the file extension for cached D3D11 shader binaries based on shader stage
        static const char* ShaderStageCachedD3D11FileExtension(ShaderType stage)
        {
            switch (stage)
            {
            case ShaderType::SHADER_TYPE_VERTEX:   return kShaderCacheExtVertex;
            case ShaderType::SHADER_TYPE_PIXEL:    return kShaderCacheExtPixel;
            case ShaderType::SHADER_TYPE_GEOMETRY: return kShaderCacheExtGeometry;
            case ShaderType::SHADER_TYPE_COMPUTE: return kShaderCacheExtCompute;
            }
            PENUMBRA_CORE_ASSERT(false);
            return "";
        }
	}

    static std::unordered_map<ShaderType, std::unordered_map<std::string, std::string>> s_GlobalDefines;

    CShader::CShader(const std::string& filePath)
        : m_FilePath(filePath)
    {
        PENUMBRA_PROFILE_FUNC();

        Utils::CreateCacheDirectoryIfNeeded();

		// Read the shader file
        const std::string src = ReadFile(filePath);
		// Pre-process the shader source to separate different shader stages
        const std::unordered_map<ShaderType, std::string> sources = PreProcess(src);

        {
            CTimer timer;
			PENUMBRA_CORE_INFO("Shader: Compiling shader at '{0}'", filePath);
			// Compile or retrieve compiled shader binaries
            CompileOrGetD3D11Binaries(sources);
			// Create the shader resources
            CreateResources();
            PENUMBRA_CORE_INFO("Shader: Shader creation took {0} ms", timer.ElapsedMillis());
        }

        // Extract name from the filePath.
        auto slash = filePath.find_last_of("/\\");
        auto dot = filePath.rfind(".");
        if (slash == std::string::npos) slash = 0; else slash++;
        m_Name = filePath.substr(slash, dot - slash);

		// Get the D3D11 device context
        CGraphicsContext* context = CApplication::Get().GetWindow().GetGraphicsContext();

        m_spContext = context->GetContext();
        PENUMBRA_CORE_ASSERT(m_spContext, "Shader: Could not get ID3D11DeviceContext from D3D11Context!");
    }

    CShader::CShader(const std::string& name, const std::string& fileSource)
        : m_Name(name)
    {
        PENUMBRA_PROFILE_FUNC();

		// Pre-process the shader source to separate different shader stages
        const std::unordered_map<ShaderType, std::string> sources = PreProcess(fileSource);

		// Compile or retrieve compiled shader binaries
        CompileOrGetD3D11Binaries(sources);
		// Create the shader resources
        CreateResources();

		// Get the D3D11 device context
        CGraphicsContext* context = CApplication::Get().GetWindow().GetGraphicsContext();

        m_spContext = context->GetContext();
        PENUMBRA_CORE_ASSERT(m_spContext, "Shader: Could not get ID3D11DeviceContext from D3D11Context!");
    }

    ID3DBlob* CShader::GetVertexBlob() const
    {
        PENUMBRA_PROFILE_FUNC();

		// Retrieve the vertex shader blob
        auto it = m_D3D11Blobs.find(ShaderType::SHADER_TYPE_VERTEX);
        if (it != m_D3D11Blobs.end())
			return it->second.Get();    // Return the ID3DBlob pointer

        PENUMBRA_CORE_ERROR("Shader: No Vertex ShaderBlob could be found!");
        return nullptr;
    }

    std::string CShader::ReadFile(const std::string& filepath)
    {
        PENUMBRA_PROFILE_FUNC();

		// Read the entire shader file
        std::ifstream in(filepath, std::ios::binary);
		// Ensure the file is open
        if (!in.is_open()) {
            PENUMBRA_CORE_ERROR("Shader: Could not open file '{0}'", filepath);
			return "";
        }

		// Read file contents into a stringstream
        std::stringstream ss;
        ss << in.rdbuf();

		// Resolve #include directives
        return Utils::ResolveIncludes(ss.str(), std::filesystem::path(filepath).parent_path());
    }

    std::unordered_map<ShaderType, std::string> CShader::PreProcess(const std::string& src)
    {
        PENUMBRA_PROFILE_FUNC();

        std::unordered_map<ShaderType, std::string> out;

		// Find all shader type tokens and extract corresponding code
        const size_t tokenLen = strlen(kShaderTypeToken);
		size_t pos = src.find(kShaderTypeToken);    // Start of first shader type declaration

		// Loop through all shader type declarations
        while (pos != std::string::npos) {
			// Find end of line for the shader type declaration
            size_t eol = src.find_first_of("\r\n", pos);
            PENUMBRA_CORE_ASSERT(eol != std::string::npos);

			// Extract shader type string
            size_t typeStart = pos + tokenLen + 1;
            std::string typeStr = src.substr(typeStart, eol - typeStart);
			// Determine shader stage from type string
            ShaderType stage = Utils::ShaderTypeFromString(typeStr);

			// Find the start of the shader code
            size_t codeStart = src.find_first_not_of("\r\n", eol);
			// Ensure there is shader code following the declaration
            size_t nextToken = src.find(kShaderTypeToken, codeStart);

			// Extract the shader code
            std::string code = (nextToken == std::string::npos)
                ? src.substr(codeStart)
                : src.substr(codeStart, nextToken - codeStart);

			// Add the DirectX define at the top
            code = kShaderDefineDirectX + code;

			// Add global defines
            if (s_GlobalDefines.contains(stage)) {
				// Insert global defines
                std::stringstream defs;
				// Iterate over global defines for this shader stage
                for (auto& [name, value] : s_GlobalDefines[stage])
                    defs << "#define " << name << " " << value << "\n";
                
				// Prepend global defines to shader code
                code = defs.str() + code;
            }

			// Store the processed shader code
            out[stage] = code;
			pos = nextToken;    // Move to the next shader type declaration
        }

        return out;
    }

    void CShader::CompileOrGetD3D11Binaries(const std::unordered_map<ShaderType, std::string>& sources)
    {
        PENUMBRA_PROFILE_FUNC();

        m_D3D11Blobs.clear();
        const std::filesystem::path cacheDir = Utils::GetCacheDirectory();

        for (const auto& [stage, src] : sources) {
			// Hash source code to create unique cache filename
            std::string hash = Utils::HashShaderSource(src);
			// Construct cache file path
            std::filesystem::path cacheFile =
                cacheDir / (hash + Utils::ShaderStageCachedD3D11FileExtension(stage));

			// Get target profile string
            std::string profile = Utils::ShaderStageToString(stage);

            // Try cache
            {
				// Load from cache
                std::ifstream in(cacheFile, std::ios::binary | std::ios::ate);
                if (in.is_open()) {
					// Get size of cached file
                    size_t size = in.tellg();
                    in.seekg(0);

					// Load blob
                    ComPtr<ID3DBlob> blob = nullptr;
					// Create blob of appropriate size
                    if (SUCCEEDED(D3DCreateBlob(size, blob.GetAddressOf()))) {
						PENUMBRA_CORE_INFO("Shader: Loaded cached shader ({})", profile);
						// Read cached bytecode into blob
                        in.read((char*)blob->GetBufferPointer(), size);
						// Store blob
                        m_D3D11Blobs[stage] = blob;
						// Continue to next shader stage
                        continue;
                    }
                }
            }

            // Compile
            PENUMBRA_CORE_TRACE("Shader: Compiling shader {} ({})", m_Name, profile);

			// Compile HLSL to bytecode using D3DCompile
            ComPtr<ID3DBlob> bytecode, errors;

            HRESULT hr = D3DCompile(
				src.c_str(), src.size(),    // Src's data and size
				nullptr, nullptr, nullptr,  // Optional parameters
				"main",                     // Entry point
				profile.c_str(),                // Target profile
				D3DCOMPILE_ENABLE_STRICTNESS,   // Compile options
				0,  // Effect flags
				bytecode.GetAddressOf(),    // Compiled bytecode
				errors.GetAddressOf()       // Compilation errors
            );

			// Check for compilation errors
            if (FAILED(hr)) {
				// Output errors
                if (errors)
                    PENUMBRA_CORE_ERROR("Shader: Shader compile error:\n{}", (char*)errors->GetBufferPointer());
                PENUMBRA_CORE_ASSERT(false);
            }

			// Store compiled bytecode
            m_D3D11Blobs[stage] = bytecode;

            // Write cache
            {
				// Cache the compiled blob to a file
                std::ofstream out(cacheFile, std::ios::binary);
                out.write((char*)bytecode->GetBufferPointer(), bytecode->GetBufferSize());
            }
        }
    }

    void CShader::CreateResources()
    {
        PENUMBRA_PROFILE_FUNC();

        PENUMBRA_CORE_ASSERT(!m_D3D11Blobs.empty(), "Shader: No compiled blobs present!");

        CGraphicsContext* pContext = CApplication::Get().GetWindow().GetGraphicsContext();
		ComPtr<ID3D11Device> spDevice = pContext->GetDevice();

		// Create shader resources for each compiled blob
        for (auto&& [stage, blob] : m_D3D11Blobs) {
            PENUMBRA_CORE_ASSERT(blob, "Shader: Blob unexpectedly null!");

            CRenderer::Submit([this, spDevice, blob, stage]()
                {
					// Create shader based on stage
                    ComPtr<ID3D11DeviceChild> shader;
                    HRESULT hr = S_OK;

                    switch (stage)
                    {
                    case ShaderType::SHADER_TYPE_VERTEX:
                        // Create vertex shader
                    {
                        PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateVertexShader");
                        hr = spDevice->CreateVertexShader(
                            blob->GetBufferPointer(), blob->GetBufferSize(),
                            nullptr,
                            reinterpret_cast<ID3D11VertexShader**>(shader.GetAddressOf()));
                        break;
                    }
                    case ShaderType::SHADER_TYPE_PIXEL:
                        // Create pixel shader
                    {
                        PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreatePixelShader");
                        hr = spDevice->CreatePixelShader(
                            blob->GetBufferPointer(), blob->GetBufferSize(),
                            nullptr,
                            reinterpret_cast<ID3D11PixelShader**>(shader.GetAddressOf()));
                        break;
                    }
                    case ShaderType::SHADER_TYPE_GEOMETRY:
                        // Create geometry shader
                    {
                        PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateGeometryShader");
                        hr = spDevice->CreateGeometryShader(
                            blob->GetBufferPointer(), blob->GetBufferSize(),
                            nullptr,
                            reinterpret_cast<ID3D11GeometryShader**>(shader.GetAddressOf()));
                        break;
                    }
                    case ShaderType::SHADER_TYPE_COMPUTE:
                    {
						PENUMBRA_PROFILE_SCOPE("ID3D11Device::CreateComputeShader");
                        hr = spDevice->CreateComputeShader(
                            blob->GetBufferPointer(), blob->GetBufferSize(),
                            nullptr,
                            reinterpret_cast<ID3D11ComputeShader**>(shader.GetAddressOf()));
						break;
                    }
                    default:
                        // Unsupported shader stage
                        PENUMBRA_CORE_ERROR("Shader: Unsupported shader stage!");
                        break;
                    }

                    // Check for creation errors
                    PENUMBRA_CORE_ASSERT(SUCCEEDED(hr), "Shader: Failed to create shader!");

                    // Store created shader
                    m_Shaders[stage] = shader;
                });

            // Reflect shader to extract metadata
            Reflect(stage, blob);
        }
    }

    void CShader::Reflect(ShaderType stage, const ComPtr<ID3DBlob>& shaderBlob)
    {
        PENUMBRA_PROFILE_FUNC();

		// Create shader reflection interface
        ComPtr<ID3D11ShaderReflection> pReflect;
		// Get shader reflection
        D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
            IID_ID3D11ShaderReflection, (void**)pReflect.GetAddressOf());

		// Get shader description
        D3D11_SHADER_DESC shaderDesc{};
        pReflect->GetDesc(&shaderDesc);

		// Map constant buffer names to their binding slots
        std::unordered_map<std::string, UINT> cbNameToSlot;
        cbNameToSlot.reserve(shaderDesc.BoundResources);

		// Iterate over bound resources
        for (UINT r = 0; r < shaderDesc.BoundResources; r++) {
			// Get resource binding description
            D3D11_SHADER_INPUT_BIND_DESC bind{};
            pReflect->GetResourceBindingDesc(r, &bind);

			// Store constant buffer bindings
            if (bind.Type == D3D_SIT_CBUFFER) {
                cbNameToSlot[bind.Name] = bind.BindPoint;
            }
			// Store texture bindings for pixel shaders
            else if (bind.Type == D3D_SIT_TEXTURE &&
                stage == ShaderType::SHADER_TYPE_PIXEL) {
				// Add texture binding info
                ShaderTexture_t tex;
                tex.m_Name = bind.Name;
                tex.m_nBinding = bind.BindPoint;

				// Add to material buffer textures
                m_MaterialBuffer.m_vecTextures.push_back(tex);
            }
        }

        PENUMBRA_CORE_TRACE("Shader::Reflect - {0} {1}", Utils::ShaderStageToString(stage), m_FilePath);

		// Iterate over constant buffers
        for (UINT i = 0; i < shaderDesc.ConstantBuffers; i++) {
			// Get constant buffer
            ID3D11ShaderReflectionConstantBuffer* pCB = pReflect->GetConstantBufferByIndex(i);

			// Get constant buffer description
            D3D11_SHADER_BUFFER_DESC cb{};
            pCB->GetDesc(&cb);

			// Get constant buffer name
			std::string name = cb.Name ? cb.Name : "Unknown";

			// Get binding slot
            UINT slot = cbNameToSlot[name];

            // Only handle the material CBuffer (pixel shader)
            if (stage == ShaderType::SHADER_TYPE_PIXEL &&
                slot == kShaderMaterialCBufferSlot) {
				// Populate material buffer info
                m_MaterialBuffer.m_bIsValid = true;
                m_MaterialBuffer.m_Name = name;
                m_MaterialBuffer.m_nSize = cb.Size;
                m_MaterialBuffer.m_nBinding = slot;

				// Iterate over variables in the constant buffer
                for (UINT v = 0; v < cb.Variables; v++) {
					// Get variable
                    ID3D11ShaderReflectionVariable* pVar = pCB->GetVariableByIndex(v);

					// Get variable description
                    D3D11_SHADER_VARIABLE_DESC varDesc{};
                    pVar->GetDesc(&varDesc);

					// Get variable type description
                    D3D11_SHADER_TYPE_DESC typeDesc{};
                    pVar->GetType()->GetDesc(&typeDesc);

					// Populate variable info
                    ShaderUVariable_t var = {};
                    var.m_Name = varDesc.Name;
                    var.m_Type = Utils::MapShaderType(typeDesc);
                    var.m_nOffset = varDesc.StartOffset;
                    var.m_nSize = varDesc.Size;

					// Add variable to material buffer
                    m_MaterialBuffer.m_vecVariables.push_back(var);
                }
            }
        }

		// Output shader info for debugging
        OutputShaderInfo(m_MaterialBuffer);
    }

    void CShader::Bind() const
    {
        PENUMBRA_PROFILE_FUNC();

        CRenderer::Submit([this]()
            {
				// Bind shaders to the pipeline
                for (const auto& [stage, shader] : m_Shaders) {
                    switch (stage)
                    {
						// Bind vertex shader
                    case ShaderType::SHADER_TYPE_VERTEX:
                    {
                        PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::VSSetShader");
                        m_spContext->VSSetShader(
                            static_cast<ID3D11VertexShader*>(shader.Get()),
                            nullptr, 0);
                    }
                        break;
						// Bind pixel shader
                    case ShaderType::SHADER_TYPE_PIXEL:
                    {
                        PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::PSSetShader");
                        m_spContext->PSSetShader(
                            static_cast<ID3D11PixelShader*>(shader.Get()),
                            nullptr, 0);
                    }
                        break;
						// Bind geometry shader
                    case ShaderType::SHADER_TYPE_GEOMETRY:
                    {
                        PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::GSSetShader");
                        m_spContext->GSSetShader(
                            static_cast<ID3D11GeometryShader*>(shader.Get()),
                            nullptr, 0);
                    }
                        break;
						// Unknown shader type
                    default:
                        PENUMBRA_CORE_ASSERT(false, "Shader::Bind: Unknown shader type!");
                        break;
                    }
                }
            });
    }

    void CShader::Unbind() const
    {
        PENUMBRA_PROFILE_FUNC();

        CRenderer::Submit([this]()
            {
				// Unbind all shader stages
                PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::VSSetShader, PSSetShader, GSSetShader");
                m_spContext->VSSetShader(nullptr, nullptr, 0);
                m_spContext->PSSetShader(nullptr, nullptr, 0);
                m_spContext->GSSetShader(nullptr, nullptr, 0);
            });
    }

    void CShader::DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const
    {
        CRenderer::Submit([this, groupCountX, groupCountY, groupCountZ]()
            {
                // Bind shaders to the pipeline
                for (const auto& [stage, shader] : m_Shaders) {
                    if (stage == ShaderType::SHADER_TYPE_COMPUTE) {
                        {
                            PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CSSetShader");
                            m_spContext->CSSetShader(
                                static_cast<ID3D11ComputeShader*>(shader.Get()),
                                nullptr, 0);
                        }

                        {
							PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::Dispatch");
                            m_spContext->Dispatch(groupCountX, groupCountY, groupCountZ);
                        }

                        {
                            PENUMBRA_PROFILE_SCOPE("ID3D11DeviceContext::CSSetShader");
                            m_spContext->CSSetShader(nullptr, nullptr, 0);
                        }
                    }
                }
            });
    }

    void CShader::AddGlobalDefine(ShaderType stage, const std::string& name, const std::string& value)
    {
        s_GlobalDefines[stage][name] = value;
    }

    void CShader::ClearGlobalDefines(ShaderType stage)
    {
        s_GlobalDefines[stage].clear();
    }

    void CShader::OutputShaderInfo(ShaderUBuffer_t& matBuffer)
	{
        PENUMBRA_PROFILE_FUNC();

		// Only output if the material buffer is valid
		if (!matBuffer.m_bIsValid)
			return;

		// Output buffer name
		PENUMBRA_CORE_TRACE("Shader::OutputShaderInfo - {0}", matBuffer.m_Name);

		// Output variables info
		for (auto& var : matBuffer.m_vecVariables) {
			PENUMBRA_CORE_TRACE("      - {0}", var.m_Name);
			PENUMBRA_CORE_TRACE("         Offset: {0}, Size: {1}, Type: {2}", var.m_nOffset, var.m_nSize, Utils::DataTypeToString(var.m_Type));
		}
		// Output textures info
		for (auto& tex : matBuffer.m_vecTextures) {
			PENUMBRA_CORE_TRACE("      - {0} (Binding: {1})", tex.m_Name, tex.m_nBinding);
		}
	}

	// Shader Library is widely unused in this framework but provided for completeness

	void CShaderLibrary::Add(const std::string& name, const Ref<CShader>& spShader)
	{
        PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(!Exists(name), "ShaderLibrary: Shader already exists!");
		m_Shaders[name] = spShader;
	}

	void CShaderLibrary::Add(const Ref<CShader>& spShader)
	{
        PENUMBRA_PROFILE_FUNC();

		auto& name = spShader->GetName();
		Add(name, spShader);
	}

	Ref<CShader> CShaderLibrary::Load(const std::string& filePath)
	{
        PENUMBRA_PROFILE_FUNC();

		auto shader = CreateRef<CShader>(filePath);
		Add(shader);

		return shader;
	}

	Ref<CShader> CShaderLibrary::Load(const std::string& name, const std::string& filePath)
	{
        PENUMBRA_PROFILE_FUNC();

		auto shader = CreateRef<CShader>(filePath);
		Add(name, shader);

		return shader;
	}

	Ref<CShader> CShaderLibrary::Get(const std::string& name)
	{
        PENUMBRA_PROFILE_FUNC();

		PENUMBRA_CORE_ASSERT(Exists(name), "ShaderLibrary: Shader not found!");

		return m_Shaders[name];
	}

	bool CShaderLibrary::Exists(const std::string& name) const
	{
        PENUMBRA_PROFILE_FUNC();

		return m_Shaders.find(name) != m_Shaders.end();
	}
}