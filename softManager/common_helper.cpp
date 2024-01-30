#include "common_helper.h"

#include <Windows.h>
#include <iostream>
#include <iphlpapi.h>
#include <regex>

#include "kf_str.h"

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Version.lib")

bool CommonHelper::GetAdapterInfoWithWindows(char* local_ip, int max_count) {
	unsigned long u_out_buffer_len = sizeof IP_ADAPTER_INFO;
	PIP_ADAPTER_INFO adapter_info = static_cast<PIP_ADAPTER_INFO>(malloc(u_out_buffer_len));

	if (GetAdaptersInfo(adapter_info, &u_out_buffer_len) != ERROR_SUCCESS) {
		adapter_info = static_cast<PIP_ADAPTER_INFO>(malloc(u_out_buffer_len));
	}

	if (GetAdaptersInfo(adapter_info, &u_out_buffer_len) != ERROR_SUCCESS) {
		std::cout << "GetAdaptersInfo failed" << std::endl;
		free(adapter_info);
		return false;
	}

	while (adapter_info) {
		if (strstr(adapter_info->Description, "PCI") || strstr(adapter_info->Description, "Intel")) {
			strcpy_s(local_ip, max_count, adapter_info->IpAddressList.IpAddress.String);
		}

		adapter_info = adapter_info->Next;
	}

	free(adapter_info);
	return true;
}

std::string CommonHelper::GetVersion(const std::string& strSrc) {
	std::smatch m;
	if (std::regex_search(strSrc, m, std::regex("(\\d+)[.](\\d+)[.](\\d+)[.](\\d+)"))) {
		return m.str();
	}

	if (std::regex_search(strSrc, m, std::regex("(\\d+)[.](\\d+)[.](\\d+)"))) {
		return m.str();
	}

	if (std::regex_search(strSrc, m, std::regex("(\\d+)[.](\\d+)"))) {
		return m.str();
	}

	return "";
}

std::string CommonHelper::GetFileVersion(std::string_view strFile) {
	const auto& nSize = GetFileVersionInfoSizeA(strFile.data(), nullptr);
	if (nSize > 0) {
		std::shared_ptr<char> buffer(new char[nSize]);

		VS_FIXEDFILEINFO* pVsInfo;
		UINT nFileInfoSize = sizeof(VS_FIXEDFILEINFO);

		if (GetFileVersionInfoA(strFile.data(), 0, nSize, buffer.get())) {
			if (VerQueryValueA(buffer.get(), "\\", (void**)&pVsInfo, &nFileInfoSize)) {
				return KfString::Format("%d.%d.%d.%d",
										HIWORD(pVsInfo->dwFileVersionMS),
										LOWORD(pVsInfo->dwFileVersionMS),
										HIWORD(pVsInfo->dwFileVersionLS),
										LOWORD(pVsInfo->dwFileVersionLS)).GetString();
			}
		}
	}

	return "";
}