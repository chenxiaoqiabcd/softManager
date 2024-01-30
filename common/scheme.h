#pragma once

#include <UIlib.h>

enum class SchemeMode
{
	None,
	Dark,
	Light
};

class Scheme
{
public:
	void SetDarkMode(DuiLib::CPaintManagerUI* pm);

	void SetLightMode(DuiLib::CPaintManagerUI* pm);

	void SetDefaultMode(DuiLib::CPaintManagerUI* pm);

	void Refresh(DuiLib::CButtonUI* button) const;

	void Refresh(DuiLib::CEditUI* edit) const;

	void Refresh(DuiLib::CLabelUI* label) const;

	SchemeMode GetMode() const;

	DWORD GetTextColor() const;

	DWORD GetBkColor() const;

	DWORD GetHotBkColor() const;

	DWORD GetPressBkColor() const;

	SchemeMode GetSystemTheme();
private:
	void Refresh(DuiLib::CPaintManagerUI* pm) const;

	void RefreshButton(DuiLib::CPaintManagerUI* pm) const;

	void RefreshEdit(DuiLib::CPaintManagerUI* pm) const;

	void RefreshLabel(DuiLib::CPaintManagerUI* pm) const;

	void RefreshList(DuiLib::CPaintManagerUI* pm) const;

	DWORD button_text_color_ = 0;
	DWORD button_bk_color_ = 0;
	DWORD button_bk_hot_color_ = 0;
	DWORD button_bk_pressed_color_ = 0;

	DWORD caption_bk_color = 0;
	DWORD window_bk_color = 0;

	inline static SchemeMode mode_ = SchemeMode::None;
};