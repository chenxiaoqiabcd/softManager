#include "UpdateDetection.h"

#include "helper.h"
#include "softInfo.h"
#include "stringHelper.h"

#include "observerMode.h"
#include "request_helper.h"

CGlobalUpdateManager* CGlobalUpdateManager::GetInstance() {
	static CGlobalUpdateManager instance;
	return &instance;
}

void CGlobalUpdateManager::Run(const std::vector<SoftInfo>& soft_infos) {
	Helper::UpdateStatusLabel(L"正在检查软件更新");

	jsoncons::json request = jsoncons::json::make_array();

	for (auto& it : soft_infos) {
		jsoncons::json item;

		item["name"] = CStringHelper::w2u(it.m_strSoftName.GetString());
		item["version"] = CStringHelper::w2u(it.m_strSoftVersion.GetString());
		item["mid"] = Helper::GetCpuId();
		item["bit"] = it.bit;

		request.emplace_back(item);
	}

	std::string request_body, response_body;
	request.dump(request_body);

	if (RequestHelper::CheckUpdate(request_body.c_str(), &response_body)) {
		HandlerRemoteResponse(response_body.c_str());
	}

	Helper::UpdateStatusLabel(L"");
}

std::optional<UpdateInfo> CGlobalUpdateManager::MatchName(std::string_view name) {
	auto findUpdateData = [name](const UpdateInfo& info) {
		return CStringHelper::IsMatch(name.data(), info.name);
	};

	const auto it_find = std::find_if(update_info_data_.begin(), update_info_data_.end(), findUpdateData);

	if(it_find != update_info_data_.end()) {
		return *it_find;
	}

	return std::nullopt;
}

std::optional<UpdateInfo> CGlobalUpdateManager::MatchName(std::wstring_view name) {
	return MatchName(CStringHelper::w2a(name.data()));
}

DataCenter* CGlobalUpdateManager::GetDataCenter() {
	if(!center_) {
		center_ = std::make_shared<DataCenter>();
	}

	return center_.get();
}

void CGlobalUpdateManager::PushUpgradeInfo(UpdateInfo& update_info) const {
	if (std::any_of(update_info_data_.begin(), update_info_data_.end(),
					[update_info](const UpdateInfo& info) {return info.name == update_info.name; })) {
		return;
	}

	center_->UpdateDate(update_info.need_update, &update_info);

	update_info_data_.push_back(update_info);
}

void CGlobalUpdateManager::HandlerRemoteResponse(const char* response_body) {
	auto w_body = CStringHelper::u2w(response_body);

	update_info_data_.clear();
	center_->ClearData();

	jsoncons::json response = jsoncons::json::parse(response_body);
	const auto size = response.size();
	for (size_t n = 0; n < size; ++n) {
		std::string remote_version;
		std::string package_url;

		std::wstring message;
		std::wstring type;

		bool need_update = false;

		if (response[n].contains("message"))
			message = CStringHelper::u2w(response[n]["message"].as_string());

		if (response[n].contains("need_update")) {
			need_update = response[n]["need_update"].as_bool();
		}

		if(!need_update && message.empty()) {
			continue;
		}

		if (response[n].contains("remote_version"))
			remote_version = CStringHelper::u2a(response[n]["remote_version"].as_string());

		if (response[n].contains("package_url"))
			package_url = CStringHelper::u2a(response[n]["package_url"].as_string());

		if (response[n].contains("type"))
			type = CStringHelper::u2w(response[n]["type"].as_string());

	 	const auto& actions = ParseActions(response[n]);

		UpdateInfo info {
			need_update,
			CStringHelper::u2a(response[n]["name"].as_string()),
			package_url,
			remote_version,
			message,
			type,
			actions,
		};

		PushUpgradeInfo(info);
	}
}

std::vector<std::map<std::wstring, std::wstring>> CGlobalUpdateManager::ParseActions(const jsoncons::json& root) {
	std::vector<std::map<std::wstring, std::wstring>> actions;

	if (!root.contains("actions")) {
		return actions;
	}

	const auto size = root["actions"].size();
	for (size_t m = 0; m < size; ++m) {
		std::map<std::wstring, std::wstring> action;

		auto name = CStringHelper::u2w(root["actions"][m]["name"].as_string());

		action[L"name"] = name;

		if(name == L"clipboard") {
			const auto text = CStringHelper::u2w(root["actions"][m]["text"].as_string());

			action[L"text"] = text;

			actions.emplace_back(action);
		}
	}

	return actions;
}

CGlobalUpdateManager* UpdateInstance = CGlobalUpdateManager::GetInstance();
