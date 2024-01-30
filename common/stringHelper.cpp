#include "stringHelper.h"

#include <Windows.h>
#include <codecvt>
#include <algorithm>

wstring CStringHelper::u2w(const string& utf8Str) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.from_bytes(utf8Str);
}

std::string CStringHelper::w2u(const std::wstring& utf16Str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.to_bytes(utf16Str);
}

std::string CStringHelper::a2u(const std::string& str)
{
	std::wstring w;
	auto l = MultiByteToWideChar(CP_ACP, NULL, str.c_str(), -1, NULL, NULL);
	w.resize(l - 1);
	l = MultiByteToWideChar(CP_ACP, NULL, str.c_str(), -1, &w[0], l);
	l = WideCharToMultiByte(CP_UTF8, NULL, w.c_str(), -1, NULL, NULL, NULL, NULL);
	std::string u;
	u.resize(l - 1);
	l = WideCharToMultiByte(CP_UTF8, NULL, w.c_str(), -1, &u[0], l, NULL, NULL);
	return u;
}

std::string CStringHelper::u2a(const std::string& str)
{
	std::wstring w;
	auto l = MultiByteToWideChar(CP_UTF8, NULL, str.c_str(), -1, NULL, NULL);
	w.resize(l - 1);
	l = MultiByteToWideChar(CP_UTF8, NULL, str.c_str(), -1, &w[0], l);
	l = WideCharToMultiByte(CP_ACP, NULL, w.c_str(), -1, NULL, NULL, NULL, NULL);
	std::string u;
	u.resize(l - 1);
	l = WideCharToMultiByte(CP_ACP, NULL, w.c_str(), -1, &u[0], l, NULL, NULL);
	return u;
}

std::wstring CStringHelper::a2w(const std::string& str)
{
	std::wstring w;
	auto l = MultiByteToWideChar(CP_ACP, NULL, str.c_str(), -1, NULL, NULL);
	w.resize(l - 1);
	l = MultiByteToWideChar(CP_ACP, NULL, str.c_str(), -1, &w[0], l);
	return w;
}

std::string CStringHelper::w2a(const std::wstring& w)
{
	auto l = WideCharToMultiByte(CP_ACP, NULL, w.c_str(), -1, NULL, NULL, NULL, NULL);
	std::string a;
	a.resize(l - 1);
	l = WideCharToMultiByte(CP_ACP, NULL, w.c_str(), -1, &a[0], l, NULL, NULL);
	return a;
}

string CStringHelper::Trim(string src, char res) {
	while(!src.empty() && src[0] == res) {
		src = src.substr(1);
	}

	while(!src.empty() && src[src.length()-1] == res) {
		src = src.substr(0, src.length() - 1);
	}

	return src;
}

vector<string> CStringHelper::Split(const string& value, const string& filter) {
	vector<string> resVec;
	
	char* p;
	char* pRes = strtok_s((char*)value.c_str(), filter.c_str(), &p);

	while(pRes != nullptr) {
		resVec.push_back(pRes);
		pRes = strtok_s(nullptr, filter.c_str(), &p);
	}

	return resVec;
}

vector<wstring> CStringHelper::Split(const wstring& value, const wstring& filter) {
	vector<wstring> resVec;

	wchar_t* p;
	wchar_t* pRes = wcstok_s((wchar_t*)value.c_str(), filter.c_str(), &p);

	while (pRes != nullptr) {
		resVec.push_back(pRes);
		pRes = wcstok_s(nullptr, filter.c_str(), &p);
	}

	return resVec;
}

bool CStringHelper::IsMatch(std::string source_data, std::string match_data) {
	if (nullptr == strstr(match_data.c_str(), "*")) {
		return 0 == _stricmp(source_data.c_str(), match_data.c_str());
	}

	transform(source_data.begin(), source_data.end(), source_data.begin(), ::tolower);
	transform(match_data.begin(), match_data.end(), match_data.begin(), ::tolower);

	int source_pos = 0;
	int match_pos = 0;

	bool match = true;
	bool b_start = false;

	while (source_pos < source_data.length()) {
		char m = match_data[match_pos];

		if (match_data[match_pos] == '*') {
			b_start = true;
			break;
		}

		if (b_start && source_data[source_pos] == match_data[match_pos]) {
			b_start = false;
			match_pos++;
			continue;
		}

		if (source_data[source_pos] == match_data[match_pos]) {
			source_pos++;
			match_pos++;
			continue;
		}

		match = false;
		break;
	}

	return match;
}

bool CStringHelper::IsMatch(std::wstring source_data, std::wstring match_data) {
	if (nullptr == wcsstr(match_data.c_str(), L"*")) {
		return 0 == _wcsicmp(source_data.c_str(), match_data.c_str());
	}

	transform(source_data.begin(), source_data.end(), source_data.begin(), ::tolower);
	transform(match_data.begin(), match_data.end(), match_data.begin(), ::tolower);

	int source_pos = 0;
	int match_pos = 0;

	bool match = true;
	bool b_start = false;

	while (source_pos < source_data.length()) {
		if (match_data[match_pos] == '*') {
			b_start = true;
			break;
		}

		if (b_start && source_data[source_pos] == match_data[match_pos]) {
			b_start = false;
			match_pos++;
			continue;
		}

		if (source_data[source_pos] == match_data[match_pos]) {
			source_pos++;
			match_pos++;
			continue;
		}

		match = false;
		break;
	}

	return match;
}

bool CStringHelper::IsMatch(const wchar_t* source_data, const char* match_data) {
	return IsMatch(w2a(source_data), match_data);
}

std::string CStringHelper::DeescapeURL(const std::string& URL) {
	std::string result = "";
	for (unsigned int i = 0; i < URL.length(); i++) {
		char c = URL[i];
		if (c != '%') {
			result += c;
		}
		else {
			char c1 = URL[++i];
			char c0 = URL[++i];
			int num = 0;
			num += HexChar2dec(c1) * 16 + HexChar2dec(c0);
			result += char(num);
		}
	}
	return result;
}

std::wstring CStringHelper::DeescapeURL(std::wstring_view URL) {
	std::wstring result = L"";
	for (unsigned int i = 0; i < URL.length(); i++) {
		char c = URL[i];
		if (c != '%') {
			result += c;
		}
		else {
			char c1 = URL[++i];
			char c0 = URL[++i];
			int num = 0;
			num += HexChar2dec(c1) * 16 + HexChar2dec(c0);
			result += char(num);
		}
	}
	return result;
}

short int CStringHelper::HexChar2dec(char c) {
	if ('0' <= c && c <= '9') {
		return short(c - '0');
	}

	if ('a' <= c && c <= 'f') {
		return (short(c - 'a') + 10);
	}

	if ('A' <= c && c <= 'F') {
		return (short(c - 'A') + 10);
	}

	return -1;
}
