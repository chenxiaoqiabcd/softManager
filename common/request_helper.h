#pragma once

#include <string>

enum class ReportType
{
	// �����
	TYPE_OPEN,

	// �������
	TYPE_UPDATE,
};

class RequestHelper
{
public:
	static bool CheckUpdate(const char* request_body, std::string* response_body);

	static bool CheckUpdate(std::string* response_body);

	static bool Report(ReportType type, const wchar_t* comment);

	static std::string GetHost();
private:
	inline static char* ip_ = "39.100.95.114";
	// inline static char* ip_ = "localhost";
};