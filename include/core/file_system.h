#pragma once

#include "core/buffer.h"

namespace penumbra
{
	class CFileSystem
	{
	public:
		static Buffer_t ReadFileBinary(const std::filesystem::path& filepath);
	};
}