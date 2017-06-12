//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPlatformDefines.h"
#include "BsString.h"
#include "BsTypes.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	struct MACAddress;

	/** Possible type of platform file dialogs. */
	enum class FileDialogType
	{
		OpenFile = 0x0, 
		OpenFolder = 0x1, 
		Save = 0x2,
		Multiselect = 0x10000,
		TypeMask = 0xFFFF
	};

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

		/**
		 * Adds a string to the clipboard.
		 *
		 * @note	Thread safe.
		 */
		static void copyToClipboard(const WString& string);

		/**
		 * Reads a string from the clipboard and returns it. If there is no string in the clipboard it returns an empty 
		 * string.
		 *
		 * @note	
		 * Both wide and normal strings will be read, but normal strings will be converted to a wide string before returning.
		 * @note
		 * Thread safe.
		 */
		static WString copyFromClipboard();

		/**
		 * Converts a keyboard key-code to a Unicode character.
		 *
		 * @note	
		 * Normally this will output a single character, but it can happen it outputs multiple in case a accent/diacritic 
		 * character could not be combined with the virtual key into a single character.
		 */
		static WString keyCodeToUnicode(UINT32 keyCode);

		/**
		 * Populates the provided buffer with a MAC address of the first available adapter, if one exists. If no adapters 
		 * exist, returns false.
		 */
		static bool getMACAddress(MACAddress& address);

		/** Returns information about the underlying hardware. */
		static SystemInfo getSystemInfo();

		/** Creates a new universally unique identifier and returns it as a string. */
		static String generateUUID();

		/**
		 * Opens the provided file or folder using the default application for that file type, as specified by the operating 
		 * system.
		 *
		 * @param[in]	path	Absolute path to the file or folder to open.
		 */
		static void open(const Path& path);

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

	/** Represents a MAC (ethernet) address. */
	struct MACAddress
	{
		UINT8 value[6];
	};

	/** @} */
}
