//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/B3DPrerequisitesUtil.h"
#include "Utility/B3DPlatformUtility.h"
#include <stdlib.h>
#include <uuid/uuid.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <limits>

#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <unicode/udata.h>
#include <unicode/uversion.h>

using namespace b3d;

// TODO: Verify this stub is still required by the ICU version currently linked; re-evaluate once the Linux target builds again.
/** Define a stub 'entry-point' required by ICU. **/
typedef struct
{
	uint16_t headerSize;
	uint8_t magic1, magic2;
	UDataInfo info;
	char padding[8];
	uint32_t count, reserved;
	int fakeNameAndData[4];
} ICU_Data_Header;

extern "C" U_EXPORT const ICU_Data_Header U_ICUDATA_ENTRY_POINT = {
	32, /* headerSize */
	0xda, /* magic1,  (see struct MappedData in udata.c)  */
	0x27, /* magic2     */
	{
		/*UDataInfo   */
		sizeof(UDataInfo), /* size        */
		0, /* reserved    */

#if U_IS_BIG_ENDIAN
		1,
#else
		0,
#endif

		U_CHARSET_FAMILY,
		sizeof(UChar),
		0, /* reserved      */
		{ /* data format identifier */
		  0x54, 0x6f, 0x43, 0x50 }, /* "ToCP" */
		{ 1, 0, 0, 0 }, /* format version major, minor, milli, micro */
		{ 0, 0, 0, 0 } /* dataVersion   */
	},
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, /* Padding[8]   */
	0, /* count        */
	0, /* Reserved     */
	{
		/*  TOC structure */
		/*        {    */
		0, 0, 0, 0 /* name and data entries.  Count says there are none,  */
		/*  but put one in just in case.                       */
		/*        }  */
	}
};

GPUInfo PlatformUtility::sGPUInfo;

void PlatformUtility::Terminate(bool force)
{
	// TODOPORT - Support clean exit by sending the main window a quit message
	exit(0);
}

SystemInfo PlatformUtility::GetSystemInfo()
{
	SystemInfo output;

	output.CpuClockSpeedMhz = 0;
	output.CpuNumCores = 0;
	output.MemoryAmountMb = 0;
	output.OsIs64Bit = false;

	// Get CPU vendor, model and number of cores
	{
		std::ifstream file("/proc/cpuinfo");
		std::string line;
		while(std::getline(file, line))
		{
			std::stringstream lineStream(line);
			std::string token;
			lineStream >> token;

			if(token == "vendor_id")
			{
				if(lineStream >> token && token == ":")
				{
					std::string vendorId;
					if(lineStream >> vendorId)
						output.CpuManufacturer = vendorId.c_str();
				}
			}
			else if(token == "model")
			{
				if(lineStream >> token && token == "name")
				{
					if(lineStream >> token && token == ":")
					{
						std::stringstream modelName;
						if(lineStream >> token)
						{
							modelName << token;

							while(lineStream >> token)
								modelName << " " << token;
						}

						output.CpuModel = modelName.str().c_str();
					}
				}
			}
		}
	}

	// Number of logical processors
	const long logicalCoreCount = sysconf(_SC_NPROCESSORS_ONLN);
	output.CpuNumCores = logicalCoreCount > 0 ? (u32)logicalCoreCount : 0;

	// Get CPU frequency
	{
		std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
		u32 frequency;
		if(file >> frequency)
			output.CpuClockSpeedMhz = frequency / 1000;
	}

	// Get amount of system memory
	{
		std::ifstream file("/proc/meminfo");
		std::string token;
		while(file >> token)
		{
			if(token == "MemTotal:")
			{
				u32 memTotal;
				if(file >> memTotal)
					output.MemoryAmountMb = memTotal / 1024;
				else
					output.MemoryAmountMb = 0;

				break;
			}

			// Ignore the rest of the line
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	// Get OS version
	utsname osInfo;
	uname(&osInfo);

	// Note: This won't report the exact distro
	output.OsName = String(osInfo.sysname) + String(osInfo.version);

	if(B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64)
		output.OsIs64Bit = true;
	else
		output.OsIs64Bit = strstr(osInfo.machine, "64") != nullptr;

	// Get GPU info
	output.GpuInfo = sGPUInfo;

	return output;
}

String PlatformUtility::ConvertCaseUtF8(const String& input, bool toUpper)
{
	UErrorCode errorCode = U_ZERO_ERROR;

	auto inputLen = (int32_t)input.size();
	int32_t bufferLen = 0;
	u_strFromUTF8(nullptr, 0, &bufferLen, input.data(), inputLen, &errorCode);

	auto uStr = B3DStackAllocate<UChar>((u32)bufferLen);
	int32_t uStrLen = 0;
	errorCode = U_ZERO_ERROR;
	// destCapacity is measured in UChars, not bytes
	u_strFromUTF8(uStr, bufferLen, &uStrLen, input.data(), inputLen, &errorCode);

	errorCode = U_ZERO_ERROR;
	if(toUpper)
		bufferLen = u_strToUpper(nullptr, 0, uStr, uStrLen, nullptr, &errorCode);
	else
		bufferLen = u_strToLower(nullptr, 0, uStr, uStrLen, nullptr, &errorCode);

	auto convertedUStr = B3DStackAllocate<UChar>((u32)bufferLen);
	int32_t convertedUStrLen = 0;

	errorCode = U_ZERO_ERROR;
	if(toUpper)
		convertedUStrLen = u_strToUpper(convertedUStr, bufferLen, uStr, uStrLen, nullptr, &errorCode);
	else
		convertedUStrLen = u_strToLower(convertedUStr, bufferLen, uStr, uStrLen, nullptr, &errorCode);

	errorCode = U_ZERO_ERROR;
	u_strToUTF8(nullptr, 0, &bufferLen, convertedUStr, convertedUStrLen, &errorCode);

	int32_t outputStrLen = 0;
	auto outputStr = B3DStackAllocate<char>(bufferLen);

	errorCode = U_ZERO_ERROR;
	u_strToUTF8(outputStr, bufferLen, &outputStrLen, convertedUStr, convertedUStrLen, &errorCode);

	String output(outputStr, outputStrLen);

	B3DStackFree(outputStr);
	B3DStackFree(convertedUStr);
	B3DStackFree(uStr);

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
