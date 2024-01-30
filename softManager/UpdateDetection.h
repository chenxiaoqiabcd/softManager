#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <jsoncons/json.hpp>

class CWkeViewUI;
struct SoftInfo;
class CWkeWindowImpl;
class DataCenter;

/*
* 全局更新管理
*/
struct UpdateInfo {
	bool need_update;

	std::string name;
	std::string url;
	std::string version;
	std::wstring msg;
	std::wstring type;

	std::vector<std::map<std::wstring, std::wstring>> actions;
};

struct RemoteConfigInfo {
	std::string name;
	std::string url;
	std::string url_path;
	std::string version_path;
	std::string local_version;
};

class CGlobalUpdateManager {
private:
	CGlobalUpdateManager() {}

	CGlobalUpdateManager(CGlobalUpdateManager&) = delete;
public:
	static CGlobalUpdateManager* GetInstance();

	void Run(const std::vector<SoftInfo>& soft_infos);
	
	std::optional<UpdateInfo> MatchName(std::string_view name);

	std::optional<UpdateInfo> MatchName(std::wstring_view name);

	DataCenter* GetDataCenter();
private:
	void PushUpgradeInfo(UpdateInfo& update_info) const;

	void HandlerRemoteResponse(const char* response_body);

	std::vector<std::map<std::wstring, std::wstring>> ParseActions(const jsoncons::json& root);

	inline static std::vector<UpdateInfo> update_info_data_;

	size_t need_update_count_ = 0;	// 需要更新的数量
	
	std::vector<RemoteConfigInfo> remote_config_info_vec_;

	std::shared_ptr<DataCenter> center_;
};

extern CGlobalUpdateManager* UpdateInstance;