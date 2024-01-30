#include "kf_str.h"

#include <Windows.h>
#include <cassert>
#include <cstdarg>

#include "kf_log.h"
#include "mp.h"

#if _MSVC_LANG > 201402L
#include <stringapiset.h>
#endif

std::string w2a(const wchar_t* w)
{
	auto l = WideCharToMultiByte(CP_ACP, NULL, w, -1, NULL, NULL, NULL, NULL);
	std::string a;
	a.resize(l - 1);
	l = WideCharToMultiByte(CP_ACP, NULL, w, -1, &a[0], l, NULL, NULL);
	return a;
}

auto u2a(const char* str) {
	std::wstring w;
	auto l = MultiByteToWideChar(CP_UTF8, NULL, str, -1, NULL, NULL);
	w.resize(l - 1);
	l = MultiByteToWideChar(CP_UTF8, NULL, str, -1, &w[0], l);
	l = WideCharToMultiByte(CP_ACP, NULL, w.c_str(), -1, NULL, NULL, NULL, NULL);
	std::string u;
	u.resize(l - 1);
	l = WideCharToMultiByte(CP_ACP, NULL, w.c_str(), -1, &u[0], l, NULL, NULL);
	return u;
}

bool IsUtf8Data(const uint8_t* data, size_t size) {
	bool ansi = true;
	int32_t byte = 0;
	for(size_t n = 0; n < size; ++n) {
		uint8_t ch = *(data + n);
		if((ch & 0x80) != 0x00) {
			ansi = false;
		}

		if(0 == byte) {
			if(ch >= 0x80) {
				if(ch >= 0xFC && ch <= 0xFD) {
					byte = 6;
				}
				else if(ch >= 0xF8) {
					byte = 5;
				}
				else if(ch >= 0xF0) {
					byte = 4;
				}
				else if(ch >= 0xE0) {
					byte = 3;
				}
				else if(ch >= 0xC0) {
					byte = 2;
				}
				else {
					return false;
				}

				byte--;
			}
		}
		else {
			if((ch & 0xC0) != 0x80) {
				return false;
			}

			byte--;
		}
	}

	if(byte > 0 || ansi) {
		return false;
	}

	return true;
}

KfString::KfString() {
	capacity_ = 1;
	// value_ = new char[1];
	value_ = static_cast<char*>(MemoryPool::GetInstance()->Allocate(1));
	value_[0] = '\0';
}

KfString::KfString(const char* value, size_t len) {
	if (IsUtf8Data(reinterpret_cast<const uint8_t*>(value), len)) {
		const auto data = u2a(value);

		capacity_ = len;
		// value_ = new char[capacity_];
		value_ = static_cast<char*>(MemoryPool::GetInstance()->Allocate(capacity_));
		memset(value_, 0, capacity_);
		strcpy(value_, data.c_str());
		return;
	}

	capacity_ = len;
	// value_ = new char[capacity_];
	value_ = static_cast<char*>(MemoryPool::GetInstance()->Allocate(capacity_));
	memset(value_, 0, capacity_);
	strcpy(value_, value);
}

KfString::KfString(const char* value) : KfString(value, strlen(value) + 1) {

}

KfString::KfString(const wchar_t* value) : KfString(w2a(value).c_str()) {
	
}

KfString::KfString(const wchar_t* value, size_t len) : KfString(w2a(value).c_str(), wcslen(value) + 1) {

}

KfString::KfString(const KfString& other) : KfString(other.GetString(), other.GetCapacity()) {

}

KfString::KfString(KfString&& other) noexcept {
	capacity_ = other.capacity_;
	value_ = other.value_;

	other.value_ = nullptr;
	other.capacity_ = 0;
}

KfString::~KfString() {
	if (nullptr != value_) {
		// delete value_;
		MemoryPool::GetInstance()->DeAllocate(value_);
		value_ = nullptr;
	}
}

KfString& KfString::operator=(const char* value) {
	if (nullptr != value_) {
		MemoryPool::GetInstance()->DeAllocate(value_);
		value_ = nullptr;
	}

	capacity_ = strlen(value) + 1;
	value_ = static_cast<char*>(MemoryPool::GetInstance()->Allocate(capacity_));
	ZeroMemory(value_, capacity_);
	strcpy_s(value_, capacity_, value);

	return *this;
}

