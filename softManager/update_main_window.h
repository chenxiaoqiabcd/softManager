#pragma once

#include <string>

#include "WndImpl.h"

class KfString;

class UpdateMainWindow : public CWndImpl
{
public:
	~UpdateMainWindow();

	void SetUpdateData(char* data);

	void EnableRestartButton() const;
protected:
	LPCTSTR GetSkinFile() override;

	void Init() override;

	void Notify(DuiLib::TNotifyUI& msg) override;
private:
	std::wstring ConcatDescription(const std::vector<KfString>& log_list, size_t log_list_size, size_t* n);

	void AddDescription(DuiLib::CVerticalLayoutUI* layout, const char* value);

	char* update_data_;
};
