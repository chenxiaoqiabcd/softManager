#include "my_time.h"

#include <Windows.h>
#include <ctime>

MyTime::MyTime() {
	time_ = 0;
}

MyTime::MyTime(__time64_t t) {
	time_ = t;
}

MyTime::MyTime(tm& t) {
	time_ = mktime(&t);
}

std::string MyTime::ToString() const {
	struct tm* p;
	p = localtime(&time_);

	p->tm_year += 1900;
	p->tm_mon += 1;

	char result[64];
	ZeroMemory(result, 64);

	std::ignore = sprintf_s(result, "%d-%02d-%02d %02d:%02d:%02d",
							p->tm_year, p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

	return result;
}

bool MyTime::operator>(tm t) const {
	return time_ > mktime(&t);
}

bool MyTime::operator>(const MyTime& other) const {
	return time_ > other.time_;
}

bool MyTime::operator<(tm t) const {
	return time_ < mktime(&t);
}

bool MyTime::operator<(const MyTime& other) const {
	return time_ < other.time_;
}