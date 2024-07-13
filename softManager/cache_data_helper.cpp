#include "cache_data_helper.h"

#include <fstream>

#include "helper.h"
#include "stringHelper.h"

void CacheDataHelper::SetValue(const wchar_t* key, const wchar_t* value) {
	const auto u_key = CStringHelper::w2u(key);
	auto u_value = CStringHelper::w2u(value);

	if (root_.empty()) {
		UpdateRootWithCacheFile(&root_);
	}

	root_[u_key] = u_value;

	auto out = std::ofstream(GetFilePath(), std::ios::out);
	root_.dump(out);
}

std::wstring CacheDataHelper::GetValue(const wchar_t* key) {
	const auto u_key = CStringHelper::w2u(key);

	if (root_.empty()) {
		UpdateRootWithCacheFile(&root_);
	}

	if(root_.contains(u_key)) {
		return CStringHelper::u2w(root_[u_key].as_string());
	}

	return L"";
}

std::string CacheDataHelper::GetFilePath() {
	return Helper::GetCacheFile("cache.data");
}

void CacheDataHelper::UpdateRootWithCacheFile(jsoncons::json* root) {
	try {
		auto in = std::ifstream(GetFilePath(), std::ios::in);
		*root = jsoncons::json::parse(in);
	}
	catch (std::exception& e) {
		// 文件是空的而已，问题不大，不需要报异常
		// KF_ERROR("failed update root with cache file, error: %s", e.what());
	}
}











void SoftInfoCacheData::SetValue(const SoftInfo& info, HKEY root_key, DWORD ulOptions) {
	auto key = CStringHelper::w2u(info.key_name.GetString());

	jsoncons::json root;

	root["icon"] = CStringHelper::w2u(info.m_strSoftIcon.GetString());
	root["name"] = CStringHelper::w2u(info.m_strSoftName.GetString());
	root["version"] = CStringHelper::w2u(info.m_strSoftVersion.GetString());
	root["install_location"] = CStringHelper::w2u(info.m_strInstallLocation.GetString());
	root["publisher"] = CStringHelper::w2u(info.m_strPublisher.GetString());
	root["main_pro_path"] = CStringHelper::w2u(info.m_strMainProPath.GetString());
	root["uninstall_path"] = CStringHelper::w2u(info.m_strUninstallPth.GetString());
	root["bit"] = info.bit;
	root["time_stamp"] = info.time_stamp;
	root["key"] = reinterpret_cast<long long>(root_key);
	root["options"] = ulOptions;

	std::string root_data;
	root.dump(root_data);

	CacheDataHelper::SetValue(info.key_name.GetString(), CStringHelper::u2w(root_data).c_str());
}

bool SoftInfoCacheData::GetValue(const wchar_t* key_name, HKEY root_key, DWORD ulOptions, SoftInfo* ptr_info) {
	const auto root_data = CacheDataHelper::GetValue(key_name);
	if (root_data.empty()) {
		return false;
	}

	auto root = jsoncons::json::parse(CStringHelper::w2u(root_data));

	if (root["key"].as<long long>() != reinterpret_cast<long long>(root_key) ||
		root["options"].as<DWORD>() != ulOptions) {
		return false;
	}

	ptr_info->m_strSoftIcon = CStringHelper::u2w(root["icon"].as_string()).c_str();
	ptr_info->m_strSoftName = CStringHelper::u2w(root["name"].as_string()).c_str();
	ptr_info->m_strSoftVersion = CStringHelper::u2w(root["version"].as_string()).c_str();
	ptr_info->m_strInstallLocation = CStringHelper::u2w(root["install_location"].as_string()).c_str();
	ptr_info->m_strPublisher = CStringHelper::u2w(root["publisher"].as_string()).c_str();
	ptr_info->m_strMainProPath = CStringHelper::u2w(root["main_pro_path"].as_string()).c_str();
	ptr_info->m_strUninstallPth = CStringHelper::u2w(root["uninstall_path"].as_string()).c_str();
	ptr_info->bit = root["bit"].as<uint8_t>();
	ptr_info->time_stamp = root["time_stamp"].as<time_t>();
	ptr_info->key_name = key_name;

	return true;
}
