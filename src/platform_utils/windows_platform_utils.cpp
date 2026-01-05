#include "ppch.h"
#include "utility/platform_utils.h"

#include "core/application.h"

namespace penumbra
{
    namespace Utils
    {
        static std::string ToProjectRelative(const std::string& absolutePath)
        {
            if (absolutePath.empty())
                return "";

			// Convert to relative path based on project working directory
            std::filesystem::path abs = absolutePath;
            std::filesystem::path projectDir = CApplication::Get().GetSpecification().m_WorkingDirectory;

            std::error_code ec;
			// Get the relative path
            std::filesystem::path rel = std::filesystem::relative(abs, projectDir, ec);

			// If no error and relative path is not empty, return it
            if (!ec && !rel.generic_string().empty())
				return rel.generic_string();    // Use generic_string to ensure forward slashes

			// Otherwise, return the original absolute path
            return absolutePath;
        }
    }

	float CPlatformUtils::GetTime()
	{
		PENUMBRA_PROFILE_FUNC();

		// Initialize frequency once
		static LARGE_INTEGER s_Frequency;
		static BOOL s_Initialized = QueryPerformanceFrequency(&s_Frequency);

		// Get current time
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);
		// Return time in seconds
		return static_cast<float>(currentTime.QuadPart / static_cast<double>(s_Frequency.QuadPart));
	}

    std::filesystem::path CPlatformUtils::OpenFileDialog(const char* filter)
    {
		// Setup OPENFILENAME structure
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
		// Initialize memory
        ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		// Configure structure
        ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = GetActiveWindow();  // Owner window
		ofn.lpstrFile = szFile;             // File buffer
		ofn.nMaxFile = sizeof(szFile);      // Max file size
		ofn.lpstrFilter = filter;           // File type filter
		ofn.nFilterIndex = 1;               // Default filter index
		// Set flags
        ofn.Flags = 
			OFN_PATHMUSTEXIST |     // Path must exist
			OFN_FILEMUSTEXIST |     // File must exist
			OFN_NOCHANGEDIR;        // Don't change the current directory

		// Open the file dialog
        if (GetOpenFileNameA(&ofn) == TRUE)
			// Return the selected file path as project-relative
            return std::filesystem::path(Utils::ToProjectRelative(ofn.lpstrFile));

		// If cancelled or error, return empty path
        return "";
    }

    std::filesystem::path CPlatformUtils::SaveFileDialog(const char* filter)
    {
		// Setup OPENFILENAME structure
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
		// Initialize memory
        ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		// Configure structure
        ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = GetActiveWindow();  // Owner window
		ofn.lpstrFile = szFile;             // File buffer
		ofn.nMaxFile = sizeof(szFile);      // Max file size
		ofn.lpstrFilter = filter;			// File type filter
		ofn.nFilterIndex = 1;				// Default filter index
        ofn.Flags = 
			OFN_PATHMUSTEXIST |	// Path must exist 
			OFN_NOCHANGEDIR;	// Don't change the current directory

		// Open the save file dialog
        if (GetSaveFileNameA(&ofn) == TRUE)
			// Return the selected file path as project-relative
            return std::filesystem::path(Utils::ToProjectRelative(ofn.lpstrFile));

		// If cancelled or error, return empty path
        return "";
    }
}
