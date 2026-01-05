#include "ppch.h"
#include "project/project.h"

#include "renderer/renderer.h"

namespace penumbra
{
	std::filesystem::path CProject::GetAssetAbsolutePath(const std::filesystem::path& path)
	{
		return GetAssetDirectory() / path;
	}
}