#include <memory>
#include <Windows.h>

#include "log_helper.h"
#include "main_window.h"
#include "resource.h"

bool IsOneInstance(const wchar_t* mutex_name);

bool SetResourceZip(HINSTANCE hInstance);

int WINAPI wWinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance,
					__in LPWSTR lpCmdLine, __in int nCmdShow) {
	if(!IsOneInstance(L"{365AA6CD-25D4-4ABF-B74D-42BCB6EF9CF5}")) {
		return 0;
	}

	SetResourceZip(nullptr);

	try {
		//创建主窗口
		auto pFrame = std::make_shared<MainWindow>();
		pFrame->Create(NULL, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
		pFrame->CenterWindow();
		DuiLib::CPaintManagerUI::MessageLoop();
	}
	catch (std::exception& e) {
		KF_ERROR("%s", e.what());
	}

    return 0;
}

bool IsOneInstance(const wchar_t* mutex_name) {
	HANDLE mutex = CreateMutexW(nullptr, TRUE, mutex_name);
	if ((mutex != nullptr) && (GetLastError() == ERROR_ALREADY_EXISTS)) {
		ReleaseMutex(mutex);
		return false;
	}

	return true;
}

bool SetResourceZip(HINSTANCE hInstance) {
	DuiLib::CPaintManagerUI::SetInstance(hInstance);

	HRSRC hResource = ::FindResource(DuiLib::CPaintManagerUI::GetResourceDll(),
	                                 MAKEINTRESOURCE(IDS_SKIN_ZIP), _T("SKIN"));
	if (hResource == nullptr)
		return false;
	HGLOBAL hGlobal = LoadResource(DuiLib::CPaintManagerUI::GetResourceDll(), hResource);
	if (hGlobal == nullptr) {
		::FreeResource(hResource);
		return false;
	}

	DWORD dwResourceSize = SizeofResource(DuiLib::CPaintManagerUI::GetResourceDll(), hResource);
	if (dwResourceSize == 0)
		return false;
	BYTE* lpResourceZIPBuffer = new BYTE[dwResourceSize];
	if (lpResourceZIPBuffer != nullptr)
		::CopyMemory(lpResourceZIPBuffer, ::LockResource(hGlobal), dwResourceSize);

	FreeResource(hResource);

	DuiLib::CPaintManagerUI::SetResourceZip(lpResourceZIPBuffer, dwResourceSize);

	return true;
}