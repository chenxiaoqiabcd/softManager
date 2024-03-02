
// FlieNmae: Softinfo.cpp

#include "softInfo.h"

#include <algorithm>
#include <filesystem>
#include <Msi.h>

#include "cache_data_helper.h"
#include "event_queue_global_manager.h"
#include "file_helper.h"
#include "helper.h"
#include "versionHelper.h"

CSoftInfo::CSoftInfo() {
	
}

std::vector<SoftInfo> CSoftInfo::GetSoftInfo(void)
{
	if(m_SoftInfoArr.empty() && mtx_.try_lock()) {
		Init(HKEY_LOCAL_MACHINE, KEY_READ | KEY_WOW64_64KEY);
		Init(HKEY_LOCAL_MACHINE, KEY_READ | KEY_WOW64_32KEY);

		Init(HKEY_CURRENT_USER, KEY_READ | KEY_WOW64_64KEY);
		Init(HKEY_CURRENT_USER, KEY_READ | KEY_WOW64_32KEY);

		mtx_.unlock();
	}

	return m_SoftInfoArr;
}

void CSoftInfo::UpdateSoftInfo() {
	if (mtx_.try_lock()) {
		m_SoftInfoArr.clear();
		m_SystemPatchesArr.clear();

		Init(HKEY_LOCAL_MACHINE, KEY_READ | KEY_WOW64_64KEY);
		Init(HKEY_LOCAL_MACHINE, KEY_READ | KEY_WOW64_32KEY);

		Init(HKEY_CURRENT_USER, KEY_READ | KEY_WOW64_64KEY);
		Init(HKEY_CURRENT_USER, KEY_READ | KEY_WOW64_32KEY);

		mtx_.unlock();
	}
}

void CSoftInfo::UpdateSoftInfo(const wchar_t* key_name) {
	if (mtx_.try_lock()) {
		auto find_soft_info = [key_name](const SoftInfo& info) {
			return info.key_name == key_name;
		};

		const auto it_find = std::find_if(m_SoftInfoArr.begin(), m_SoftInfoArr.end(), find_soft_info);
		if (it_find != m_SoftInfoArr.end()) {
			m_SoftInfoArr.erase(it_find);
		}

		Init(HKEY_LOCAL_MACHINE, KEY_READ | KEY_WOW64_64KEY, key_name);
		Init(HKEY_LOCAL_MACHINE, KEY_READ | KEY_WOW64_32KEY, key_name);

		Init(HKEY_CURRENT_USER, KEY_READ | KEY_WOW64_64KEY, key_name);
		Init(HKEY_CURRENT_USER, KEY_READ | KEY_WOW64_32KEY, key_name);

		mtx_.unlock();
	}
}

void CSoftInfo::GetSoftName(std::vector<LPCTSTR>& lpszSoftName)
{
	std::vector<SoftInfo>::iterator iter;
	for (iter = m_SoftInfoArr.begin(); iter != m_SoftInfoArr.end(); iter++)
	{
		lpszSoftName.push_back(iter->m_strSoftName);
	}
}

void CSoftInfo::GetSoftVersion(std::vector<LPCTSTR>& lpszSoftVersion)
{
	std::vector<SoftInfo>::iterator iter;
	for (iter = m_SoftInfoArr.begin(); iter != m_SoftInfoArr.end(); iter++)
	{
		if (!(iter->m_strSoftVersion).IsEmpty())
		{
			lpszSoftVersion.push_back(iter->m_strSoftVersion);
		}
	}
}

void CSoftInfo::GetInstallLocation(std::vector<LPCTSTR>& lpszInstallLocation)
{
	std::vector<SoftInfo>::iterator iter;
	for (iter = m_SoftInfoArr.begin(); iter != m_SoftInfoArr.end(); iter++)
	{
		if (!(iter->m_strInstallLocation).IsEmpty())
		{
			lpszInstallLocation.push_back(iter->m_strInstallLocation);
		}
	}
}

void CSoftInfo::GetPublisher(std::vector<LPCTSTR>& lpszPublisher)
{
	for (auto & iter : m_SoftInfoArr) {
		auto FindPublisher = [&iter](const LPCTSTR& value) {
			return iter.m_strPublisher == value;
		};

		if(!std::any_of(lpszPublisher.begin(), lpszPublisher.end(), FindPublisher)) {
			lpszPublisher.emplace_back(iter.m_strPublisher);
		}
	}
}

void CSoftInfo::GetMainProPath(std::vector<LPCTSTR>& lpszMainProPath)
{
	std::vector<SoftInfo>::iterator iter;
	for (iter = m_SoftInfoArr.begin(); iter != m_SoftInfoArr.end(); iter++)
	{
		if (!iter->m_strMainProPath.IsEmpty())
		{
			lpszMainProPath.push_back(iter->m_strMainProPath);
		}
	}
}

