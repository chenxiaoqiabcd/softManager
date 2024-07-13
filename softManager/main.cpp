#include <Windows.h>

#include "helper.h"
#include "log_helper.h"
#include "mail.h"
#include "mainWindow.h"
#include "mp.h"
#include "resource.h"
#include "track.h"

#pragma comment(lib, "Msi.lib")

using DuiLib::CPaintManagerUI;

bool SetResourceZip(HINSTANCE hInstance);

bool IsOneInstance(const wchar_t* mutex_name);

bool IsRestart();

void UploadErrorLog();

LONG WINAPI MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);

int WINAPI wWinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance,
					__in LPWSTR lpCmdLine, __in int nCmdShow) {
	if (!IsOneInstance(L"{D93797CB-E79B-451C-B7BB-57A5FBE54288}")) {
		if (!IsRestart())
			return 0;
	}

	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	KfLog::EnableLocalLog();

	wchar_t bak_path[MAX_PATH];
	ZeroMemory(bak_path, MAX_PATH * sizeof(wchar_t));
	GetModuleFileName(nullptr, bak_path, MAX_PATH);
	wcscat_s(bak_path, L".bak");

	DeleteFile(bak_path);

	MemoryPool::GetInstance()->Init(30);

	Helper::CreateDesktopIcon(L"�����������");

	SetResourceZip(hInstance);

	try {
		//����������
		auto pFrame = std::make_shared<CMainWindow>();
		pFrame->Create(NULL, L"", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
		pFrame->CenterWindow();
		CPaintManagerUI::MessageLoop();
	}
	catch(std::exception& e) {
		KF_ERROR("%s", e.what());
	}

	if (KfLog::HasErrorLog()) {
		UploadErrorLog();
	}

	return 0;
}

bool SetResourceZip(HINSTANCE hInstance) {
	CPaintManagerUI::SetInstance(hInstance);

	HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(),
		MAKEINTRESOURCE(IDS_SKIN_ZIP), _T("SKIN"));
	if (hResource == nullptr)
		return false;
	HGLOBAL hGlobal = LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
	if (hGlobal == nullptr) {
		::FreeResource(hResource);
		return false;
	}

	DWORD dwResourceSize = SizeofResource(CPaintManagerUI::GetResourceDll(), hResource);
	if (dwResourceSize == 0)
		return false;
	BYTE* lpResourceZIPBuffer = new BYTE[dwResourceSize];
	if (lpResourceZIPBuffer != nullptr)
		::CopyMemory(lpResourceZIPBuffer, ::LockResource(hGlobal), dwResourceSize);

	FreeResource(hResource);

	CPaintManagerUI::SetResourceZip(lpResourceZIPBuffer, dwResourceSize);

	return true;
}

bool IsOneInstance(const wchar_t* mutex_name) {
	HANDLE mutex = CreateMutexW(nullptr, TRUE, mutex_name);
	if ((mutex != nullptr) && (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		ReleaseMutex(mutex);
		return false;
	}
	return true;
}

bool IsRestart() {
	int args = 0;
	const auto command_lines = CommandLineToArgvW(GetCommandLine(), &args);
	if (2 == args) {
		if (StrStrIW(command_lines[1], L"--restart")) {
			return true;
		}
	}

	return false;
}

LONG WINAPI MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo) {
	KF_ERROR("exception...");

	ExceptionFilter(pExceptionInfo);

	UploadErrorLog();

	return EXCEPTION_EXECUTE_HANDLER;
}

void UploadErrorLog() {
	SimpleSslSmtpEmail ssl_smtp_email("smtp.qq.com", "465");
	ssl_smtp_email.SendEmail("765284382@qq.com", "qmuxmuntfjozbfec",
							 { "chenxiaoqiabcd@outlook.com" }, "������������쳣", "������������쳣",
							 { KfLog::GetLogPath() }, {});
}