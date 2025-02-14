#pragma once

#include <string>

#include "WndImpl.h"

class AddDownloadTaskWindow : public CWndImpl
{
public:
	std::wstring GetUrl() const;

	std::wstring GetSavePath() const;

	bool GetAcceptRanges() const;

	double GetSize() const;
protected:
	LPCTSTR GetSkinFile() override;

	void Init() override;

	void NotifyTextChanged(DuiLib::TNotifyUI& msg);

	void Notify(DuiLib::TNotifyUI& msg) override;
private:
	double size_ = 0.0f;
	bool accept_ranges_ = false;
};
