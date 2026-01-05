#include "ppch.h"
#include "core/file_system.h"

namespace penumbra
{
	Buffer_t CFileSystem::ReadFileBinary(const std::filesystem::path& filepath)
	{
		PENUMBRA_PROFILE_FUNC();

		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream) {
			// Failed to open the file
			return {};
		}


		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();

		if (size == 0) {
			// File is empty
			return {};
		}

		Buffer_t buffer(size);
		stream.read(buffer.As<char>(), size);
		stream.close();
		return buffer;
	}
}