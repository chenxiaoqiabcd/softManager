// ------------------------------------------------------------------------------------------------------------------------
// FlieNmae: 
//   SofInfo.h
// Remark:
//   ͨ����ȡע����ñ����Ѱ�װ�����Ϣ��
// ------------------------------------------------------------------------------------------------------------------------
#pragma once

#include <atlstr.h>
#include <vector>
#include <mutex>

struct SoftInfo
{
	// ���ͼ�����ͨ�������г���
	CString m_strSoftIcon;
	// �����
	CString m_strSoftName;
	// ����汾��
	CString m_strSoftVersion;
	// �����װĿ¼
	CString m_strInstallLocation;
	// �����������
	CString m_strPublisher;
	// ��������������·��
	CString m_strMainProPath;
	// ж��exe��������·��
	CString m_strUninstallPth;
	// ע���������
	CString key_name;

	uint8_t bit;	// 32/64
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

	// ��ȡһ���������������װ��Ϣ��Vector
	std::vector<SoftInfo> GetSoftInfo(void);

	void UpdateSoftInfo();

	// ��ȡ�����Ѱ�װ���������
	void GetSoftName(std::vector<LPCTSTR>& lpszSoftName);
	// ��ȡ�����Ѱ�װ��������汾��
	void GetSoftVersion(std::vector<LPCTSTR>& lpszSoftVersion);
	// ��ȡ�����Ѱ�װ���������װĿ¼
	void GetInstallLocation(std::vector<LPCTSTR>& lpszInstallLocation);
	// ��ȡ�����Ѱ�װ���������������
	void GetPublisher(std::vector<LPCTSTR>& lpszPublisher);
	// ��ȡ�����Ѱ�װ�����������������·��
	void GetMainProPath(std::vector<LPCTSTR>& lpszMainProPath);
	// ��ȡ�����Ѱ�װ�������ж�س�������·��
	void GetUninstallPth(std::vector<LPCTSTR>& lpszSoftName);

	// ��ȡһ������ϵͳ������Ϣ��Vector
	std::vector<SoftInfo> GetSystemPatchesInfo(void) const;
	// ��ȡ�����Ѱ�װϵͳ������
	void GetSystemPatchesName(std::vector<LPCTSTR>& lpszSoftName);
protected:
	// ��ȡ����ͼ��
	CString GetIcon(HKEY key);

	CString GetIcon(CString soft_name, CString install_path);

	bool CheckData(HKEY key, std::wstring_view szKeyName, SoftInfo* info);

	void PushData(HKEY key, DWORD ulOptions, SoftInfo* soft_info);

	void AddSoftInfo(HKEY root_key, std::wstring_view lpSubKey, std::wstring_view szKeyName,
					 DWORD ulOptions);

	void Init(HKEY root_key, DWORD ulOptions);
private:
	// �����Ѱ�װ���������װ��Ϣ
	inline static std::vector<SoftInfo> m_SoftInfoArr;
	// ����ϵͳ������Ϣ
	inline static std::vector<SoftInfo> m_SystemPatchesArr;

	inline static std::mutex mtx_;
};

