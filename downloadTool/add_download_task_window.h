#pragma once

#include "WndImpl.h"

class AddDownloadTaskWindow : public CWndImpl
{
protected:
	LPCTSTR GetSkinFile() override;

	void Init() override;

	void NotifyClickOk(DuiLib::TNotifyUI& msg);

	void Notify(DuiLib::TNotifyUI& msg) override;
};
