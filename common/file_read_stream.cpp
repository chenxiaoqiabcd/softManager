#include "file_read_stream.h"

FileReadStream::FileReadStream(const char* file_path) {
	in_file_steam_.open(file_path, std::ios::in | std::ios::binary);
}

FileReadStream::~FileReadStream() {
	in_file_steam_.close();
}

bool FileReadStream::IsOpen() const {
	return in_file_steam_.is_open();
}

std::streampos FileReadStream::GetSize() {
	in_file_steam_.seekg(0, std::ios::end);

	auto file_size = in_file_steam_.tellg();

	in_file_steam_.seekg(0, std::ios::beg);

	return file_size;
}

std::string FileReadStream::Read() {
	const size_t size = (size_t)GetSize();

	std::shared_ptr<char> buffer(new char[size]);

	in_file_steam_.read(buffer.get(), size);

	return std::string{ buffer.get(), size };
}