//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/B3DPlatformDefines.h"
#include "String/B3DString.h"
#include "Prerequisites/B3DTypes.h"
#include "Utility/B3DUUID.h"

namespace b3d
{
	/** @addtogroup Platform
	 *  @{
	 */

	struct MACAddress;

	/** Contains information about available GPUs on the system. */
	struct GPUInfo
	{
		String Names[5];
		u32 NumGpUs;
	};

	/** Contains information about the system hardware and operating system. */
	struct SystemInfo
	{
		String CpuManufacturer;
		String CpuModel;
		u32 CpuClockSpeedMhz;
		u32 CpuNumCores;
		u32 MemoryAmountMb;
		String OsName;
		bool OsIs64Bit;

		GPUInfo GpuInfo;
	};

	/** Provides access to various operating system specific utility functions. */
	class B3D_EXPORT PlatformUtility
	{
	public:
		/**
		 * Terminates the current process.
		 *
		 * @param	force	True if the process should be forcefully terminated with no cleanup.
		 */
		[[noreturn]] static void Terminate(bool force = false);

		/**
		 * Disables interactive error dialogs (assert message boxes, abort prompts, OS error popups) for the current
		 * process, routing the reports to stderr instead. Intended for unattended runs (CI, headless tests) where a
		 * modal dialog would block the process until manually dismissed. No-op on platforms without such dialogs.
		 */
		static void DisableInteractiveErrorDialogs();

		/** Returns information about the underlying hardware. */
		static SystemInfo GetSystemInfo();

		/** Creates a new universally unique identifier (UUID/GUID). */
		static UUID GenerateUuid();

		/**
		 * Converts a UTF8 encoded string into uppercase or lowercase.
		 *
		 * @param	input	String to convert.
		 * @param	toUpper	If true, converts the character to uppercase. Otherwise convert to lowercase.
		 * @return			Converted string.
		 */
		static String ConvertCaseUtF8(const String& input, bool toUpper);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Assigns information about GPU hardware. This data will be returned by getSystemInfo() when requested. This is
		 * expeced to be called by the render API backend when initialized.
		 */
		static void SetGPUInfo(GPUInfo gpuInfo) { sGPUInfo = gpuInfo; }

		/** @} */

	private:
		static GPUInfo sGPUInfo;
	};

	/** @} */
} // namespace b3d
