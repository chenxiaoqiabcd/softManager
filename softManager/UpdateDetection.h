#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
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

	void Run(const std::vector<SoftInfo>& soft_infos,
			 size_t start, size_t end);

	void Run(const std::vector<SoftInfo>& soft_infos, int count);

	void Run(const SoftInfo& info);

	std::optional<UpdateInfo> MatchName(std::string_view name);

	std::optional<UpdateInfo> MatchName(std::wstring_view name);

	DataCenter* GetDataCenter();
private:
	void PushUpgradeInfo(UpdateInfo& update_info) const;

	void HandlerRemoteResponse(const char* response_body, size_t soft_count);

	void HandlerRemoteResponse(const char* response_body,
							   const char* soft_name);

	std::vector<std::map<std::wstring, std::wstring>> ParseActions(const jsoncons::json& root);

	inline static std::vector<UpdateInfo> update_info_data_;
		
	std::vector<RemoteConfigInfo> remote_config_info_vec_;

	std::shared_ptr<DataCenter> center_;

	std::vector<std::thread> threads_;

	int handle_count_ = 0;

	std::mutex mutex_;
};

extern CGlobalUpdateManager* UpdateInstance;