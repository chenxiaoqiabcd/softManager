#pragma once

#include <string>

class CommonHelper {
public:
	static bool GetAdapterInfoWithWindows(char* local_ip, int max_count);

	static std::string GetVersion(const std::string& strSrc);

	static std::string GetFileVersion(std::string_view strFile);
};