KfString& KfString::operator=(const wchar_t* value) {
	operator=(w2a(value).c_str());
	return *this;
}

KfString& KfString::operator=(const KfString& other) {
	if(other != *this) {
		operator=(other.GetString());
	}

	return *this;
}

KfString& KfString::operator=(KfString&& other) noexcept {
	if (nullptr != value_) {
		MemoryPool::GetInstance()->DeAllocate(value_);
		value_ = nullptr;
	}

	capacity_ = other.capacity_;
	value_ = other.value_;
	other.capacity_ = 0;
	other.value_ = nullptr;

	return *this;
}

bool KfString::operator==(const char* value) const {
	return 0 == strcmp(value_, value);
}

bool KfString::operator==(const wchar_t* value) const {
	return 0 == strcmp(value_, w2a(value).c_str());
}

bool KfString::operator==(const KfString& value) const {
	return  0 == strcmp(value_, value.GetString());
}

bool KfString::CompareNoCase(const char* value) const {
	return 0 == _stricmp(value_, value);
}

bool KfString::CompareNoCase(const wchar_t* value) const {
	return 0 == _stricmp(value_, w2a(value).c_str());
}

bool KfString::CompareNoCase(const KfString& value) const {
	return 0 == _stricmp(value_, value.GetString());
}

KfString KfString::operator+(const char* value) const {
	KfString result(value_);
	result += value;
	return result;
}

KfString KfString::operator+(const wchar_t* value) const {
	return operator+(w2a(value).c_str());
}

KfString KfString::operator+(const KfString& other) const {
	return operator+(other.GetString());
}

KfString& KfString::operator+=(const char* value) {
	return Append(value);
}

KfString& KfString::operator+=(const wchar_t* value) {
	return Append(w2a(value).c_str());
}

KfString& KfString::operator+=(const KfString& value) {
	return Append(value.GetString());
}

char KfString::operator[](int index) const {
	return value_[index];
}

KfString::operator const char*() const {
	return value_;
}

KfString::operator std::wstring() const {
	return GetWString();
}

KfString& KfString::MakeUpper() {
	int index = 0;

	while(true) {
		char& data = value_[index++];
		if(data == '\0') {
			break;
		}

		if(data >= 'a' && data <= 'z') {
			data -= 32;
		}
	}

	return *this;
}

KfString& KfString::MakeLower() {
	int index = 0;

	while (true) {
		char& data = value_[index++];
		if (data == '\0') {
			break;
		}

		if (data >= 'A' && data <= 'Z') {
			data += 32;
		}
	}

	return *this;
}

KfString& KfString::Trim(char filter) {
	while(value_[0] == filter) {
		auto temp = SubStr(1, -1);
		std::swap(*this, temp);
	}

	while(value_[strlen(value_)-1] == filter) {
		auto temp = SubStr(0, strlen(value_) - 1);
		std::swap(*this, temp);
	}

	return *this;
}

KfString& KfString::AppendFormat(const char* const format, ...) {
	std::unique_ptr<char> value(new char[MAX_PATH]);
	memset(value.get(), 0, MAX_PATH);

	va_list list;
	va_start(list, format);
	vsprintf(value.get(), format, list);
	va_end(list);

	return Append(value.get());
}

KfString& KfString::AppendFormat(const wchar_t* const format, ...) {
	std::unique_ptr<wchar_t> value(new wchar_t[MAX_PATH]);
	memset(value.get(), 0, MAX_PATH);

	va_list list;
	va_start(list, format);
	vswprintf(value.get(), format, list);
	va_end(list);

	return Append(value.get());
}

KfString& KfString::Append(const char* value) {
	CheckAppendCapacity(strlen(value));

	strcat(value_, value);
	return *this;
}

KfString& KfString::Append(const wchar_t* value) {
	KfString temp(value);

	CheckAppendCapacity(temp.GetLength());

	strcat(value_, temp.GetString());
	return *this;
}

