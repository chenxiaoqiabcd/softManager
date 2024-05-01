#include "helper.h"

#include <intrin.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <atlcomcli.h>
#include <filesystem>

#include "event_queue_global_manager.h"
#include "stringHelper.h"

#include "kf_str.h"

bool Helper::ExecuteApplication(const wchar_t* file_path, const wchar_t* argument) {
	SHELLEXECUTEINFO sei;
	sei.cbSize = sizeof SHELLEXECUTEINFO;
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = nullptr;
	sei.lpVerb = L"open";
	sei.lpFile = file_path;
	sei.lpParameters = argument;
	sei.lpDirectory = L"";
	sei.nShow = SW_SHOW;
	sei.hInstApp = nullptr;
	const bool bSuccess = ShellExecuteEx(&sei);

	WaitForSingleObject(sei.hProcess, INFINITE);

	return bSuccess;
}

bool Helper::AppendFileData(const char* path, const std::string& data) {
	FILE* f;
	fopen_s(&f, path, "ab");
	if (f == nullptr) {
		return false;
	}
	fwrite(data.c_str(), data.size(), 1, f);
	fclose(f);
	return true;
}

unsigned long Helper::GetDirectorySize(const wchar_t* szDir) {
	wchar_t szFindDirectory[MAX_PATH] = { 0 };
	wcscpy(szFindDirectory, szDir);
	PathAppend(szFindDirectory, L"*.*");

	DWORD llSize = 0;

	WIN32_FIND_DATA wfd;

	const HANDLE hFind = FindFirstFile(szFindDirectory, &wfd);

	if(hFind == INVALID_HANDLE_VALUE) {
		return llSize;
	}

	do {
		if (0 == wcscmp(wfd.cFileName, L".") || 0 == wcscmp(wfd.cFileName, L"..")) {
			continue;
		}

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			wchar_t szNewPath[MAX_PATH] = { 0 };
			wcsncpy(szNewPath, szDir, MAX_PATH);
			PathAppend(szNewPath, wfd.cFileName);

			llSize += GetDirectorySize(szNewPath);
			continue;
		}

		llSize += wfd.nFileSizeLow;
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);

	return llSize;
}

bool Helper::IsEmptyDirectory(const wchar_t* value) {
	wchar_t find_folder[MAX_PATH];
	ZeroMemory(find_folder, sizeof(find_folder));
	wcscpy(find_folder, value);
	PathAppend(find_folder, L"*.*");

	WIN32_FIND_DATA wfd;
	const auto hFind = FindFirstFile(find_folder, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return true;
	}

	bool empty_directory = true;

	do {
		if (0 == wcscmp(wfd.cFileName, L".") || 0 == wcscmp(wfd.cFileName, L"..")) {
			continue;
		}

		empty_directory = false;
		break;
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);
	return empty_directory;
}

bool Helper::UpdateClipboard(std::string_view value) {
	if (!OpenClipboard(nullptr)) {
		return true;
	}

	EmptyClipboard();

	const auto clipBuffer = GlobalAlloc(GMEM_DDESHARE, value.size() + 1);
	if (nullptr != clipBuffer) {
		auto buffer = static_cast<char*>(GlobalLock(clipBuffer));
		strcpy(buffer, value.data());
		GlobalUnlock(clipBuffer);
	}
	
	SetClipboardData(CF_TEXT, clipBuffer);

	CloseClipboard();

	return true;
}

constexpr long g_k_bit = 1024;
constexpr long g_m_bit = 1024 * 1024;
constexpr long g_g_bit = 1024 * 1024 * 1024;

constexpr double g_single_k_bit = 1.0 / g_k_bit;
constexpr double g_single_m_bit = 1.0 / g_m_bit;
constexpr double g_single_g_bit = 1.0 / g_g_bit;

std::string Helper::ToStringSize(const std::variant<double, long long>& value) {
	auto result = std::visit([](auto arg) -> std::string {
		char text[MAX_PATH];
		ZeroMemory(text, MAX_PATH);

		if (arg < g_k_bit) {
			sprintf_s(text, "%.0lf B", arg * 1.0f);
		}
		else if (arg < g_m_bit) {
			sprintf_s(text, "%.2f KB", arg * g_single_k_bit);
		}
		else if (arg < g_g_bit) {
			sprintf_s(text, "%.2f MB", arg * g_single_m_bit);
		}
		else {
			sprintf_s(text, "%.2f GB", arg * g_single_g_bit);
		}

		return text;
	}, value);

	return result;
}

std::wstring Helper::ToWStringSize(const std::variant<double, long long>& value) {
	return CStringHelper::a2w(ToStringSize(value));
}

