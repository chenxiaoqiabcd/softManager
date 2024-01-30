#pragma once

#include <memory>
#include <UIlib.h>

class Scheme;

class CDialogBuilderCallbackEx : public DuiLib::IDialogBuilderCallback {
public:
	DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override;
};

class CWndImpl : public DuiLib::CWindowWnd, public DuiLib::INotifyUI
{
public:
	void Notify(DuiLib::TNotifyUI& msg) override;
protected:
	virtual LPCTSTR GetSkinFile() = 0;

	virtual void Init() = 0;
	
	LPCTSTR GetWindowClassName() const override;

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle);

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle);
	
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
public:
	DuiLib::CPaintManagerUI m_pm;

	std::shared_ptr<Scheme> scheme_;
};
