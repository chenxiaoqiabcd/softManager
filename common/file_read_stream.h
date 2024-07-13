#pragma once

#include <fstream>

class FileReadStream
{
public:
	FileReadStream(const char* file_path);

	~FileReadStream();

	bool IsOpen() const;

	std::streampos GetSize();

	std::string Read();
private:
	std::ifstream in_file_steam_;
};
