//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Threading
	 *  @{
	 */

	/** Provides information about, and unique identifies a single CPU core. */
	struct CPUCore
	{
		struct WindowsInformation
		{
			u8 CoreGroup;
			u8 IndexWithinGroup;
		};

		struct PthreadInformation
		{
			u16 Index;
		};

		union
		{
			WindowsInformation Windows;
			PthreadInformation Pthread;
		};

		CPUCore()
		{
			Pthread.Index = 0;
		}

		bool operator==(CPUCore other) const { return Pthread.Index == other.Pthread.Index; }
		bool operator<(CPUCore other) const { return Pthread.Index < other.Pthread.Index; }
	};

	/** Contains one or multiple logical CPU cores that a thread can run on. */
	class B3D_EXPORT ThreadCoreMask
	{
	public:
		/** True if thread affinity is supported on the current platform. */
#if B3D_PLATFORM_WIN32 || B3D_PLATFORM_MACOS || B3D_PLATFORM_LINUX
		static constexpr bool kIsSupported = true;
#else
		static constexpr bool kIsSupported = false;
#endif

		ThreadCoreMask() = default;
		ThreadCoreMask(const ThreadCoreMask& other) = default;
		ThreadCoreMask(ThreadCoreMask&& other): mCores(std::move(other.mCores)) { }
		ThreadCoreMask(std::initializer_list<CPUCore> initializerList);
		ThreadCoreMask(const TInlineArray<CPUCore, 32>& cores): mCores(cores) {}

		ThreadCoreMask& operator=(ThreadCoreMask&& other)
		{
			if (this == &other)
				return *this;

			mCores = std::move(other.mCores);
			return *this;
		}

		/** Returns the number of enabled cores. */
		size_t GetCoreCount() const { return mCores.Size(); }

		/** Retrieves a core from the affinity. */
		CPUCore operator[](size_t index) const { return mCores[index]; }

		/** Adds cores from the provided mask to the current mask, and returns current mask. */
		ThreadCoreMask& Add(const ThreadCoreMask&);

		/** Removes cores from the provided mask from the current mask, and returns current mask. */
		ThreadCoreMask& Remove(const ThreadCoreMask&);

		/** Creates a mask that allows the thread to run on any available CPU core. */
		static ThreadCoreMask CreateAnyThreadMask();

	private:
		TInlineArray<CPUCore, 32> mCores;
	};

	/** Assigns thread affinities to threads according to a particular policy. */
	class B3D_EXPORT ThreadAffinityPolicy
	{
	public:
		virtual ~ThreadAffinityPolicy() = default;

		/**
		 * Returns a mask that lets the caller know on which cores can the thread be executed on.
		 *
		 * @param	threadIndex		Index of the thread for which to determine affinity. It's best these are sequential so cores
		 *							can be distributed in a round-robin fashion, for policies where that matters.
		 * @return					Returns mask that determines on which cores should the thread run on.
		 */
		virtual ThreadCoreMask GetMaskForThread(u32 threadIndex) const = 0;
	};

	/**
	 * Policy that returns affinity that allows a thread to execute on any of the cores
	 * provided to the policy. On Windows returned affinity will return only cores from the
	 * same group, and the groups are give out depending on the provided thread index.
	 */
	class B3D_EXPORT AnyOfThreadAffinityPolicy : public ThreadAffinityPolicy
	{
	public:
		/** Creates the policy with a list of cores that threads will be allowed to run on. */
		AnyOfThreadAffinityPolicy(const ThreadCoreMask& availableCores): mAvailableCores(availableCores) {}

		ThreadCoreMask GetMaskForThread(u32 threadIndex) const override;

	private:
		ThreadCoreMask mAvailableCores;
	};

	/** Policy that returns an affinity that pins a thread to a single core. The cores are given out depending on the provided thread index, from the policy's affinity mask. */
	class B3D_EXPORT OneOfThreadAffinityPolicy : public ThreadAffinityPolicy
	{
	public:
		/** Creates the policy with a list of cores that will be given out to the requesting threads in round-robin fashion. */
		OneOfThreadAffinityPolicy(const ThreadCoreMask& availableCores): mAvailableCores(availableCores) {}

		ThreadCoreMask GetMaskForThread(u32 threadIndex) const override;

	private:
		ThreadCoreMask mAvailableCores;
	};

	/** Wrapper for an OS thread. */
	class Thread final : INonCopyable
	{
	public:
		B3D_EXPORT Thread() = default;
		B3D_EXPORT Thread(const ThreadCoreMask& affinity, Function<void()>&& workerFunction);
		B3D_EXPORT Thread(Function<void()>&& workerFunction);
		B3D_EXPORT Thread(Thread&&);
		B3D_EXPORT ~Thread();

		B3D_EXPORT Thread& operator=(Thread&&);

		/** Returns a unique identifier for the thread. */
		B3D_EXPORT u32 GetId() const;

		/** Blocks the calling thread until this thread completes. */
		B3D_EXPORT void WaitUntilComplete();

		/** Assigns a name to the current thread, primarily for debugging purposes. */
		B3D_EXPORT static void SetName(const char* format, ...);

		/** Returns the total available number of logical CPU cores. */
		B3D_EXPORT static u32 GetLogicalCoreCount();

		/** Gets the id of the current thread. */
		B3D_EXPORT static u32 GetCurrentThreadId() { return CurrentId; }

	private:
		class Implementation;
		Implementation* m = nullptr;

		static thread_local u32 CurrentId;
	};

	/** @} */

} // namespace b3d

namespace std
{
	/**	Hash value generator for CPUCore. */
	template <>
	struct hash<b3d::CPUCore>
	{
		size_t operator()(const b3d::CPUCore& value) const
		{
			return value.Pthread.Index;
		}
	};
}
