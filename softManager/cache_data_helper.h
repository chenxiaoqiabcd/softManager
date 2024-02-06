#pragma once

#include <string>

#include <jsoncons/json.hpp>

#include "softInfo.h"

class CacheDataHelper {
public:
	static void SetValue(const wchar_t* key, const wchar_t* value);

	static std::wstring GetValue(const wchar_t* key);
private:
	static std::string GetFilePath();

	static void UpdateRootWithCacheFile(jsoncons::json* root);

	inline static jsoncons::json root_;
};











class SoftInfoCacheData {
public:
	static void SetValue(const SoftInfo& info, HKEY root_key, DWORD ulOptions);

	static bool GetValue(const wchar_t* key_name, HKEY root_key, DWORD ulOptions, SoftInfo* ptr_info);
};