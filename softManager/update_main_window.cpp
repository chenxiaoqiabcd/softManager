#include "update_main_window.h"

#include <jsoncons/basic_json.hpp>

#include "log_helper.h"
#include "kf_str.h"
#include "stringHelper.h"

UpdateMainWindow::~UpdateMainWindow() {
	delete update_data_;
	update_data_ = nullptr;
}

void UpdateMainWindow::SetUpdateData(char* data) {
	std::swap(update_data_, data);
}

void UpdateMainWindow::EnableRestartButton() const {
	m_pm.FindControl(L"btn_restart")->SetEnabled(true);
}

LPCTSTR UpdateMainWindow::GetSkinFile() {
	return L"updateMainWnd.xml";
}

void UpdateMainWindow::Init() {
	m_pm.FindControl(L"btn_restart")->SetEnabled(false);

	auto log_layout = static_cast<DuiLib::CVerticalLayoutUI*>(m_pm.FindControl(L"log_layout"));

	try {
		jsoncons::json root = jsoncons::json::parse(update_data_);
		auto size = root.size();
		for(auto i = 0; i < size; ++i) {
			DuiLib::CLabelUI* label_title = new DuiLib::CLabelUI;
			label_title->SetFixedHeight(22);
			label_title->SetText(CStringHelper::u2w(root[i]["CreateTime"].as_string()).c_str());

			log_layout->Add(label_title);

			auto label_version = new DuiLib::CLabelUI;
			label_version->SetFixedHeight(22);
			label_version->SetText(CStringHelper::u2w(root[i]["Version"].as_string()).c_str());

			log_layout->Add(label_version);

			AddDescription(log_layout, root[i]["Description"].as_string().c_str());
		}
	}
	catch(std::exception& e) {
		KF_ERROR("cache error: %s", e.what());
	}
}

void UpdateMainWindow::Notify(DuiLib::TNotifyUI& msg) {
	if(msg.sType == DUI_MSGTYPE_CLICK) {
		auto name = msg.pSender->GetName();

		if(0 == name.Compare(L"closebtn")) {
			Close(0);
			return;
		}

		if (0 == name.Compare(L"btn_restart")) {
			PostQuitMessage(0);

			wchar_t file_name[MAX_PATH];
			ZeroMemory(file_name, sizeof(wchar_t) * MAX_PATH);
			GetModuleFileName(nullptr, file_name, MAX_PATH);

			ShellExecute(nullptr, L"open", file_name, L"--restart", nullptr, SW_SHOW);
			return;
		}
	}
}

std::wstring UpdateMainWindow::ConcatDescription(const vector<KfString>& log_list, size_t log_list_size,
												 size_t* n) {
	auto content = log_list[*n];
	while (*n < log_list_size - 1) {
		const auto& next_content = log_list[*n + 1];
		if (next_content[0] >= '0' && next_content[0] <= '9' && next_content[1] == '.') {
			break;
		}

		content.Append(" " + next_content);
		(*n)++;
	}

	return content.GetWString();
}

void UpdateMainWindow::AddDescription(DuiLib::CVerticalLayoutUI* layout, const char* value) {
	KfString description(value);
	auto log_list = description.Split(" ");

	const auto log_list_size = log_list.size();

	for (size_t n = 0; n < log_list_size; ++n) {
		auto description = ConcatDescription(log_list, log_list_size, &n);

		const auto label_content = new DuiLib::CLabelUI;
		label_content->SetFixedHeight(22);
		label_content->SetPadding({ 20, 0, 0, n == log_list_size - 1 ? 20 : 0 });
		label_content->SetText(description.c_str());

		layout->Add(label_content);
	}
}
