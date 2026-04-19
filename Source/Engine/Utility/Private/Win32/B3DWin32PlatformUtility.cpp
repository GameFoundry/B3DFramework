//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "Private/Win32/B3DWin32PlatformUtility.h"
#include "Image/B3DColor.h"
#include <windows.h>
#include <iphlpapi.h>
#include <VersionHelpers.h>
#include <intrin.h>
#include "String/B3DUnicode.h"

using namespace b3d;

GPUInfo PlatformUtility::sGPUInfo;

void PlatformUtility::Terminate(bool force)
{
	if(!force)
		PostQuitMessage(0);
	else
		TerminateProcess(GetCurrentProcess(), 0);
}

typedef LONG NTSTATUS, *PNTSTATUS;
typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

RTL_OSVERSIONINFOW GetRealOSVersion()
{
	HMODULE handle = GetModuleHandleW(L"ntdll.dll");
	if(handle)
	{
		RtlGetVersionPtr rtlGetVersionFunc = (RtlGetVersionPtr)GetProcAddress(handle, "RtlGetVersion");
		if(rtlGetVersionFunc != nullptr)
		{
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			if(rtlGetVersionFunc(&rovi) == 0)
				return rovi;
		}
	}

	RTL_OSVERSIONINFOW rovi = { 0 };
	return rovi;
}

SystemInfo PlatformUtility::GetSystemInfo()
{
	SystemInfo output;

	i32 CPUInfo[4] = { -1 };

	// Get CPU manufacturer
	__cpuid(CPUInfo, 0);
	output.CpuManufacturer = String(12, ' ');
	memcpy((char*)output.CpuManufacturer.data(), &CPUInfo[1], 4);
	memcpy((char*)output.CpuManufacturer.data() + 4, &CPUInfo[3], 4);
	memcpy((char*)output.CpuManufacturer.data() + 8, &CPUInfo[2], 4);

	// Get CPU brand string
	char brandString[48];

	//// Get the information associated with each extended ID.
	__cpuid(CPUInfo, 0x80000000);
	u32 extensionIdCount = CPUInfo[0];
	for(u32 extensionIdIndex = 0x80000000; extensionIdIndex <= extensionIdCount; ++extensionIdIndex)
	{
		__cpuid(CPUInfo, extensionIdIndex);

		if(extensionIdIndex == 0x80000002)
			memcpy(brandString, CPUInfo, sizeof(CPUInfo));
		else if(extensionIdIndex == 0x80000003)
			memcpy(brandString + 16, CPUInfo, sizeof(CPUInfo));
		else if(extensionIdIndex == 0x80000004)
			memcpy(brandString + 32, CPUInfo, sizeof(CPUInfo));
	}

	output.CpuModel = brandString;

	// Get number of CPU cores
	SYSTEM_INFO sysInfo;
	::GetSystemInfo(&sysInfo);
	output.CpuNumCores = (u32)sysInfo.dwNumberOfProcessors;

	// Get CPU clock speed
	HKEY hKey;

	long status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey);

	if(status == ERROR_SUCCESS)
	{
		DWORD mhz;
		DWORD bufferSize = 4;
		RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&mhz, &bufferSize);

		output.CpuClockSpeedMhz = (u32)mhz;
	}
	else
		output.CpuClockSpeedMhz = 0;

	// Get amount of system memory
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	output.MemoryAmountMb = (u32)(statex.ullTotalPhys / (1024 * 1024));

	if(B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64)
		output.OsIs64Bit = true;
	else
	{
		HANDLE process = GetCurrentProcess();
		BOOL is64Bit = false;
		IsWow64Process(process, (PBOOL)&is64Bit);

		output.OsIs64Bit = is64Bit > 0;
	}

	// Get OS version
	output.OsName = "Windows " + ToString((u32)GetRealOSVersion().dwMajorVersion);

	// Get GPU info
	output.GpuInfo = sGPUInfo;

	return output;
}

b3d::UUID PlatformUtility::GenerateUuid()
{
	::UUID uuid;
	UuidCreate(&uuid);

	// Endianess might not be correct, but it shouldn't matter
	u32 data1 = uuid.Data1;
	u32 data2 = uuid.Data2 | (uuid.Data3 << 16);
	u32 data3 = uuid.Data3 | (uuid.Data4[0] << 16) | (uuid.Data4[1] << 24);
	u32 data4 = uuid.Data4[2] | (uuid.Data4[3] << 8) | (uuid.Data4[4] << 16) | (uuid.Data4[5] << 24);

	return UUID(data1, data2, data3, data4);
}

String PlatformUtility::ConvertCaseUtF8(const String& input, bool toUpper)
{
	if(input.empty())
		return "";

	WString wideString = UTF8::ToWide(input);

	DWORD flags = LCMAP_LINGUISTIC_CASING;
	flags |= toUpper ? LCMAP_UPPERCASE : LCMAP_LOWERCASE;

	u32 requiredNumChars = LCMapStringEx(
		LOCALE_NAME_USER_DEFAULT,
		flags,
		wideString.data(),
		(int)wideString.length(),
		nullptr,
		0,
		nullptr,
		nullptr,
		0);

	WString outputWideString(requiredNumChars, ' ');

	LCMapStringEx(
		LOCALE_NAME_USER_DEFAULT,
		flags,
		wideString.data(),
		(int)wideString.length(),
		&outputWideString[0],
		(int)outputWideString.length(),
		nullptr,
		nullptr,
		0);

	return UTF8::FromWide(outputWideString);
}

HBITMAP Win32PlatformUtility::CreateBitmap(const Color* pixels, u32 width, u32 height, bool premultiplyAlpha)
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

	// Select the bitmaps to DC
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBitmapDC, hBitmap);

	// Scan each pixel of the source bitmap and create the masks
	Color pixel;
	DWORD* destination = (DWORD*)data;
	for(u32 rowIndex = 0; rowIndex < height; ++rowIndex)
	{
		for(u32 columnIndex = 0; columnIndex < width; ++columnIndex)
		{
			u32 reversedY = height - rowIndex - 1;
			pixel = pixels[reversedY * width + columnIndex];

			if(premultiplyAlpha)
			{
				pixel.R *= pixel.A;
				pixel.G *= pixel.A;
				pixel.B *= pixel.A;
			}

			*destination = pixel.GetAsRgba();

			destination++;
		}
	}

	SelectObject(hBitmapDC, hOldBitmap);
	DeleteDC(hBitmapDC);

	return hBitmap;
}
