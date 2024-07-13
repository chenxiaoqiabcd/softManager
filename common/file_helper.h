#pragma once

#include <string>
#include <vector>

#include <windows.h>

namespace Gdiplus
{
	class Bitmap;
}

class FileHelper {
public:
	static std::wstring GetIconWithExePath(const wchar_t* szFile);

	static std::wstring GetIconWithIcoPath(const wchar_t* path);

	static std::wstring GetIconWithDllPath(const wchar_t* path, int index);

	static std::wstring GetIconWithGuid(const wchar_t* guid);

	static std::wstring GetIconWithExeOrIcoFile(const wchar_t* folder);

	static std::wstring GetFilePath(std::wstring_view cmd_line);

	static std::wstring GetFileVersion(const wchar_t* strFile);

	static bool WriteData(const char* file_path, const char* data);
protected:
	static int GetEncoderClsid(const wchar_t* szFormat, CLSID* pClsid);

	static Gdiplus::Bitmap* GenerateBitmapWithBitmapHandle(HBITMAP hbmColor);

	static Gdiplus::Bitmap* GenerateBitmap(const HICON& icon);

	static bool SaveIconToFile(HICON icon, const wchar_t* szFilePath);

	static HICON LoadIconFile(const wchar_t* icon_path);
};