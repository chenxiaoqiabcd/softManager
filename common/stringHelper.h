#pragma once

#include <string>
#include <vector>

using std::string;
using std::wstring;
using std::vector;

class CStringHelper {
public:
	static wstring u2w(const string& utf8Str);

	static std::string w2u(const std::wstring& utf16Str);

	static std::string a2u(const std::string& str);

	static std::string u2a(const std::string& str);

	static std::wstring a2w(const std::string& str);

	static std::string w2a(const std::wstring& w);

	static string Trim(string src, char res);
	
	static vector<string> Split(const string& value, const string& filter);

	static vector<wstring> Split(const wstring& value, const wstring& filter);

	static bool IsMatch(std::string source_data, std::string match_data);

	static bool IsMatch(std::wstring source_data, std::wstring match_data);

	static bool IsMatch(const wchar_t* source_data, const char* match_data);

	static std::string DeescapeURL(const std::string& URL);

	static std::wstring DeescapeURL(std::wstring_view URL);

	static short int HexChar2dec(char c);
};