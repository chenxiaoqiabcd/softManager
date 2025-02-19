#include "file_helper.h"

#include <filesystem>
#include <gdiplus.h>
#include <shlobj_core.h>
#include <Shlwapi.h>

#include "helper.h"
#include "kf_str.h"
#include "md_5.h"
#include "stringHelper.h"

#pragma comment(lib, "Version.lib")

std::wstring FileHelper::GetIconWithExePath(const wchar_t* szFile) {
	const HICON hIcon = ::ExtractIcon(GetModuleHandle(nullptr), szFile, 0);
	if (hIcon == nullptr) {
		return L"";
	}

	KfString file_name(KF::MD5(CStringHelper::w2a(szFile)).toString().c_str());
	file_name.Append(".png");

	const auto temp_path = Helper::GetCacheFile(file_name.GetWString().c_str());

	return SaveIconToFile(hIcon, temp_path.c_str());
}

std::wstring FileHelper::GetIconWithIcoPath(const wchar_t* path) {
	const auto icon = LoadIconFile(path);
	if (nullptr == icon) {
		return L"";
	}

	KfString file_name(KF::MD5(CStringHelper::w2a(path)).toString().c_str());
	file_name.Append(".png");

	auto bitmap_path = Helper::GetCacheFile(file_name.GetWString().c_str());

	if (!DeleteFile(bitmap_path.c_str())) {
		return bitmap_path;
	}

	return SaveIconToFile(icon, bitmap_path.c_str());
}

std::wstring FileHelper::GetIconWithDllPath(const wchar_t* path, int index) {
	// 判断后缀名是否为dll
	auto extension = PathFindExtension(path);

	if (0 != _wcsicmp(extension, L".dll") && 0 != _wcsicmp(extension, L".exe")) {
		return L"";
	}

	HICON hIcon = ExtractIcon(GetModuleHandle(nullptr), path, index);
	if (hIcon != nullptr) {
		KfString file_name(KF::MD5(CStringHelper::w2a(path)).toString().c_str());
		file_name.Append(".png");

		const auto temp_path = Helper::GetCacheFile(file_name.GetWString().c_str());

		return SaveIconToFile(hIcon, temp_path.c_str());
	}

	return L"";
}

std::wstring FileHelper::GetIconWithGuid(const wchar_t* guid) {
	wchar_t install_dir[MAX_PATH];
	GetWindowsDirectory(install_dir, MAX_PATH);
	PathAppend(install_dir, L"installer");
	PathAppend(install_dir, guid);

	auto result = GetIconWithExeOrIcoFile(install_dir);

	if (!result.empty()) {
		return result;
	}

	wchar_t program_files_folder[MAX_PATH];
	ZeroMemory(program_files_folder, MAX_PATH * sizeof(wchar_t));
	ExpandEnvironmentStrings(L"%programFiles(x86)%", program_files_folder, MAX_PATH);
	PathAppend(program_files_folder, L"InstallShield Installation Information");
	PathAppend(program_files_folder, guid);

	result = GetIconWithExeOrIcoFile(program_files_folder);

	if(!result.empty()) {
		return result;
	}

	wchar_t cache_folder[MAX_PATH];
	ZeroMemory(cache_folder, sizeof(cache_folder));
	SHGetSpecialFolderPath(nullptr, cache_folder, CSIDL_APPDATA, FALSE);
	PathAppend(cache_folder, L"Microsoft\\Installer");
	PathAppend(cache_folder, guid);

	return GetIconWithExeOrIcoFile(cache_folder);
}

std::wstring FileHelper::GetIconWithExeOrIcoFile(const wchar_t* folder) {
	if (!std::filesystem::is_directory(folder)) {
		return L"";
	}

	std::wstring result;

	for (const auto& it : std::filesystem::directory_iterator(folder)) {
		if (it.is_directory()) {
			continue;
		}

		auto extension = it.path().extension();

		if (0 == _wcsicmp(extension.wstring().c_str(), L".exe")) {
			result = GetIconWithExePath(it.path().c_str());
			if (!result.empty()) {
				break;
			}
		}

		if (0 == _wcsicmp(extension.wstring().c_str(), L".ico") || !it.path().has_extension()) {
			result = GetIconWithIcoPath(it.path().c_str());
			if (!result.empty()) {
				break;
			}
		}
	}

	return result;
}

std::wstring FileHelper::GetFilePath(std::wstring_view cmd_line) {
	// 如果第一个字符串是双引号，则文件名取第一个双引号到第二个双引号之间的内容
	// 如果第一个字符不是双引号，则文件名取第一个空格之前的内容
	if (cmd_line.empty()) {
		return L"";
	}

	std::wstring file_path;

	if (cmd_line[0] == '\"') {
		file_path = cmd_line.substr(1, cmd_line.find('\"', 1) - 1);
	}
	else if (PathFileExists(cmd_line.data())) {
		file_path = cmd_line;
	}
	else {
		const auto index = cmd_line.find(' ');
		if (std::string_view::npos == index) {
			return cmd_line.data();
		}

		const auto file_path_temp = cmd_line.substr(0, index);
		if (PathFileExists(file_path_temp.data())) {
			return file_path_temp.data();
		}
	}

	return file_path;
}