void CSoftInfo::GetUninstallPth(std::vector<LPCTSTR>& lpszSoftName)
{
	std::vector<SoftInfo>::iterator iter;
	for (iter = m_SoftInfoArr.begin(); iter != m_SoftInfoArr.end(); iter++)
	{
		if (!iter->m_strUninstallPth.IsEmpty())
		{
			lpszSoftName.push_back(iter->m_strUninstallPth);
		}
	}
}

std::vector<SoftInfo> CSoftInfo::GetSystemPatchesInfo(void) const
{
	return m_SystemPatchesArr;
}

void CSoftInfo::GetSystemPatchesName(std::vector<LPCTSTR>& lpszSoftName)
{
	std::vector<SoftInfo>::iterator iter;
	for (iter = m_SystemPatchesArr.begin(); iter != m_SystemPatchesArr.end(); iter++)
	{
		lpszSoftName.push_back(iter->m_strSoftName);
	}
}

CString CSoftInfo::GetIcon(HKEY key) {
	DWORD dw_type = REG_SZ;

	DWORD dw_len = 1024;
	TCHAR szBuffer[1024] = { 0 };

	RegQueryValueEx(key, _T("DisplayIcon"), nullptr, &dw_type, reinterpret_cast<LPBYTE>(szBuffer), &dw_len);

	std::wstring icon_path = szBuffer;

	const auto index = icon_path.find(',');

	if (std::string::npos != index) {
		auto dll_path = icon_path.substr(0, index);
		auto icon_index = std::stoi(icon_path.substr(index + 1));

		return FileHelper::GetIconWithDllPath(dll_path.c_str(), icon_index).c_str();
	}

	return FileHelper::GetFilePath(icon_path.c_str()).c_str();
}

bool IsValidInstallPath(const CString& install_path, const CString& uninstall_path) {
	const auto install_path_temp = FileHelper::GetFilePath(install_path.GetString());
	const auto uninstall_path_temp = FileHelper::GetFilePath(uninstall_path.GetString());

	if(!install_path_temp.empty()) {
		return !Helper::IsEmptyDirectory(install_path_temp.c_str());
	}

	if(!uninstall_path_temp.empty()) {
		wchar_t szUnInstallPath[MAX_PATH];
		ZeroMemory(szUnInstallPath, MAX_PATH * sizeof(wchar_t));
		wcscpy(szUnInstallPath, uninstall_path_temp.c_str());
		PathRemoveFileSpec(szUnInstallPath);

		if (PathIsDirectory(szUnInstallPath)) {
			return !Helper::IsEmptyDirectory(szUnInstallPath);
		}
	}

	return false;
}

SoftInfo CSoftInfo::GenerateSoftInfo(HKEY key, const wchar_t* key_name, DWORD ulOptions) {
	SoftInfo info{};

	info.time_stamp = Helper::RegisterQueryLastWriteTime(key);
	info.m_strSoftName = Helper::RegisterQueryValue(key, L"DisplayName").c_str();
	info.m_strInstallLocation = Helper::RegisterQueryValue(key, L"InstallLocation").c_str();
	info.m_strUninstallPth = Helper::RegisterQueryValue(key, L"UninstallString").c_str();
	info.m_strSoftVersion = Helper::RegisterQueryValue(key, L"DisplayVersion").c_str();
	info.key_name = key_name;
	info.m_strPublisher = Helper::RegisterQueryValue(key, L"Publisher").c_str();
	info.m_strMainProPath = Helper::RegisterQueryValue(key, L"InstallLocation").c_str();
	info.m_strSoftIcon = GetIcon(key);
	info.bit = (ulOptions & KEY_WOW64_64KEY) ? 64 : 32;

	return info;
}

bool CSoftInfo::CheckSoftInfo(SoftInfo* info) {
	if (-1 != info->key_name.Find(L"KB") && -1 != info->key_name.Find(L"{")) {
		m_SystemPatchesArr.push_back(*info);
		return false;
	}

	if (info->key_name[0] == 'K' && info->key_name[1] == 'B') {
		m_SystemPatchesArr.push_back(*info);
		return false;
	}

	if (StrStrIW(info->m_strInstallLocation, L"ProgramData") ||
		StrStrIW(info->m_strUninstallPth, L"ProgramData")) {
		return false;
	}

	if (info->m_strSoftName.IsEmpty()) {
		// KF_WARN(L"版本号为空的软件 %s %s", info->key_name, info->m_strSoftName);
		return false;
	}

	if (!IsValidInstallPath(info->m_strInstallLocation, info->m_strUninstallPth)) {
		// KF_WARN(L"找不到安装目录的软件 %s %s %s",
		// 		info->m_strSoftName, info->m_strInstallLocation, info->m_strUninstallPth);
		return false;
	}

	if (info->m_strSoftVersion.IsEmpty()) {
		info->m_strSoftVersion =
			FileHelper::GetFileVersion(info->m_strSoftIcon).c_str();
	}

	if (info->m_strSoftVersion.IsEmpty() || info->m_strSoftVersion == L"1") {
		// KF_WARN(L"版本号为空的软件 %s %s", soft_info->key_name, soft_info->m_strSoftName);
		return false;
	}

	auto FindNewerSoftInfo = [info](const SoftInfo& soft_info) {
		const VersionHelper soft_info_version(soft_info.m_strSoftVersion);
		VersionHelper info_version(info->m_strSoftVersion);

		return info->key_name == soft_info.key_name && info_version > soft_info_version;
	};

	const auto it_find = std::find_if(m_SoftInfoArr.begin(), m_SoftInfoArr.end(), FindNewerSoftInfo);
	if (it_find != m_SoftInfoArr.end()) {
		// KF_WARN(L"找到了更大版本的注册表位置 %s", it_find->m_strSoftName.GetString());
		m_SoftInfoArr.erase(it_find);
	}

	auto FindSoftInfo = [info](const SoftInfo& s) {
		return s.key_name == info->key_name;
	};

	// 过滤重复的软件
	if (std::any_of(m_SoftInfoArr.begin(), m_SoftInfoArr.end(), FindSoftInfo)) {
		// KF_WARN(L"已经在注册表找到的软件 %s", info.m_strSoftName);
		return false;
	}

	return true;
}

