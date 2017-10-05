//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Utility/BsPlatformUtility.h"
#include <stdlib.h>
#include <uuid/uuid.h>
#include <sys/utsname.h>

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
			std::ifstream file("/proc/meminfo");
			std::string token;
			while(file >> token)
			{
				if(token == "vendor_id")
				{
					if(file >> token && token == ":")
					{
						std::string vendorId;
						if(file >> vendorId)
							output.cpuManufacturer = vendorId.c_str();
					}
				}
				else if(token == "model")
				{
					if(file >> token && token == "name")
					{
						if (file >> token && token == ":")
						{
							std::stringstream modelName;
							if (file >> token)
							{
								modelName << token;

								while (file >> token)
									modelName << " " << token;
							}

							output.cpuManufacturer = modelName.str().c_str();
						}
					}
				}
				else if(token == "cpu")
				{
					if(file >> token)
					{
						if (token == "cores")
						{
							if (file >> token && token == ":")
							{
								UINT32 numCores;
								if (file >> numCores)
									output.cpuNumCores = numCores;
							}
						}
					}
				}

				// Ignore the rest of the line
				file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}
		}

		// Get CPU frequency
		{
			std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
			UINT32 frequency;
			if(file >> frequency)
				output.cpuClockSpeedMhz = frequency / (1000 * 1000);
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
						output.memoryAmountMb = memTotal / (1024 * 1024);
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

	String PlatformUtility::generateUUID()
	{
		uuid_t nativeUUID;
		uuid_generate(nativeUUID);

		char uuidChars[37];
		uuid_unparse(nativeUUID, uuidChars);

		return String(uuidChars);
	}
}
