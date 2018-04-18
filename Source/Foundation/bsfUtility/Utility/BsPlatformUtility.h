//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPlatformDefines.h"
#include "String/BsString.h"
#include "Prerequisites/BsTypes.h"
#include "Utility/BsUUID.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	struct MACAddress;

	/** Contains information about available GPUs on the system. */
	struct GPUInfo
	{
		String names[5];
		UINT32 numGPUs;
	};

	/** Contains information about the system hardware and operating system. */
	struct SystemInfo
	{
		String cpuManufacturer;
		String cpuModel;
		UINT32 cpuClockSpeedMhz;
		UINT32 cpuNumCores;
		UINT32 memoryAmountMb;
		String osName;
		bool osIs64Bit;

		GPUInfo gpuInfo;
	};

	/** Provides access to various operating system specific utility functions. */
	class BS_UTILITY_EXPORT PlatformUtility
	{
	public:
		/**
		 * Terminates the current process.
		 * 			
		 * @param[in]	force	True if the process should be forcefully terminated with no cleanup.
		 */
		static void terminate(bool force = false);

		/** Returns information about the underlying hardware. */
		static SystemInfo getSystemInfo();

		/** Creates a new universally unique identifier (UUID/GUID). */
		static UUID generateUUID();

		/** 
		 * Converts a UTF8 encoded string into uppercase or lowercase. 
		 * 
		 * @param[in]	input	String to convert.
		 * @param[in]	toUpper	If true, converts the character to uppercase. Otherwise convert to lowercase.
		 * @return				Converted string.
		 */
		static String convertCaseUTF8(const String& input, bool toUpper);

		/** @name Internal
		 *  @{
		 */

		/** 
		 * Assigns information about GPU hardware. This data will be returned by getSystemInfo() when requested. This is
		 * expeced to be called by the render API backend when initialized.
		 */
		static void _setGPUInfo(GPUInfo gpuInfo) { sGPUInfo = gpuInfo; }

		/** @} */

	private:
		static GPUInfo sGPUInfo;
	};

	/** @} */
}
