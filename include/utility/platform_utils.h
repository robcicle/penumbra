#pragma once

namespace penumbra
{
	class CPlatformUtils
	{
	public:
		static double GetTime();

		static std::filesystem::path OpenFileDialog(const char* filter);
		static std::filesystem::path SaveFileDialog(const char* filter);
	};
}
