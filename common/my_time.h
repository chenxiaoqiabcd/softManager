#pragma once

#include <string>

class MyTime {
public:
	MyTime();

	MyTime(__time64_t t);

	MyTime(tm& t);

	std::string ToString() const;

	bool operator>(tm t) const;

	bool operator>(const MyTime& other) const;

	bool operator<(tm t) const;

	bool operator<(const MyTime& other) const;
private:
	time_t time_;
};