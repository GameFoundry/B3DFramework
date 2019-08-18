//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsPlatformUtility.h"
#include <stdlib.h>
#include <uuid/uuid.h>
#include <sys/utsname.h>

#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <unicode/udata.h>
#include <unicode/uversion.h>

namespace bs
{
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
			32,          /* headerSize */
			0xda,        /* magic1,  (see struct MappedData in udata.c)  */
			0x27,        /* magic2     */
			{            /*UDataInfo   */
				sizeof(UDataInfo),      /* size        */
				0,                      /* reserved    */

	#if U_IS_BIG_ENDIAN
				1,
	#else
				0,
	#endif

				U_CHARSET_FAMILY,
				sizeof(UChar),
				0,               /* reserved      */
				{                /* data format identifier */
					0x54, 0x6f, 0x43, 0x50}, /* "ToCP" */
					{1, 0, 0, 0},   /* format version major, minor, milli, micro */
					{0, 0, 0, 0}    /* dataVersion   */
			},
			{0, 0, 0, 0, 0, 0, 0, 0},  /* Padding[8]   */
			0,                  /* count        */
			0,                  /* Reserved     */
			{                   /*  TOC structure */
/*        {    */
					0, 0, 0, 0  /* name and data entries.  Count says there are none,  */
					/*  but put one in just in case.                       */
/*        }  */
			}
	};

	GPUInfo PlatformUtility::sGPUInfo;

	void PlatformUtility::terminate(bool force)
	{
		// TODOPORT - Support clean exit by sending the main window a quit message
		exit(0);
	}

	SystemInfo PlatformUtility::getSystemInfo()
	{
		SystemInfo output;

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
							output.cpuManufacturer = vendorId.c_str();
					}
				}
				else if(token == "model")
				{
					if(lineStream >> token && token == "name")
					{
						if (lineStream >> token && token == ":")
						{
							std::stringstream modelName;
							if (lineStream >> token)
							{
								modelName << token;

								while (lineStream >> token)
									modelName << " " << token;
							}

							output.cpuModel = modelName.str().c_str();
						}
					}
				}
				else if(token == "cpu")
				{
					if(lineStream >> token)
					{
						if (token == "cores")
						{
							if (lineStream >> token && token == ":")
							{
								UINT32 numCores;
								if (lineStream >> numCores)
									output.cpuNumCores = numCores;
							}
						}
					}
				}
			}
		}

		// Get CPU frequency
		{
			std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
			UINT32 frequency;
			if(file >> frequency)
				output.cpuClockSpeedMhz = frequency / 1000;
		}

		// Get amount of system memory
		{
			std::ifstream file("/proc/meminfo");
			std::string token;
			while(file >> token)
			{
				if(token == "MemTotal:")
				{
					UINT32 memTotal;
					if(file >> memTotal)
						output.memoryAmountMb = memTotal / 1024;
					else
						output.memoryAmountMb = 0;

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
		output.osName = String(osInfo.sysname) + String(osInfo.version);

		if (BS_ARCH_TYPE == BS_ARCHITECTURE_x86_64)
			output.osIs64Bit = true;
		else
			output.osIs64Bit = strstr(osInfo.machine, "64") != nullptr;

		// Get GPU info
		output.gpuInfo = sGPUInfo;

		return output;
	}

	String PlatformUtility::convertCaseUTF8(const String& input, bool toUpper)
	{
		UErrorCode errorCode = U_ZERO_ERROR;

		auto inputLen = (int32_t)input.size();
		int32_t bufferLen = 0;
		u_strFromUTF8(nullptr, 0, &bufferLen, input.data(), inputLen, &errorCode);

		auto uStr = bs_stack_alloc<UChar>((UINT32)bufferLen);
		int32_t uStrLen = 0;
		errorCode = U_ZERO_ERROR;
		u_strFromUTF8(uStr, bufferLen * sizeof(UChar), &uStrLen, input.data(), inputLen, &errorCode);

		errorCode = U_ZERO_ERROR;
		if(toUpper)
			bufferLen = u_strToUpper(nullptr, 0, uStr, uStrLen, nullptr, &errorCode);
		else
			bufferLen = u_strToLower(nullptr, 0, uStr, uStrLen, nullptr, &errorCode);

		auto convertedUStr = bs_stack_alloc<UChar>((UINT32) bufferLen);
		int32_t convertedUStrLen = 0;

		errorCode = U_ZERO_ERROR;
		if(toUpper)
			convertedUStrLen = u_strToUpper(convertedUStr, bufferLen * sizeof(UChar), uStr, uStrLen, nullptr, &errorCode);
		else
			convertedUStrLen = u_strToLower(convertedUStr, bufferLen * sizeof(UChar), uStr, uStrLen, nullptr, &errorCode);

		errorCode = U_ZERO_ERROR;
		u_strToUTF8(nullptr, 0, &bufferLen, convertedUStr, convertedUStrLen, &errorCode);

		int32_t outputStrLen = 0;
		auto outputStr = bs_stack_alloc<char>(bufferLen);

		errorCode = U_ZERO_ERROR;
		u_strToUTF8(outputStr, bufferLen, &outputStrLen, convertedUStr, convertedUStrLen, &errorCode);

		String output(outputStr, outputStrLen);

		bs_stack_free(outputStr);
		bs_stack_free(convertedUStr);
		bs_stack_free(uStr);

		return output;
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
}
