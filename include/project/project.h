#pragma once

#include "core/base.h"

namespace penumbra
{
	struct ProjectConfig_t
	{
		std::string m_Name = "Untitled";

		std::filesystem::path m_AssetDirectory = "Assets";
		std::filesystem::path m_AssetRegistryPath = "Library/AssetRegistry.alreg";
		std::filesystem::path m_ScriptModulePath = "Library/ScriptAssemblies";
	};

	class CProject
	{
	public:
		const std::filesystem::path& GetProjectDirectory() { return m_ProjectDirectory; }
		std::filesystem::path GetAssetDirectory() { return GetProjectDirectory() / s_spActiveProject->m_Config.m_AssetDirectory; }
		std::filesystem::path GetAssetRegistryPath() { return GetProjectDirectory() / s_spActiveProject->m_Config.m_AssetRegistryPath; }
		// TO-DO: Move to asset manager when we have one
		std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path) { return GetAssetDirectory() / path; }

		std::filesystem::path GetAssetAbsolutePath(const std::filesystem::path& path);

		static const std::filesystem::path& GetActiveProjectDirectory()
		{
			PENUMBRA_CORE_ASSERT(s_spActiveProject);
			return s_spActiveProject->GetProjectDirectory();
		}

		static std::filesystem::path GetActiveAssetDirectory()
		{
			PENUMBRA_CORE_ASSERT(s_spActiveProject);
			return s_spActiveProject->GetAssetDirectory();
		}

		static std::filesystem::path GetActiveAssetRegistryPath()
		{
			PENUMBRA_CORE_ASSERT(s_spActiveProject);
			return s_spActiveProject->GetAssetRegistryPath();
		}

		static std::filesystem::path GetScriptModulePath()
		{
			PENUMBRA_CORE_ASSERT(s_spActiveProject);
			return s_spActiveProject->GetProjectDirectory() / s_spActiveProject->m_Config.m_ScriptModulePath;
		}

		// TO-DO: Move to asset manager when we have one
		static std::filesystem::path GetActiveAssetFileSystemPath(const std::filesystem::path& path)
		{
			PENUMBRA_CORE_ASSERT(s_spActiveProject);
			return s_spActiveProject->GetAssetFileSystemPath(path);
		}

		ProjectConfig_t& GetConfig() { return m_Config; }

		static Ref<CProject> GetActive() { return s_spActiveProject; }
	private:
		ProjectConfig_t m_Config;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<CProject> s_spActiveProject;
	};
}