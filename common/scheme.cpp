#include "scheme.h"

#include "helper.h"

void Scheme::SetDarkMode(DuiLib::CPaintManagerUI* pm) {
	mode_ = SchemeMode::Dark;

	button_bk_color_ = 0xFF2e2e2e;
	button_bk_hot_color_ = 0xFF3d3d3d;
	button_bk_pressed_color_ = 0xFF2e2e2e;

	button_text_color_ = 0xFFf2fbec;

	caption_bk_color = 0xFF202020;
	window_bk_color = 0xFF191919;

	Refresh(pm);
}

void Scheme::SetLightMode(DuiLib::CPaintManagerUI* pm) {
	mode_ = SchemeMode::Light;

	button_text_color_ = 0xFF333333;
	button_bk_color_ = 0xFFEEEEEE;
	button_bk_hot_color_ = 0xFFe2effc;
	button_bk_pressed_color_ = 0xFFcce4f7;

	caption_bk_color = 0xFFEEEEEE;
	window_bk_color = 0xFFFFFFFF;

	Refresh(pm);
}

void Scheme::SetDefaultMode(DuiLib::CPaintManagerUI* pm) {
	const auto theme = GetSystemTheme();

	switch (theme) {
	case SchemeMode::Light:
		SetLightMode(pm);
		break;
	default:
		SetDarkMode(pm);
		break;
	}
}

void Scheme::Refresh(DuiLib::CButtonUI* button) const {
	button->SetTextColor(button_text_color_);
	button->SetBkColor(button_bk_color_);
}

void Scheme::Refresh(DuiLib::CEditUI* edit) const {
	edit->SetTextColor(button_text_color_);
	edit->SetNativeEditTextColor(button_text_color_);
	edit->SetBkColor(button_bk_color_);
	edit->SetNativeEditBkColor(button_bk_color_);
}

void Scheme::Refresh(DuiLib::CLabelUI* label) const {
	label->SetTextColor(button_text_color_);
}

SchemeMode Scheme::GetMode() const {
	return mode_;
}

DWORD Scheme::GetTextColor() const {
	return button_text_color_;
}

DWORD Scheme::GetBkColor() const {
	return button_bk_color_;
}

DWORD Scheme::GetHotBkColor() const {
	return button_bk_hot_color_;
}

DWORD Scheme::GetPressBkColor() const {
	return button_bk_pressed_color_;
}

SchemeMode Scheme::GetSystemTheme() {
	HKEY key;
	auto res = RegOpenKeyEx(HKEY_CURRENT_USER,
							L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
							0, KEY_READ, &key);
	if(res != ERROR_SUCCESS) {
		return SchemeMode::None;
	}

	const auto theme = Helper::RegisterQueryDWordValue(key, L"AppsUseLightTheme");

	RegCloseKey(key);

	return theme == 1 ? SchemeMode::Light : SchemeMode::Dark;
}

void Scheme::Refresh(DuiLib::CPaintManagerUI* pm) const {
	RefreshButton(pm);
	RefreshEdit(pm);
	RefreshLabel(pm);
	RefreshList(pm);

	const auto caption = pm->FindControl(L"caption");
	if (nullptr != caption) {
		caption->SetBkColor(caption_bk_color);
	}

	pm->GetRoot()->SetBkColor(window_bk_color);

	pm->NeedUpdate();
}

void Scheme::RefreshButton(DuiLib::CPaintManagerUI* pm) const {
	int index = 0;

	while (true) {
		const auto button = dynamic_cast<DuiLib::CButtonUI*>(pm->FindSubControlByClass(nullptr, L"ButtonUI",
																					   index++));
		if (nullptr == button) {
			break;
		}

		Refresh(button);
	}
}

void Scheme::RefreshEdit(DuiLib::CPaintManagerUI* pm) const {
	int index = 0;

	while (true) {
		const auto edit = dynamic_cast<DuiLib::CEditUI*>(pm->FindSubControlByClass(nullptr, L"EditUI",
																				   index++));
		if (nullptr == edit) {
			break;
		}

		Refresh(edit);
	}
}

void Scheme::RefreshLabel(DuiLib::CPaintManagerUI* pm) const {
	int index = 0;

	while (true) {
		const auto label = dynamic_cast<DuiLib::CLabelUI*>(pm->FindSubControlByClass(nullptr, L"LabelUI",
																					   index++));
		if (nullptr == label) {
			break;
		}

		Refresh(label);
	}
}

void Scheme::RefreshList(DuiLib::CPaintManagerUI* pm) const {
	int index = 0;

	while (true) {
		const auto list = dynamic_cast<DuiLib::CListUI*>(pm->FindSubControlByClass(nullptr, L"ListUI",
																				   index++));
		if (nullptr == list) {
			break;
		}

		list->SetHotItemBkColor(button_bk_hot_color_);
		list->SetSelectedItemBkColor(button_bk_pressed_color_);

		auto header = list->GetHeader();

		header->SetBkColor(button_bk_color_);

		auto header_count = header->GetCount();
		for(int n = 0; n < header_count; ++n) {
			static_cast<DuiLib::CListHeaderItemUI*>(header->GetItemAt(n))->SetTextColor(button_text_color_);
		}
	}
}
