
// FlieNmae: Softinfo.cpp

#include "softInfo.h"

#include <algorithm>
#include <filesystem>
#include <Msi.h>

#include "event_queue_global_manager.h"
#include "file_helper.h"
#include "helper.h"
#include "kf_log.h"
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
		icon_path = icon_path.substr(0, index);

		if (icon_path[0] == '\"') {
			icon_path.append(TEXT("\""));
		}
	}

	return FileHelper::GetFilePath(icon_path.c_str()).c_str();
}

CString CSoftInfo::GetIcon(CString soft_name, CString install_path) {
	if(soft_name == L"Inkscape") {
		wchar_t icon_path[1024];
		ZeroMemory(icon_path, sizeof(wchar_t) * 1024);
		wcscpy_s(icon_path, install_path);
		PathAppend(icon_path, L"bin\\inkscape.exe");
	
		return icon_path;
	}
	
	if (StrStrIW(soft_name, L"notepad--")) {
		wchar_t icon_path[1024];
		ZeroMemory(icon_path, sizeof(wchar_t) * 1024);
		wcscpy_s(icon_path, install_path);
		PathAppend(icon_path, L"notepad--.exe");

		return icon_path;
	}

	return L"";
}

DWORD GetSize(const CString& install_path, const CString& uninstall_path) {
	const auto install_path_temp = FileHelper::GetFilePath(install_path.GetString());
	const auto uninstall_path_temp = FileHelper::GetFilePath(uninstall_path.GetString());

	if(!install_path_temp.empty()) {
		return Helper::GetDirectorySize(install_path_temp.c_str());
	}

	if(!uninstall_path_temp.empty()) {
		wchar_t szUnInstallPath[MAX_PATH];
		ZeroMemory(szUnInstallPath, MAX_PATH * sizeof(wchar_t));
		wcscpy(szUnInstallPath, uninstall_path_temp.c_str());
		PathRemoveFileSpec(szUnInstallPath);

		if (PathIsDirectory(szUnInstallPath)) {
			return Helper::GetDirectorySize(szUnInstallPath);
		}
	}

	return 0;
}

bool CSoftInfo::CheckData(HKEY key, const wchar_t* szKeyName, SoftInfo* info) {
	info->m_strSoftName = Helper::RegisterQueryValue(key, L"DisplayName").c_str();
	info->m_strInstallLocation = Helper::RegisterQueryValue(key, L"InstallLocation").c_str();
	info->m_strUninstallPth = Helper::RegisterQueryValue(key, L"UninstallString").c_str();
	info->m_strSoftVersion = Helper::RegisterQueryValue(key, L"DisplayVersion").c_str();
	info->key_name = szKeyName;

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
		KF_WARN(L"programdataĿ¼�µ����� %s", info->m_strSoftName.GetString());
		return false;
	}

	auto FindNewerSoftInfo = [info](const SoftInfo& soft_info) {
		const VersionHelper soft_info_version(soft_info.m_strSoftVersion);
		VersionHelper info_version(info->m_strSoftVersion);

		return info->key_name == soft_info.key_name && info_version > soft_info_version;
	};

	const auto it_find = std::find_if(m_SoftInfoArr.begin(), m_SoftInfoArr.end(), FindNewerSoftInfo);
	if (it_find != m_SoftInfoArr.end()) {
		KF_WARN(L"�ҵ��˸���汾��ע���λ�� %s", it_find->m_strSoftName.GetString());
		m_SoftInfoArr.erase(it_find);
	}

	auto FindSoftInfo = [szKeyName](const SoftInfo& info) {
		return info.key_name == szKeyName;
	};

	// �����ظ�������
	if (std::any_of(m_SoftInfoArr.begin(), m_SoftInfoArr.end(), FindSoftInfo)) {
		KF_WARN(L"�Ѿ���ע����ҵ������� %s", info->m_strSoftName);
		return false;
	}

	if (info->m_strSoftName.IsEmpty()) {
		KF_WARN(L"�汾��Ϊ�յ����� %s", info->m_strSoftName);
		return false;
	}

	if (0 == GetSize(info->m_strInstallLocation, info->m_strUninstallPth)) {
		KF_WARN(L"�Ҳ�����װĿ¼������ %s %s %s",
				info->m_strSoftName, info->m_strInstallLocation, info->m_strUninstallPth);
		return false;
	}

	return true;
}

void CSoftInfo::PushData(HKEY key, DWORD ulOptions, SoftInfo* soft_info) {
	soft_info->m_strPublisher = Helper::RegisterQueryValue(key, L"Publisher").c_str();
	soft_info->m_strMainProPath = Helper::RegisterQueryValue(key, L"InstallLocation").c_str();
	soft_info->m_strSoftIcon = GetIcon(key);
	soft_info->bit = (ulOptions & KEY_WOW64_64KEY) ? 64 : 32;

	if (soft_info->m_strSoftIcon.IsEmpty()) {
		soft_info->m_strSoftIcon = GetIcon(soft_info->m_strSoftName,
										  soft_info->m_strInstallLocation);
	}

	if (soft_info->m_strSoftVersion.IsEmpty()) {
		soft_info->m_strSoftVersion =
			FileHelper::GetFileVersion(soft_info->m_strSoftIcon).c_str();
	}

	if (soft_info->m_strSoftVersion.IsEmpty() || soft_info->m_strSoftVersion == L"1") {
		KF_WARN(L"�汾��Ϊ�յ����� %s", soft_info->m_strSoftName);
		return;
	}

	m_SoftInfoArr.push_back(*soft_info);
}

void CSoftInfo::AddSoftInfo(HKEY root_key, std::wstring_view lpSubKey, std::wstring_view szKeyName,
							DWORD ulOptions) {
	if(szKeyName.empty()) {
		return;
	}

	HKEY hkRKey;

	CString sub_path;
	sub_path.Format(L"%s\\%s", lpSubKey.data(), szKeyName.data());

	if (RegOpenKeyEx(root_key, sub_path, 0, ulOptions, &hkRKey) != ERROR_SUCCESS) {
		return;
	}

	SoftInfo soft_info;
	if(!CheckData(hkRKey, szKeyName.data(), &soft_info)) {
		return;
	}

	PushData(hkRKey, ulOptions, &soft_info);
}

void CSoftInfo::Init(HKEY root_key, DWORD ulOptions) {
	LPCTSTR lpSubKey;        // �Ӽ�����
	HKEY hkResult;            // ��Ҫ�򿪼��ľ�� 
	LONG lReturn;            // ��¼��ȡע����Ƿ�ɹ�

	lpSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

	lReturn = RegOpenKeyEx(root_key, lpSubKey, 0, ulOptions, &hkResult);

	if (lReturn == ERROR_SUCCESS) {
		DWORD dwKeyLen = 255;
		TCHAR szKeyName[255];        // ע���������
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

	::MessageBox(NULL, _T("��ע���ʧ��!"), NULL, MB_ICONWARNING);
}