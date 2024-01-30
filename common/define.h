#pragma once

#include <string>

using std::wstring;

struct SoftwareInfo {
	// ���ͼ��
	wstring soft_icon_;

	// �������
	wstring soft_name_;

	// �����汾
	wstring soft_local_ver_;

	// ���°汾
	wstring soft_remote_ver_;

	// ���ش�С
	wstring soft_size_;

	// ��������
	wstring soft_download_url;
};

#define WM_USER_EVENT_MSG				(WM_USER + 1002)
#define WM_USER_SHOW_UPDATE_WINDOW		(WM_USER + 1003)
#define WM_USER_DOWNLOAD_FAILED_PACKAGE (WM_USER + 1004)