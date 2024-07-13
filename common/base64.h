#ifndef __BASE64_H__
#define __BASE64_H__

#include <string>

std::string base64_encode(const char* bytes_to_encode, unsigned int in_len);

std::string base64_decode(std::string const& encoded_string);

#endif
