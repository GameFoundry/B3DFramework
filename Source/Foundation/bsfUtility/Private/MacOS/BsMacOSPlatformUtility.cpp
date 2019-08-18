//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsPlatformUtility.h"
#include <uuid/uuid.h>
#include <sys/sysctl.h>

namespace bs
{
	GPUInfo PlatformUtility::sGPUInfo;

	void PlatformUtility::terminate(bool force)
	{
		// TODOPORT - Support clean exit by sending the main window a quit message
		exit(0);
	}

	SystemInfo PlatformUtility::getSystemInfo()
	{
		char buffer[256];

		SystemInfo sysInfo;

		size_t bufferlen = sizeof(buffer);
		if(sysctlbyname("machdep.cpu.vendor", buffer, &bufferlen, nullptr, 0) == 0)
			sysInfo.cpuManufacturer = buffer;

		bufferlen = sizeof(buffer);
		if(sysctlbyname("machdep.cpu.brand_string", buffer, &bufferlen, nullptr, 0) == 0)
			sysInfo.cpuModel = buffer;

		bufferlen = sizeof(buffer);
		if(sysctlbyname("kern.osrelease", buffer, &bufferlen, nullptr, 0) == 0)
			sysInfo.osName = "macOS " + String(buffer);

		bufferlen = sizeof(buffer);
		if(sysctlbyname("kern.version", buffer, &bufferlen, nullptr, 0) == 0)
			sysInfo.osIs64Bit = strstr(buffer, "X86_64") != nullptr;
		else
			sysInfo.osIs64Bit = false;

		bufferlen = sizeof(buffer);
		if(sysctlbyname("hw.cpufrequency", buffer, &bufferlen, nullptr, 0) == 0)
		{
			UINT32 speedHz = *(UINT32*)buffer;
			sysInfo.cpuClockSpeedMhz = speedHz / (1000 * 1000);
		}
		else
			sysInfo.cpuClockSpeedMhz = 0;

		bufferlen = sizeof(buffer);
		if(sysctlbyname("hw.physicalcpu", buffer, &bufferlen, nullptr, 0) == 0)
			sysInfo.cpuNumCores = *(UINT32*)buffer;
		else
			sysInfo.cpuNumCores = 0;

		bufferlen = sizeof(buffer);
		if(sysctlbyname("hw.memsize", buffer, &bufferlen, nullptr, 0) == 0)
		{
			UINT64 memAmountBytes = *(UINT64*)buffer;
			sysInfo.memoryAmountMb = (UINT32)(memAmountBytes / (1024 * 1024));
		}
		else
			sysInfo.memoryAmountMb = 0;

		sysInfo.gpuInfo = sGPUInfo;
		return sysInfo;
	}

	UUID PlatformUtility::generateUUID()
	{
		uuid_t nativeUUID;
		uuid_generate(nativeUUID);

		return UUID(
			*(UINT32*)&nativeUUID[0],
			*(UINT32*)&nativeUUID[4],
			*(UINT32*)&nativeUUID[8],
			*(UINT32*)&nativeUUID[12]);
	}

	String PlatformUtility::convertCaseUTF8(const bs::String& input, bool toUpper)
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
			String output(chars);
			CFRelease(mutableString);

			return output;
		}

		CFIndex stringLength = CFStringGetLength(mutableString) + 1;
		auto buffer = bs_stack_alloc<char>((UINT32)stringLength);

		CFStringGetCString(mutableString, buffer, stringLength, kCFStringEncodingUTF8);
		CFRelease(mutableString);

		String output(buffer);
		bs_stack_free(buffer);

		return output;
	}
}