bool Helper::ParseContentDispositionFileName(std::wstring_view k, std::wstring_view v,
											 std::string* ptr_file_name) {
	if (nullptr == StrStrIW(k.data(), L"Content-Disposition")) {
		return false;
	}

	const auto data = KfString(v.data()).Split(";");
	if(data.size() == 2) {
		auto attribute_key_value = data[1].Split("=");
		if (attribute_key_value.size() == 2 && attribute_key_value[0].Trim() == "filename") {
			const auto u_file_name = CStringHelper::DeescapeURL(attribute_key_value[1].GetString());
			*ptr_file_name = KfString(u_file_name.c_str());
			return true;
		}
	}	

	return false;
}

std::string Helper::GetCpuId() {
	int cpuInfo[4] = { -1 };
	__cpuid(cpuInfo, 1);

	char cpu_id_buffer[32] = { 0 };
	sprintf(cpu_id_buffer, "%08X%08X", cpuInfo[3], cpuInfo[0]);
	return cpu_id_buffer;
}

void Helper::UpdateStatusLabel(const wchar_t* content) {
	assert(nullptr != content);

	auto msg(new wchar_t[1024]);
	ZeroMemory(msg, 1024 * sizeof(wchar_t));
	wsprintf(msg, content);

	EventQueueInstance->SendEvent(EVENT_UPDATE_STATUS_LABEL, reinterpret_cast<WPARAM>(msg), 0);
}

bool Helper::OpenFolderAndSelectFile(const wchar_t* file_path) {
	if (nullptr == file_path || 0 == _wcsicmp(file_path, TEXT(""))) {
		return false;
	}

	LPITEMIDLIST pidl;
	LPCITEMIDLIST cpidl;
	LPSHELLFOLDER pDesktopFolder;
	ULONG chEaten;
	HRESULT hr;
	WCHAR wfilePath[MAX_PATH + 1] = { 0 };

	::CoInitialize(NULL);

	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{
		// IShellFolder::ParseDisplayName要传入宽字节
		LPWSTR lpWStr = NULL;
#ifdef _UNICODE
		_tcscpy(wfilePath, file_path);
		lpWStr = wfilePath;
#else
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)strFilePath, -1, wfilePath, MAX_PATH);
		lpWStr = wfilePath;
#endif

		hr = pDesktopFolder->ParseDisplayName(NULL, 0, lpWStr, &chEaten, &pidl, NULL);
		if (FAILED(hr))
		{
			pDesktopFolder->Release();
			::CoUninitialize();
			return FALSE;
		}

		cpidl = pidl;

		// SHOpenFolderAndSelectItems是非公开的API函数，需要从shell32.dll获取
		// 该函数只有XP及以上的系统才支持，Win2000和98是不支持的，考虑到Win2000
		// 和98已经基本不用了，所以就不考虑了，如果后面要支持上述老的系统，则要
				// 添加额外的处理代码
		HMODULE hShell32DLL = ::LoadLibrary(_T("shell32.dll"));
		assert(hShell32DLL != NULL);
		if (hShell32DLL != NULL)
		{
			typedef HRESULT(WINAPI* pSelFun)(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST* apidl, DWORD dwFlags);
			pSelFun pFun = (pSelFun)::GetProcAddress(hShell32DLL, "SHOpenFolderAndSelectItems");
			assert(pFun != NULL);
			if (pFun != NULL)
			{
				hr = pFun(cpidl, 0, NULL, 0); // 第二个参数cidl置为0，表示是选中文件
				if (FAILED(hr))
				{
					::FreeLibrary(hShell32DLL);
					pDesktopFolder->Release();
					::CoUninitialize();
					return FALSE;
				}
			}

			::FreeLibrary(hShell32DLL);
		}
		else
		{
			pDesktopFolder->Release();
			::CoUninitialize();
			return FALSE;
		}

		// 释放pDesktopFolder
		pDesktopFolder->Release();
	}
	else
	{
		::CoUninitialize();
		return FALSE;
	}

	::CoUninitialize();
	return TRUE;
}

void Helper::CreateDesktopIcon(const std::wstring& name) {
	wchar_t shortcut_path[MAX_PATH];
	memset(shortcut_path, 0, sizeof(MAX_PATH));
	SHGetSpecialFolderPath(NULL, shortcut_path, CSIDL_DESKTOPDIRECTORY, FALSE);

	std::wstring shortcut_name = name + L".lnk";
	std::wstring shortcut_path_name = std::wstring(shortcut_path) + L"\\" + shortcut_name;

	CComPtr<IShellLink> psl;
	CComPtr<IPersistFile> ppf;

	CoInitialize(NULL);

	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl))) {
		wchar_t file_name[MAX_PATH];
		ZeroMemory(file_name, sizeof(file_name));
		GetModuleFileName(nullptr, file_name, MAX_PATH);

		psl->SetPath(file_name);
		psl->SetArguments(L"");
		psl->SetDescription(name.c_str());
		psl->SetIconLocation(file_name, 0);

		if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf))) {
			ppf->Save(shortcut_path_name.c_str(), TRUE);
		}

		ppf.Release();
	}

	CoUninitialize();
}

