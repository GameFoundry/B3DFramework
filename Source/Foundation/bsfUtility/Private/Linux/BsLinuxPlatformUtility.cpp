//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsPlatformUtility.h"
#include <stdlib.h>
#include <uuid/uuid.h>
#include <sys/utsname.h>
#include <unicode/unistr.h>

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
		UnicodeString uniStr = UnicodeString::fromUTF8(StringPiece(&input[0], input.size()));

		if(toUpper)
			uniStr.toUpper();
		else
			uniStr.toLower();

		String output;
		uniStr.toUTF8String(output);

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
