//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Win32/BsWin32PlatformUtility.h"
#include "Image/BsColor.h"
#include <windows.h>
#include <iphlpapi.h>
#include <VersionHelpers.h>
#include <intrin.h>

namespace bs
{
	GPUInfo PlatformUtility::sGPUInfo;

	void PlatformUtility::terminate(bool force)
	{
		if (!force)
			PostQuitMessage(0);
		else
			TerminateProcess(GetCurrentProcess(), 0);
	}

	typedef LONG NTSTATUS, *PNTSTATUS;
	typedef NTSTATUS (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

	RTL_OSVERSIONINFOW GetRealOSVersion() 
	{
		HMODULE handle = GetModuleHandleW(L"ntdll.dll");
		if (handle) 
		{
			RtlGetVersionPtr rtlGetVersionFunc = (RtlGetVersionPtr)GetProcAddress(handle, "RtlGetVersion");
			if (rtlGetVersionFunc != nullptr) 
			{
				RTL_OSVERSIONINFOW rovi = { 0 };
				rovi.dwOSVersionInfoSize = sizeof(rovi);
				if (rtlGetVersionFunc(&rovi) == 0)
					return rovi;
			}
		}

		RTL_OSVERSIONINFOW rovi = { 0 };
		return rovi;
	}

	SystemInfo PlatformUtility::getSystemInfo()
	{
		SystemInfo output;

		INT32 CPUInfo[4] = { -1 };

		// Get CPU manufacturer
		__cpuid(CPUInfo, 0);
		output.cpuManufacturer = String(12, ' ');
		memcpy((char*)output.cpuManufacturer.data(), &CPUInfo[1], 4);
		memcpy((char*)output.cpuManufacturer.data() + 4, &CPUInfo[3], 4);
		memcpy((char*)output.cpuManufacturer.data() + 8, &CPUInfo[2], 4);

		// Get CPU brand string
		char brandString[48];

		//// Get the information associated with each extended ID.
		__cpuid(CPUInfo, 0x80000000);
		UINT32 numExtensionIds = CPUInfo[0];
		for (UINT32 i = 0x80000000; i <= numExtensionIds; ++i)
		{
			__cpuid(CPUInfo, i);

			if (i == 0x80000002)
				memcpy(brandString, CPUInfo, sizeof(CPUInfo));
			else if (i == 0x80000003)
				memcpy(brandString + 16, CPUInfo, sizeof(CPUInfo));
			else if (i == 0x80000004)
				memcpy(brandString + 32, CPUInfo, sizeof(CPUInfo));
		}

		output.cpuModel = brandString;

		// Get number of CPU cores
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		output.cpuNumCores = (UINT32)sysInfo.dwNumberOfProcessors;

		// Get CPU clock speed
		HKEY hKey;

		long status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, 
			&hKey);

		if (status == ERROR_SUCCESS)
		{
			DWORD mhz;
			DWORD bufferSize = 4;
			RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE) &mhz, &bufferSize);

			output.cpuClockSpeedMhz = (UINT32)mhz;
		}
		else
			output.cpuClockSpeedMhz = 0;

		// Get amount of system memory
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		GlobalMemoryStatusEx(&statex);

		output.memoryAmountMb = (UINT32)(statex.ullTotalPhys / (1024 * 1024));

		if (BS_ARCH_TYPE == BS_ARCHITECTURE_x86_64)
			output.osIs64Bit = true;
		else
		{
			HANDLE process = GetCurrentProcess();
			BOOL is64Bit = false;
			IsWow64Process(process, (PBOOL)&is64Bit);

			output.osIs64Bit = is64Bit > 0;
		}

		// Get OS version
		output.osName = "Windows " + toString((UINT32)GetRealOSVersion().dwMajorVersion);

		// Get GPU info
		output.gpuInfo = sGPUInfo;

		return output;
	}

	UUID PlatformUtility::generateUUID()
	{
		::UUID uuid;
		UuidCreate(&uuid);

		// Endianess might not be correct, but it shouldn't matter
		UINT32 data1 = uuid.Data1;
		UINT32 data2 = uuid.Data2 | (uuid.Data3 << 16);
		UINT32 data3 = uuid.Data3 | (uuid.Data4[0] << 16) | (uuid.Data4[1] << 24);
		UINT32 data4 = uuid.Data4[2] | (uuid.Data4[3] << 8) | (uuid.Data4[4] << 16) | (uuid.Data4[5] << 24);

		return UUID(data1, data2, data3, data4);
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