KfString& KfString::Append(const KfString& value) {
	return Append(value.GetString());
}

KfString& KfString::Insert(int pos, const char* value) {
	KfString temp(Left(pos) + value + SubStr(pos));
	std::swap(*this, temp);
	return *this;
}

KfString& KfString::Insert(int pos, const wchar_t* value) {
	KfString temp(Left(pos) + value + SubStr(pos));
	std::swap(*this, temp);
	return *this;
}

KfString& KfString::Insert(int pos, const KfString& value) {
	KfString temp(Left(pos) + value + SubStr(pos));
	std::swap(*this, temp);
	return *this;
}

KfString KfString::Format(const char* const format, ...) {
	std::unique_ptr<char> value(new char[MAX_PATH]);
	memset(value.get(), 0, MAX_PATH);

	va_list list;
	va_start(list, format);
	vsprintf(value.get(), format, list);
	va_end(list);

	return KfString(value.get());
}

KfString KfString::Format(const wchar_t* const format, ...) {
	std::unique_ptr<wchar_t> value(new wchar_t[MAX_PATH]);
	memset(value.get(), 0, MAX_PATH);

	va_list list;
	va_start(list, format);
 	vswprintf(value.get(), format, list);
	va_end(list);

	return KfString(value.get());
}

size_t KfString::Find(const char* filter) const {
	const auto filter_len = strlen(filter);

	const auto value_len = strlen(value_);

	if (value_len < filter_len) {
		return std::string::npos;
	}

	if(filter_len == 1) {
		return Find(filter[0]);
	}

	for (size_t n = 0; n < value_len - filter_len + 1; ++n) {
		if (0 == strncmp(value_ + n, filter, filter_len)) {
			return n;
		}
	}

	return std::string::npos;
}

size_t KfString::Find(const wchar_t* filter) const {
	return Find(w2a(filter).c_str());
}

size_t KfString::Find(char filter) const {
	const auto value_len = strlen(value_);
	for (size_t n = 0; n < value_len; ++n) {
		if(value_[n] == filter) {
			return n;
		}
	}

	return std::string::npos;
}

size_t KfString::ReverseFind(const char* filter) const {
	const auto filter_len = strlen(filter);

	if (filter_len == 1) {
		return ReverseFind(filter[0]);
	}

	const auto value_len = strlen(value_);

	for (size_t n = value_len - filter_len; n >= 0; --n) {
		if (0 == strncmp(value_ + n, filter, filter_len)) {
			return n;
		}
	}

	return std::string::npos;
}

size_t KfString::ReverseFind(const wchar_t* filter) const {
	return ReverseFind(w2a(filter).c_str());
}

size_t KfString::ReverseFind(char filter) const {
	assert(strlen(value_) > 0);

	for (int n = strlen(value_) - 1; n >= 0; --n) {
		if (value_[n] == filter) {
			return n;
		}
	}

	return std::string::npos;
}

KfString KfString::SubStr(int start_pos, size_t length) const {
	const auto data_len = strlen(value_);
	assert(start_pos >= 0 && start_pos <= data_len);

	if(start_pos + length > data_len || length == std::string::npos) {
		if(data_len == start_pos) {
			return "";
		}

		std::unique_ptr<char> data(new char[data_len - start_pos + 1]);
		memset(data.get(), 0, data_len - start_pos + 1);

		strcpy(data.get(), value_ + start_pos);

		return KfString(data.get());
	}

	std::unique_ptr<char> data(new char[length + 1]);
	memset(data.get(), 0, length + 1);

	strncpy(data.get(), value_ + start_pos, length);

	return KfString(data.get());
}

KfString KfString::Left(int length) const {
	return SubStr(0, length);
}

KfString KfString::Right(size_t length) const {
	return SubStr(static_cast<int>(GetLength() - length), -1);
}

KfString& KfString::Replace(const char* src, const char* dest) {
	KfString temp, result;
	std::swap(temp, *this);
	capacity_ = 0;

	while(true) {
		const auto index = temp.Find(src);
		if (std::string::npos == index) {
			break;
		}

		result += temp.Left(static_cast<int>(index));
		result += dest;

		temp = temp.SubStr(static_cast<int>(index + strlen(src)));
	}

	if(!temp.IsEmpty()) {
		result += temp;
	}

	std::swap(*this, result);
	return *this;
}

