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
		/** Returns whether a debugger is currently attached to this process. */
		static bool IsDebuggerAttached();

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

		/**
		 * Pushes a new autorelease pool on the calling thread's pool stack and returns its handle. Objects autoreleased by
		 * Objective-C code are released once the pool is popped via PopAutoreleasePool(). Returns null on platforms without
		 * autorelease pools.
		 *
		 * Prefer AutoreleasePoolScope over calling this directly, and see its documentation for restrictions regarding fibers.
		 */
		static void* PushAutoreleasePool();

		/** Pops the autorelease pool returned by PushAutoreleasePool(), along with any pools pushed after it. */
		static void PopAutoreleasePool(void* pool);

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

#if !B3D_PLATFORM_MACOS
	inline void* PlatformUtility::PushAutoreleasePool() { return nullptr; }
	inline void PlatformUtility::PopAutoreleasePool(void*) { }
#endif

	/**
	 * Pushes an autorelease pool when constructed and pops it when destroyed. Does nothing on platforms without autorelease
	 * pools.
	 *
	 * Autorelease pools form a per-thread stack, while fibers interleave their execution on the same thread. Therefore a scope
	 * must never be created within a scheduler task (Scheduler, SingleConsumerQueue) if the task can yield (e.g. Signal::Wait())
	 * while the scope is alive. Tasks receive their pools from the scheduler instead. Scopes created outside of tasks, on the
	 * thread's own stack, are free to stay alive across yields.
	 */
	class AutoreleasePoolScope
	{
	public:
		AutoreleasePoolScope() : mPool(PlatformUtility::PushAutoreleasePool()) { }
		~AutoreleasePoolScope() { PlatformUtility::PopAutoreleasePool(mPool); }

		AutoreleasePoolScope(const AutoreleasePoolScope&) = delete;
		AutoreleasePoolScope& operator=(const AutoreleasePoolScope&) = delete;

	private:
		void* mPool;
	};

	/** @} */
} // namespace b3d