std::wstring Helper::RegisterQueryValue(const HKEY& hkRKey, std::wstring_view value_name) {
	DWORD dwNameLen = 255;
	DWORD dwType = REG_BINARY | REG_DWORD | REG_EXPAND_SZ | REG_MULTI_SZ | REG_NONE | REG_SZ;

	TCHAR szBuffer[255];
	ZeroMemory(szBuffer, sizeof(TCHAR) * 255);

	RegQueryValueEx(hkRKey, value_name.data(), nullptr, &dwType, reinterpret_cast<LPBYTE>(szBuffer), &dwNameLen);

	return szBuffer;
}

DWORD Helper::RegisterQueryDWordValue(const HKEY& hkRKey, std::wstring_view value_name) {
	DWORD dwNameLen = 255;
	DWORD dwType = REG_DWORD;

	DWORD dwBuffer = 0;
	RegQueryValueEx(hkRKey, value_name.data(), nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwBuffer),
					&dwNameLen);

	return dwBuffer;
}

time_t Helper::RegisterQueryLastWriteTime(const HKEY& key) {
	FILETIME file_time;
	const LSTATUS st = RegQueryInfoKey(key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
									   nullptr, nullptr, nullptr, &file_time);
	if (st != ERROR_SUCCESS) {
		return 0;
	}

	return FileTimeToTimeStamp(file_time);
}

std::wstring Helper::MakeUniqueName(const wchar_t* folder, const wchar_t* file_name) {
	wchar_t new_path[MAX_PATH];
	ZeroMemory(new_path, sizeof(new_path));
	wcscpy_s(new_path, folder);
	PathAppend(new_path, file_name);

	if (!PathFileExists(new_path)) {
		return new_path;
	}

	wchar_t unique_name[MAX_PATH];
	ZeroMemory(unique_name, sizeof(unique_name));
	PathMakeUniqueName(unique_name, MAX_PATH, file_name, file_name, folder);

	return unique_name;
}

std::wstring Helper::MakeUniqueName(const wchar_t* file_path) {
	if (!PathFileExists(file_path)) {
		return file_path;
	}

	const auto file_name = PathFindFileName(file_path);

	wchar_t folder[MAX_PATH];
	ZeroMemory(folder, sizeof(folder));
	wcscpy_s(folder, file_path);
	PathRemoveFileSpec(folder);

	return MakeUniqueName(folder, file_name);
}

std::string Helper::GetCacheFile(const char* file_name) {
	const auto cache_folder = GetCacheFolder("softManager");
	char cache_file[MAX_PATH];
	ZeroMemory(cache_file, sizeof(cache_file));
	PathAppendA(cache_file, cache_folder.c_str());
	PathAppendA(cache_file, file_name);

	return cache_file;
}

std::wstring Helper::GetCacheFile(const wchar_t* file_name) {
	const auto cache_folder = GetCacheFolder(L"softManager");
	wchar_t cache_file[MAX_PATH];
	ZeroMemory(cache_file, sizeof(cache_file));
	PathAppend(cache_file, cache_folder.c_str());
	PathAppend(cache_file, file_name);

	return cache_file;
}

time_t Helper::FileTimeToTimeStamp(const FILETIME& file_time) {
	SYSTEMTIME sys_time;
	FileTimeToSystemTime(&file_time, &sys_time);

	struct tm time;
	time.tm_year = sys_time.wYear - 1900;
	time.tm_mon = sys_time.wMonth - 1;
	time.tm_mday = sys_time.wDay;
	time.tm_hour = sys_time.wHour;
	time.tm_min = sys_time.wMinute;
	time.tm_sec = sys_time.wSecond;
	time.tm_isdst = -1;

	return mktime(&time);
}

// private
std::string Helper::GetCacheFolder(const char* folder_name) {
	char cache_folder[MAX_PATH];
	ZeroMemory(cache_folder, sizeof(cache_folder));
	SHGetSpecialFolderPathA(nullptr, cache_folder, CSIDL_APPDATA, FALSE);

	PathAppendA(cache_folder, folder_name);

	if (!PathFileExistsA(cache_folder)) {
		CreateDirectoryA(cache_folder, nullptr);
	}

	return cache_folder;
}

std::wstring Helper::GetCacheFolder(const wchar_t* folder_name) {
	wchar_t cache_folder[MAX_PATH];
	ZeroMemory(cache_folder, sizeof(cache_folder));
	SHGetSpecialFolderPath(nullptr, cache_folder, CSIDL_APPDATA, FALSE);

	PathAppend(cache_folder, folder_name);

	if (!PathFileExists(cache_folder)) {
		CreateDirectory(cache_folder, nullptr);
	}

	return cache_folder;
}