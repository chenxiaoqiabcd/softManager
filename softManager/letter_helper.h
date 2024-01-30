#pragma once

#include <string>

class LetterHelper {
public:
	static std::string GetLetter(const char* value);

	static std::wstring GetLetter(const wchar_t* value);
protected:
	static std::string FindLetter(int nCode);
};