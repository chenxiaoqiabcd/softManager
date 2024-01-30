#pragma once
#include <string>

using std::string;

class VersionHelper {
public:
	VersionHelper();

	VersionHelper(const char* version);

	VersionHelper(const wchar_t* version);

	bool Parse(const char* version);

	std::string ToString();

	bool operator < (const VersionHelper& version_helper);

	bool operator > (const VersionHelper& version_helper);

	bool operator >= (const VersionHelper& version_helper);

	bool operator <= (const VersionHelper& version_helper);

	bool operator == (const VersionHelper& version_helper) const;
private:
	int base_code_;
	int alpha_code;
	int beta_code_;
	int rc_code_;
};