SoftInfo CSoftInfo::GetSoftInfo(HKEY hkRKey, std::wstring_view szKeyName, DWORD ulOptions) {
	const auto time_stamp = Helper::RegisterQueryLastWriteTime(hkRKey);

	SoftInfo soft_info;
	if (SoftInfoCacheData::GetValue(szKeyName.data(), hkRKey, ulOptions, &soft_info)) {
		if (time_stamp == soft_info.time_stamp) {
			return soft_info;
		}
	}

	return GenerateSoftInfo(hkRKey, szKeyName.data(), ulOptions);
}

void CSoftInfo::AddSoftInfo(HKEY root_key, std::wstring_view lpSubKey, 
											   std::wstring_view szKeyName, DWORD ulOptions) {
	if(szKeyName.empty()) {
		return;
	}

	HKEY hkRKey;

	CString sub_path;
	sub_path.Format(L"%s\\%s", lpSubKey.data(), szKeyName.data());

	if (RegOpenKeyEx(root_key, sub_path, 0, ulOptions, &hkRKey) != ERROR_SUCCESS) {
		return;
	}

	auto soft_info = GetSoftInfo(hkRKey, szKeyName, ulOptions);

	if(CheckSoftInfo(&soft_info)) {
		m_SoftInfoArr.push_back(soft_info);
	}
}

void CSoftInfo::Init(HKEY root_key, DWORD ulOptions) {
	LPCTSTR lpSubKey;        // 子键名称
	HKEY hkResult;            // 将要打开键的句柄 
	LONG lReturn;            // 记录读取注册表是否成功

	lpSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

	lReturn = RegOpenKeyEx(root_key, lpSubKey, 0, ulOptions, &hkResult);

	if (lReturn == ERROR_SUCCESS) {
		DWORD dwKeyLen = 255;
		TCHAR szKeyName[255];        // 注册表项名称
		DWORD index = 0;

		ZeroMemory(szKeyName, sizeof(TCHAR) * 255);

		while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(hkResult, index++, szKeyName, &dwKeyLen, nullptr, nullptr,
												   nullptr , nullptr)) {
			AddSoftInfo(root_key, lpSubKey, szKeyName, ulOptions);
			dwKeyLen = 255;
			ZeroMemory(szKeyName, sizeof(TCHAR) * 255);
		}

		RegCloseKey(hkResult);

		return;
	}

	::MessageBox(NULL, _T("打开注册表失败!"), NULL, MB_ICONWARNING);
}

void CSoftInfo::Init(HKEY root_key, DWORD ulOptions, const wchar_t* key_name) {
	LPCTSTR lpSubKey;        // 子键名称
	HKEY hkResult;            // 将要打开键的句柄
	LONG lReturn;            // 记录读取注册表是否成功

	lpSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

	lReturn = RegOpenKeyEx(root_key, lpSubKey, 0, ulOptions, &hkResult);

	if (lReturn == ERROR_SUCCESS) {
		DWORD dwKeyLen = 255;
		TCHAR szKeyName[255];        // 注册表项名称
		DWORD index = 0;

		ZeroMemory(szKeyName, sizeof(TCHAR) * 255);

		while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(hkResult, index++, szKeyName, &dwKeyLen,
												   nullptr, nullptr, nullptr, nullptr)) {
			if (0 == wcscmp(szKeyName, key_name)) {
				AddSoftInfo(root_key, lpSubKey, szKeyName, ulOptions);
				break;
			}

			dwKeyLen = 255;
			ZeroMemory(szKeyName, sizeof(TCHAR) * 255);
		}

		RegCloseKey(hkResult);
		return;
	}

	::MessageBox(NULL, _T("打开注册表失败!"), NULL, MB_ICONWARNING);
}
