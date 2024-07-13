#pragma once

#include <variant>
#include <string>

#include <Windows.h>

typedef int (pDownloadProgressCallback)(long long total_value, long long download_value, void* data,
                                        const char* name);

class Helper {
public:
	static bool ExecuteApplication(const wchar_t* file_path, const wchar_t* argument);
	
	static bool AppendFileData(const char* path, const std::string& data);

	static unsigned long GetDirectorySize(const wchar_t* szDir);

	static bool IsEmptyDirectory(const wchar_t* value);

	static bool UpdateClipboard(std::string_view value);

	static std::string ToStringSize(const std::variant<double, long long>& value);

	static std::wstring ToWStringSize(const std::variant<double, long long>& value);

	static bool ParseContentDispositionFileName(std::wstring_view k, std::wstring_view v,
												std::string* ptr_file_name);

	static std::string GetCpuId();

	static void UpdateStatusLabel(const wchar_t* content);

	static bool OpenFolderAndSelectFile(const wchar_t* file_path);

	static void CreateDesktopIcon(const std::wstring& name);

	static std::wstring RegisterQueryValue(const HKEY& hkRKey, std::wstring_view value_name);

	static DWORD RegisterQueryDWordValue(const HKEY& hkRKey, std::wstring_view value_name);

	static time_t RegisterQueryLastWriteTime(const HKEY& key);

	static std::wstring MakeUniqueName(const wchar_t* folder, const wchar_t* file_name);

	static std::wstring MakeUniqueName(const wchar_t* file_path);

	static std::string GetCacheFile(const char* file_name);

	static std::wstring GetCacheFile(const wchar_t* file_name);

	static time_t FileTimeToTimeStamp(const FILETIME& file_time);

	static std::string GetRoamingDir();
private:
	static std::string GetCacheFolder(const char* folder_name = "softManager");

	static std::wstring GetCacheFolder(const wchar_t* folder_name = L"softManager");
};