KfString& KfString::Replace(const wchar_t* src, const wchar_t* dest) {
	return Replace(w2a(src).c_str(), w2a(dest).c_str());
}

size_t KfString::GetLength() const {
	return strlen(value_);
}

uint32_t KfString::GetCapacity() const {
	return capacity_;
}

bool KfString::IsEmpty() const {
	return value_ == '\0';
}

const char* KfString::GetString() const {
	return value_;
}

char* KfString::GetData() const {
	return value_;
}

std::string KfString::GetUtf8String() const {
	auto a2u = [](const std::string & str) {
		std::wstring w;
		auto l = MultiByteToWideChar(CP_ACP, NULL, str.c_str(), -1, NULL, NULL);
		w.resize(l - 1);
		l = MultiByteToWideChar(CP_ACP, NULL, str.c_str(), -1, &w[0], l);
		l = WideCharToMultiByte(CP_UTF8, NULL, w.c_str(), -1, NULL, NULL, NULL, NULL);
		std::string u;
		u.resize(l - 1);
		l = WideCharToMultiByte(CP_UTF8, NULL, w.c_str(), -1, &u[0], l, NULL, NULL);
		return u;
	};

	return KfString{ a2u(value_).c_str() }.value_;
}

std::wstring KfString::GetWString() const {
	auto a2w = [](const std::string& str) {
		std::wstring w;
		auto l = MultiByteToWideChar(CP_ACP, NULL, str.c_str(), -1, NULL, NULL);
		w.resize(l - 1);
		l = MultiByteToWideChar(CP_ACP, NULL, str.c_str(), -1, &w[0], l);
		return w;
	};

	return a2w(value_);
}

std::vector<KfString> KfString::Split(const char* filter) const {
	std::vector<KfString> result;

	KfString temp{ value_ };

	char* data = strtok(temp.value_, filter);
	while(data != '\0') {
		result.emplace_back(data);
		data = strtok(nullptr, filter);
	}

	return result;
}

void KfString::CheckAppendCapacity(size_t append_size) {
	const size_t total_len = strlen(value_) + append_size + 1;
	if(total_len > capacity_) {
		Reallocate(total_len);
	}
}

void KfString::Reallocate(size_t size) {
	KfString temp(std::move(*this));

	capacity_ = size;

	// value_ = new char[capacity_];
	value_ = static_cast<char*>(MemoryPool::GetInstance()->Allocate(capacity_));
	ZeroMemory(value_, capacity_);
	strcpy(value_, temp);
}

KfString operator+(const char* value, const KfString& other) {
	KfString result(value);
	result += other;
	return result;
}

std::ostream& operator<<(std::ostream& os, const KfString& other) {
	os << other.GetString();
	return os;
}

void KfStringTest() {
	KF_TIMER("%s", __FUNCTION__);

	KfString hello("hello");

	assert(hello == "hello");
	
	hello.Insert(static_cast<int>(hello.GetLength()), " world");

	assert(hello == "hello world");
	
	assert(hello.MakeUpper() == "HELLO WORLD");
	
	assert(hello.MakeLower() == "hello world");
	
	assert(std::string::npos == hello.ReverseFind('i'));
	
	hello = "12345";

	hello = hello;
	
	assert(hello == "12345");
	
	assert(hello + "6" == "123456");
	
	assert((hello += " ") == "12345 ");
	
	assert(hello[0] == '1');
	
	assert(hello.Trim() == "12345");
	
	assert(hello.AppendFormat("%d", 67) == "1234567");
	
	assert(hello.Append("8") == "12345678");
	
	assert(hello.Insert(0, "0") == "012345678");
	
	assert(hello.Find('7') == 7);
	
	assert(hello.ReverseFind('7') == 7);
	
	assert(hello.Left(3) == "012");

	assert(hello.Replace("3", "e") == "012e45678");
}
