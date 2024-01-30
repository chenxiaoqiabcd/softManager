#pragma once

#include <string>

using std::wstring;

struct SoftwareInfo {
	// 软件图标
	wstring soft_icon_;

	// 软件名称
	wstring soft_name_;

	// 本机版本
	wstring soft_local_ver_;

	// 最新版本
	wstring soft_remote_ver_;

	// 本地大小
	wstring soft_size_;

	// 下载链接
	wstring soft_download_url;
};

#define WM_USER_EVENT_MSG				(WM_USER + 1002)
#define WM_USER_SHOW_UPDATE_WINDOW		(WM_USER + 1003)
#define WM_USER_DOWNLOAD_FAILED_PACKAGE (WM_USER + 1004)