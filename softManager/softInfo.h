// ------------------------------------------------------------------------------------------------------------------------
// FlieNmae: 
//   SofInfo.h
// Remark:
//   通过读取注册表获得本机已安装软件信息。
// ------------------------------------------------------------------------------------------------------------------------
#pragma once

#include <atlstr.h>
#include <vector>
#include <mutex>

#include <jsoncons/json.hpp>

struct SoftInfo
{
	// 软件图标程序（通常是运行程序）
	CString m_strSoftIcon;
	// 软件名
	CString m_strSoftName;
	// 软件版本号
	CString m_strSoftVersion;
	// 软件安装目录
	CString m_strInstallLocation;
	// 软件发布厂商
	CString m_strPublisher;
	// 主程序所在完整路径
	CString m_strMainProPath;
	// 卸载exe所在完整路径
	CString m_strUninstallPth;
	// 注册表项名称
	CString key_name;

	uint8_t bit;	// 32/64

	time_t time_stamp;
};

class CSoftInfo
{
	CSoftInfo();
	CSoftInfo(const CSoftInfo&) = delete;
public:
 	static CSoftInfo* GetInstance() {
		CSoftInfo instance;
		return &instance;
	}

	// 获取一个包含常用软件安装信息的Vector
	std::vector<SoftInfo> GetSoftInfo(void);

	void UpdateSoftInfo();

	void UpdateSoftInfo(const wchar_t* key_name);

	// 获取所有已安装常用软件名
	void GetSoftName(std::vector<LPCTSTR>& lpszSoftName);
	// 获取所有已安装常用软件版本号
	void GetSoftVersion(std::vector<LPCTSTR>& lpszSoftVersion);
	// 获取所有已安装常用软件安装目录
	void GetInstallLocation(std::vector<LPCTSTR>& lpszInstallLocation);
	// 获取所有已安装常用软件发布厂商
	void GetPublisher(std::vector<LPCTSTR>& lpszPublisher);
	// 获取所有已安装常用软件主程序所在路径
	void GetMainProPath(std::vector<LPCTSTR>& lpszMainProPath);
	// 获取所有已安装常用软件卸载程序所在路径
	void GetUninstallPth(std::vector<LPCTSTR>& lpszSoftName);

	// 获取一个包含系统补丁信息的Vector
	std::vector<SoftInfo> GetSystemPatchesInfo(void) const;
	// 获取所有已安装系统补丁名
	void GetSystemPatchesName(std::vector<LPCTSTR>& lpszSoftName);
protected:
	// 获取程序图标
	CString GetIcon(HKEY key);

	SoftInfo GenerateSoftInfo(HKEY key, const wchar_t* key_name, DWORD ulOptions);

	bool CheckSoftInfo(SoftInfo* info);

	SoftInfo GetSoftInfo(HKEY hkRKey, std::wstring_view szKeyName, DWORD ulOptions);

	void AddSoftInfo(HKEY root_key, std::wstring_view lpSubKey,
										std::wstring_view szKeyName, DWORD ulOptions);

	void Init(HKEY root_key, DWORD ulOptions);

	void Init(HKEY root_key, DWORD ulOptions, const wchar_t* key_name);
private:
	// 保存已安装常用软件安装信息
	inline static std::vector<SoftInfo> m_SoftInfoArr;
	// 保存系统补丁信息
	inline static std::vector<SoftInfo> m_SystemPatchesArr;

	inline static std::mutex mtx_;
};