std::wstring FileHelper::GetFileVersion(const wchar_t* strFile) {
	std::wstring strResultVersion;

	const auto& nSize = GetFileVersionInfoSize(strFile, nullptr);
	if (nSize <= 0) {
		return strResultVersion;
	}

	std::shared_ptr<wchar_t> szBuff(new wchar_t[nSize]);

	VS_FIXEDFILEINFO* pVsInfo;
	UINT nFileInfoSize = sizeof(VS_FIXEDFILEINFO);

	if (GetFileVersionInfo(strFile, 0, nSize, szBuff.get())) {
		if (VerQueryValue(szBuff.get(), L"\\", (LPVOID*)&pVsInfo, &nFileInfoSize)) {
			wsprintf(szBuff.get(), L"%d.%d.%d.%d", HIWORD(pVsInfo->dwFileVersionMS),
					 LOWORD(pVsInfo->dwFileVersionMS), HIWORD(pVsInfo->dwFileVersionLS),
					 LOWORD(pVsInfo->dwFileVersionLS));
			strResultVersion = szBuff.get();
		}
	}

	return strResultVersion;
}

bool FileHelper::WriteData(const char* file_path, const char* data) {
	FILE* f;
	std::ignore = fopen_s(&f, file_path, "wb");
	if (f == nullptr) {
		return false;
	}
	std::ignore = fwrite(data, strlen(data), 1, f);
	std::ignore = fclose(f);
	return true;
}

int FileHelper::GetEncoderClsid(const wchar_t* szFormat, CLSID* pClsid) {
	unsigned int num = 0;
	unsigned int size = 0;
	Gdiplus::GetImageEncodersSize(&num, &size);
	if (0 == size) {
		return -1;
	}

	const auto pImageCodeInfo = static_cast<Gdiplus::ImageCodecInfo*>(malloc(size));
	GetImageEncoders(num, size, pImageCodeInfo);

	bool bFound = false;

	for (unsigned int nx = 0; nx < num; ++nx) {
		if (0 == wcscmp(pImageCodeInfo[nx].MimeType, szFormat)) {
			*pClsid = pImageCodeInfo[nx].Clsid;
			bFound = true;
			break;
		}
	}

	free(pImageCodeInfo);
	return bFound;
}

Gdiplus::Bitmap* FileHelper::GenerateBitmapWithBitmapHandle(HBITMAP hbmColor) {
	const auto wrap_bitmap = Gdiplus::Bitmap::FromHBITMAP(hbmColor, nullptr);
	if (nullptr == wrap_bitmap) {
		return nullptr;
	}

	const Gdiplus::Rect rc_image{
		0,
		0,
		static_cast<int>(wrap_bitmap->GetWidth()),
		static_cast<int>(wrap_bitmap->GetHeight())
	};

	Gdiplus::BitmapData bitmap_data;

	wrap_bitmap->LockBits(&rc_image, Gdiplus::ImageLockModeRead,
						  wrap_bitmap->GetPixelFormat(), &bitmap_data);
	const auto result = new Gdiplus::Bitmap(static_cast<int>(bitmap_data.Width),
											static_cast<int>(bitmap_data.Height), bitmap_data.Stride,
											PixelFormat32bppARGB, static_cast<BYTE*>(bitmap_data.Scan0));
	wrap_bitmap->UnlockBits(&bitmap_data);

	return result;
}

Gdiplus::Bitmap* FileHelper::GenerateBitmap(const HICON& icon) {
	ICONINFO icon_info;
	if (!GetIconInfo(icon, &icon_info)) {
		return nullptr;
	}

	BITMAP bitmap;
	GetObject(icon_info.hbmColor, sizeof(BITMAP), &bitmap);

	Gdiplus::Bitmap* ptr_bitmap;

	if (bitmap.bmBitsPixel != 32) {
		ptr_bitmap = Gdiplus::Bitmap::FromHICON(icon);
	}
	else {
		ptr_bitmap = GenerateBitmapWithBitmapHandle(icon_info.hbmColor);
	}

	DeleteObject(icon_info.hbmColor);
	DeleteObject(icon_info.hbmMask);

	return ptr_bitmap;
}

std::wstring FileHelper::SaveIconToFile(HICON icon, const wchar_t* szFilePath) {
	if (icon == nullptr) {
		return L"";
	}

	auto bmp = GenerateBitmap(icon);
	if (nullptr == bmp) {
		return L"";
	}

	CLSID encoderCLSID;
	GetEncoderClsid(L"image/png", &encoderCLSID);
	const Gdiplus::Status st = bmp->Save(szFilePath, &encoderCLSID, nullptr);

	delete bmp;
	bmp = nullptr;

	return st == Gdiplus::Ok ? szFilePath : L"";
}

HICON FileHelper::LoadIconFile(const wchar_t* icon_path)
{
	if (icon_path == nullptr) {
		return nullptr;
	}

	const auto hIcon = static_cast<HICON>(LoadImage(nullptr, icon_path, IMAGE_ICON, 96, 96, LR_LOADFROMFILE));
	if (hIcon != nullptr) {
		return hIcon;
	}

	const auto hBitmap = static_cast<HBITMAP>(LoadImage(nullptr, icon_path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	if (hBitmap == nullptr) {
		return nullptr;
	}

	ICONINFO icon_info;
	icon_info.fIcon = TRUE;
	icon_info.hbmColor = hBitmap;
	icon_info.hbmMask = nullptr;

	const auto hIconTemp = CreateIconIndirect(&icon_info);
	DeleteObject(hBitmap);

	return hIconTemp;
}