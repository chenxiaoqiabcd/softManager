#include "versionHelper.h"

#include "common_helper.h"
#include "kf_str.h"
#include "stringHelper.h"

VersionHelper::VersionHelper()
	: base_code_(0), alpha_code(0), beta_code_(0), rc_code_(0) {

}

VersionHelper::VersionHelper(const char* version) : VersionHelper() {
	Parse(version);
}

VersionHelper::VersionHelper(const wchar_t* version) : VersionHelper() {
	Parse(CStringHelper::w2a(version).c_str());
}

bool VersionHelper::Parse(const char* version) {
	std::string version_temp = CommonHelper::GetVersion(version);

	char* version_temp_data;
	base_code_ = strtol(version_temp.c_str(), &version_temp_data, 10);

	if (nullptr == version_temp_data) {
		return false;
	}

	alpha_code = strtol(version_temp_data + 1, &version_temp_data, 10);

	if (nullptr == version_temp_data) {
		return true;
	}

	beta_code_ = strtol(version_temp_data + 1, &version_temp_data, 10);

	if (nullptr == version_temp_data) {
		return true;
	}

	rc_code_ = strtol(version_temp_data + 1, &version_temp_data, 10);

	return true;
}

std::string VersionHelper::ToString() const {
	return KfString::Format("%d.%d.%d.%d",
							base_code_, alpha_code, beta_code_,
							rc_code_).GetString();
}

bool VersionHelper::operator<(const VersionHelper& version_helper) const {
	if(base_code_ > version_helper.base_code_) {
		return false;
	}

	if(base_code_ == version_helper.base_code_) {
		if(alpha_code > version_helper.alpha_code) {
			return false;
		}

		if(alpha_code == version_helper.alpha_code) {
			if(beta_code_ > version_helper.beta_code_) {
				return false;
			}

			if(beta_code_ == version_helper.beta_code_) {
				return rc_code_ < version_helper.rc_code_;
			}

			return true;
		}

		return true;
	}

	return true;
}

bool VersionHelper::operator>(const VersionHelper& version_helper) const {
	if (base_code_ < version_helper.base_code_) {
		return false;
	}

	if (base_code_ == version_helper.base_code_) {
		if (alpha_code < version_helper.alpha_code) {
			return false;
		}

		if (alpha_code == version_helper.alpha_code) {
			if (beta_code_ < version_helper.beta_code_) {
				return false;
			}

			if (beta_code_ == version_helper.beta_code_) {
				return rc_code_ > version_helper.rc_code_;
			}

			return true;
		}

		return true;
	}

	return true;
}

bool VersionHelper::operator>=(const VersionHelper& version_helper) {
	return *this > version_helper || *this == version_helper;
}

bool VersionHelper::operator<=(const VersionHelper& version_helper) {
	return *this < version_helper || *this == version_helper;
}

bool VersionHelper::operator==(const VersionHelper& version_helper) const {
	return base_code_ == version_helper.base_code_ &&
		alpha_code == version_helper.alpha_code &&
		beta_code_ == version_helper.beta_code_ &&
		rc_code_ == version_helper.rc_code_;
}
