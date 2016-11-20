//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPrerequisitesUtil.h"
#include "Win32/BsWin32PlatformUtility.h"
#include "BsColor.h"
#include <windows.h>
#include <iphlpapi.h>

namespace bs
{
	void PlatformUtility::terminate(bool force)
	{
		if (!force)
			PostQuitMessage(0);
		else
			TerminateProcess(GetCurrentProcess(), 0);
	}

	void PlatformUtility::copyToClipboard(const WString& string)
	{
		HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (string.size() + 1) * sizeof(WString::value_type));
		WString::value_type* buffer = (WString::value_type*)GlobalLock(hData);

		string.copy(buffer, string.size());
		buffer[string.size()] = '\0';

		GlobalUnlock(hData);

		if (OpenClipboard(NULL))
		{
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hData);
			CloseClipboard();
		}
		else
		{
			GlobalFree(hData);
		}
	}

	WString PlatformUtility::copyFromClipboard()
	{
		if (OpenClipboard(NULL))
		{
			HANDLE hData = GetClipboardData(CF_UNICODETEXT);

			if (hData != NULL)
			{
				WString::value_type* buffer = (WString::value_type*)GlobalLock(hData);
				WString string(buffer);
				GlobalUnlock(hData);

				CloseClipboard();
				return string;
			}
			else
			{
				CloseClipboard();
				return L"";
			}
		}

		return L"";
	}

	WString PlatformUtility::keyCodeToUnicode(UINT32 keyCode)
	{
		static HKL keyboardLayout = GetKeyboardLayout(0);
		static UINT8 keyboarState[256];

		if (GetKeyboardState(keyboarState) == FALSE)
			return 0;

		UINT virtualKey = MapVirtualKeyExW(keyCode, 1, keyboardLayout);

		wchar_t output[2];
		int count = ToUnicodeEx(virtualKey, keyCode, keyboarState, output, 2, 0, keyboardLayout);
		if (count > 0)
			return WString(output, count);

		return StringUtil::WBLANK;
	}

	bool PlatformUtility::getMACAddress(MACAddress& address)
	{
		std::memset(&address, 0, sizeof(address));

		PIP_ADAPTER_INFO adapterInfo = bs_alloc<IP_ADAPTER_INFO>();

		ULONG len = sizeof(IP_ADAPTER_INFO);
		DWORD rc = GetAdaptersInfo(adapterInfo, &len);

		if (rc == ERROR_BUFFER_OVERFLOW)
		{
			bs_free(adapterInfo);
			adapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(bs_alloc(len));
		}
		else if (rc != ERROR_SUCCESS)
		{
			bs_free(adapterInfo);
			return false;
		}

		if (GetAdaptersInfo(adapterInfo, &len) == NO_ERROR)
		{
			PIP_ADAPTER_INFO curAdapter = nullptr;
			curAdapter = adapterInfo;

			while (curAdapter)
			{
				if (curAdapter->Type == MIB_IF_TYPE_ETHERNET && curAdapter->AddressLength == sizeof(address))
				{
					std::memcpy(&address, curAdapter->Address, curAdapter->AddressLength);
					return true;
				}

				curAdapter = curAdapter->Next;
			}
		}

		bs_free(adapterInfo);
		return false;
	}

	String PlatformUtility::generateUUID()
	{
		UUID uuid;
		UuidCreate(&uuid);

		UINT8* uuidStr;
		UuidToStringA(&uuid, &uuidStr);

		String output((char*)uuidStr);
		RpcStringFreeA(&uuidStr);

		return output;
	}

	void PlatformUtility::open(const Path& path)
	{
		ShellExecute(nullptr, "open", path.toString().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}

	HBITMAP Win32PlatformUtility::createBitmap(const Color* pixels, UINT32 width, UINT32 height, bool premultiplyAlpha)
	{
		BITMAPINFO bi;

		ZeroMemory(&bi, sizeof(BITMAPINFO));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = width;
		bi.bmiHeader.biHeight = height;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;

		HDC hDC = GetDC(nullptr);

		void* data = nullptr;
		HBITMAP hBitmap = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, (void**)&data, nullptr, 0);

		HDC hBitmapDC = CreateCompatibleDC(hDC);
		ReleaseDC(nullptr, hDC);

		//Select the bitmaps to DC
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBitmapDC, hBitmap);

		//Scan each pixel of the source bitmap and create the masks
		Color pixel;
		DWORD *dst = (DWORD*)data;
		for (UINT32 y = 0; y < height; ++y)
		{
			for (UINT32 x = 0; x < width; ++x)
			{
				UINT32 revY = height - y - 1;
				pixel = pixels[revY * width + x];

				if (premultiplyAlpha)
				{
					pixel.r *= pixel.a;
					pixel.g *= pixel.a;
					pixel.b *= pixel.a;
				}

				*dst = pixel.getAsBGRA();

				dst++;
			}
		}

		SelectObject(hBitmapDC, hOldBitmap);
		DeleteDC(hBitmapDC);

		return hBitmap;
	}
}