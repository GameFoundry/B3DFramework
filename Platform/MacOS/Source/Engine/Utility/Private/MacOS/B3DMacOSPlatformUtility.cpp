//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DPlatformUtility.h"
#include <uuid/uuid.h>
#include <sys/sysctl.h>
#include <CoreFoundation/CoreFoundation.h>

using namespace b3d;

GPUInfo PlatformUtility::sGPUInfo;

void PlatformUtility::Terminate(bool force)
{
	// TODOPORT - Support clean exit by sending the main window a quit message
	exit(0);
}

void PlatformUtility::DisableInteractiveErrorDialogs()
{
	// No interactive error dialogs on macOS - asserts and aborts already go to stderr
}

SystemInfo PlatformUtility::GetSystemInfo()
{
	char buffer[256];

	SystemInfo output;

	size_t bufferLen = sizeof(buffer);
	if(sysctlbyname("machdep.cpu.vendor", buffer, &bufferLen, nullptr, 0) == 0)
		output.CpuManufacturer = buffer;
	else
	{
		// Not available on Apple Silicon
		output.CpuManufacturer = "Apple";
	}

	bufferLen = sizeof(buffer);
	if(sysctlbyname("machdep.cpu.brand_string", buffer, &bufferLen, nullptr, 0) == 0)
		output.CpuModel = buffer;

	bufferLen = sizeof(buffer);
	if(sysctlbyname("kern.osrelease", buffer, &bufferLen, nullptr, 0) == 0)
		output.OsName = "macOS " + String(buffer);

	output.OsIs64Bit =
		B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_ARM64;

	// Not available on Apple Silicon
	u64 frequencyHz = 0;
	size_t frequencyLen = sizeof(frequencyHz);
	if(sysctlbyname("hw.cpufrequency", &frequencyHz, &frequencyLen, nullptr, 0) == 0)
		output.CpuClockSpeedMhz = (u32)(frequencyHz / (1000 * 1000));
	else
		output.CpuClockSpeedMhz = 0;

	i32 physicalCoreCount = 0;
	size_t coreCountLen = sizeof(physicalCoreCount);
	if(sysctlbyname("hw.physicalcpu", &physicalCoreCount, &coreCountLen, nullptr, 0) == 0)
		output.CpuNumCores = (u32)physicalCoreCount;
	else
		output.CpuNumCores = 0;

	u64 memoryAmountBytes = 0;
	size_t memoryAmountLen = sizeof(memoryAmountBytes);
	if(sysctlbyname("hw.memsize", &memoryAmountBytes, &memoryAmountLen, nullptr, 0) == 0)
		output.MemoryAmountMb = (u32)(memoryAmountBytes / (1024 * 1024));
	else
		output.MemoryAmountMb = 0;

	output.GpuInfo = sGPUInfo;
	return output;
}

UUID PlatformUtility::GenerateUuid()
{
	uuid_t nativeUUID;
	uuid_generate(nativeUUID);

	return UUID(
		*(u32*)&nativeUUID[0],
		*(u32*)&nativeUUID[4],
		*(u32*)&nativeUUID[8],
		*(u32*)&nativeUUID[12]);
}

String PlatformUtility::ConvertCaseUtF8(const String& input, bool toUpper)
{
	CFMutableStringRef mutableString = CFStringCreateMutable(nullptr, 0);
	CFStringAppendCString(mutableString, input.c_str(), kCFStringEncodingUTF8);

	if(toUpper)
		CFStringUppercase(mutableString, nullptr);
	else
		CFStringLowercase(mutableString, nullptr);

	const char* chars = CFStringGetCStringPtr(mutableString, kCFStringEncodingUTF8);
	if(chars)
	{
		String convertedString(chars);
		CFRelease(mutableString);

		return convertedString;
	}

	CFIndex maxBufferLength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(mutableString), kCFStringEncodingUTF8) + 1;
	auto buffer = B3DStackAllocate<char>((u32)maxBufferLength);

	CFStringGetCString(mutableString, buffer, maxBufferLength, kCFStringEncodingUTF8);
	CFRelease(mutableString);

	String convertedString(buffer);
	B3DStackFree(buffer);

	return convertedString;
}
