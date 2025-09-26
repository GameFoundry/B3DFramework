//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Profiling
	 *  @{
	 */

	class CPUProfilerReport;

	/**
	 * Provides various performance measuring methods.
	 *
	 * @note	Thread safe. Matching begin* \ end* calls must belong to the same thread though.
	 */
	class B3D_EXPORT ProfilerCPU : public Module<ProfilerCPU>
	{
		/**	Timer class responsible for tracking elapsed time. */
		class Timer
		{
		public:
			Timer();

			/** Sets the start time for the timer. */
			void Start();

			/** Stops the timer and calculates the elapsed time from start time to now. */
			void Stop();

			/**	Resets the elapsed time to zero. */
			void Reset();

			double Time;

		private:
			double startTime = 0.0f;
			std::chrono::high_resolution_clock mHRClock;

			/**	Returns time elapsed since CPU was started in millseconds. */
			inline double GetCurrentTime() const;
		};

		/**	Timer class responsible for tracking number of elapsed CPU cycles. */
		class TimerPrecise
		{
		public:
			TimerPrecise();

			/** Starts the counter marking the current number of executed CPU cycles since CPU was started. */
			void Start();

			/** Ends the counter and calculates the number of CPU cycles between now and the start time. */
			void Stop();

			/**	Resets the cycle count to zero. */
			void Reset();

			u64 Cycles;

		private:
			u64 startCycles;

			/** Queries the CPU for the current number of CPU cycles executed since the program was started. */
			static inline u64 GetNumCycles();
		};

		/**
		 * Contains data about a single profiler sample (counting time in milliseconds).
		 *
		 * @note
		 * A sample is created whenever a named profile block is entered. For example if you have a function you are
		 * profiling, and it gets called 10 times, there will be 10 samples.
		 */
		struct ProfileSample
		{
			ProfileSample(double _time, u64 _numAllocs, u64 _numFrees)
				: Time(_time), NumAllocs(_numAllocs), NumFrees(_numFrees)
			{}

			double Time;
			u64 NumAllocs;
			u64 NumFrees;
		};

		/**
		 * Contains data about a single precise profiler sample (counting CPU cycles).
		 *
		 * @note
		 * A sample is created whenever a named profile block is entered. For example if you have a function you are
		 * profiling, and it gets called 10 times, there will be 10 samples.
		 */
		struct PreciseProfileSample
		{
			PreciseProfileSample(u64 _cycles, u64 _numAllocs, u64 _numFrees)
				: Cycles(_cycles), NumAllocs(_numAllocs), NumFrees(_numFrees)
			{}

			u64 Cycles;
			u64 NumAllocs;
			u64 NumFrees;
		};

		/**	Contains basic (time based) profiling data contained in a profiling block. */
		struct ProfileData
		{
			ProfileData(FrameAllocator* alloc);

			/** Begins a new sample and records current sample state. Previous sample must not be active. */
			void BeginSample();

			/**
			 * Records current sample state and creates a new sample based on start and end state. Adds the sample to the
			 * sample list.
			 */
			void EndSample();

			/**
			 * Removes the last added sample from the sample list and makes it active again. You must call endSample()
			 * when done as if you called beginSample().
			 */
			void ResumeLastSample();

			Vector<ProfileSample, StdFrameAlloc<ProfileSample>> Samples;
			Timer Timer;

			u64 MemAllocs;
			u64 MemFrees;
		};

		/**	Contains precise (CPU cycle based) profiling data contained in a profiling block. */
		struct PreciseProfileData
		{
			PreciseProfileData(FrameAllocator* alloc);

			/** Begins a new sample and records current sample state. Previous sample must not be active. */
			void BeginSample();

			/**
			 * Records current sample state and creates a new sample based on start and end state. Adds the sample to the
			 * sample list.
			 */
			void EndSample();

			/**
			 * Removes the last added sample from the sample list and makes it active again. You must call endSample()
			 * when done as if you called beginSample.
			 */
			void ResumeLastSample();

			Vector<PreciseProfileSample, StdFrameAlloc<PreciseProfileSample>> Samples;
			TimerPrecise Timer;

			u64 MemAllocs;
			u64 MemFrees;
		};

		/**
		 * Contains all sampling information about a single named profiling block. Each block has its own sampling
		 * information and optionally child blocks.
		 */
		struct ProfiledBlock
		{
			ProfiledBlock(FrameAllocator* alloc);
			~ProfiledBlock();

			/**	Attempts to find a child block with the specified name. Returns null if not found. */
			ProfiledBlock* FindChild(const char* name) const;

			char* Name;

			ProfileData Basic;
			PreciseProfileData Precise;

			Vector<ProfiledBlock*, StdFrameAlloc<ProfiledBlock*>> Children;
		};

		/**	CPU sampling type. */
		enum class ActiveSamplingType
		{
			Basic, /**< Sample using milliseconds. */
			Precise /**< Sample using CPU cycles. */
		};

		/**	Contains data about the currently active profiling block. */
		struct ActiveBlock
		{
			ActiveBlock()
				: Type(ActiveSamplingType::Basic), Block(nullptr)
			{}

			ActiveBlock(ActiveSamplingType _type, ProfiledBlock* _block)
				: Type(_type), Block(_block)
			{}

			ActiveSamplingType Type;
			ProfiledBlock* Block;
		};

		/** Contains data about an active profiling thread. */
		struct ThreadInfo
		{
			ThreadInfo();

			/**
			 * Starts profiling on the thread. New primary profiling block is created with the given name.
			 */
			void Begin(const char* _name);

			/**
			 * Ends profiling on the thread. You should end all samples before calling this, but if you don't they will be
			 * terminated automatically.
			 */
			void End();

			/**
			 * 	Deletes all internal profiling data and makes the object ready for another iteration. Should be called
			 * after end in order to delete any existing data.
			 */
			void Reset();

			/**	Gets the primary profiling block used by the thread. */
			ProfiledBlock* GetBlock(const char* name);

			/** Deletes the provided block. */
			void ReleaseBlock(ProfiledBlock* block);

			static B3D_THREADLOCAL ThreadInfo* activeThread;
			bool IsActive = false;

			ProfiledBlock* RootBlock = nullptr;

			FrameAllocator FrameAllocator;
			ActiveBlock ActiveBlock;
			Stack<ProfilerCPU::ActiveBlock, StdFrameAlloc<ProfilerCPU::ActiveBlock>>* ActiveBlocks = nullptr;
		};

	public:
		ProfilerCPU();
		~ProfilerCPU();

		/**
		 * Registers a new thread we will be doing sampling in. This needs to be called before any beginSample* \ endSample*
		 * calls are made in that thread.
		 *
		 * @param[in]	name	Name that will allow you to more easily identify the thread.
		 */
		void BeginThread(const char* name);

		/**	Ends sampling for the current thread. No beginSample* \ endSample* calls after this point. */
		void EndThread();

		/**
		 * Begins sample measurement. Must be followed by endSample().
		 *
		 * @param[in]	name	Unique name for the sample you can later use to find the sampling data.
		 */
		void BeginSample(const char* name);

		/**
		 * Ends sample measurement.
		 *
		 * @param[in]	name	Unique name for the sample.
		 *
		 * @note
		 * Unique name is primarily needed to more easily identify mismatched begin/end sample pairs. Otherwise the name in
		 * beginSample() would be enough.
		 */
		void EndSample(const char* name);

		/**
		 * Begins precise sample measurement. Must be followed by endSamplePrecise().
		 *
		 * @param[in]	name	Unique name for the sample you can later use to find the sampling data.
		 *
		 * @note
		 * This method uses very precise CPU counters to determine variety of data not provided by standard beginSample().
		 * However due to the way these counters work you should not use this method for larger parts of code. It does not
		 * consider context switches so if the OS decides to switch context between measurements you will get invalid data.
		 */
		void BeginSamplePrecise(const char* name);

		/**
		 * Ends precise sample measurement.
		 *
		 * @param[in]	name	Unique name for the sample.
		 *
		 * @note
		 * Unique name is primarily needed to more easily identify mismatched begin/end sample pairs. Otherwise the name
		 * in beginSamplePrecise() would be enough.
		 */
		void EndSamplePrecise(const char* name);

		/** Clears all sampling data, and ends any unfinished sampling blocks. */
		void Reset();

		/**
		 * Generates a report from all previously sampled data.
		 *
		 * @note	Generating a report will stop all in-progress sampling. You should make sure
		 * 			you call endSample* manually beforehand so this doesn't have to happen.
		 */
		CPUProfilerReport GenerateReport();

	private:
		/**
		 * Calculates overhead that the timing and sampling methods themselves introduce so we might get more accurate
		 * measurements when creating reports.
		 */
		void EstimateTimerOverhead();

	private:
		double mBasicTimerOverhead = 0.0;
		u64 mPreciseTimerOverhead = 0;

		double mBasicSamplingOverheadMs = 0.0;
		double mPreciseSamplingOverheadMs = 0.0;
		u64 mBasicSamplingOverheadCycles = 0;
		u64 mPreciseSamplingOverheadCycles = 0;

		ProfilerVector<ThreadInfo*> mActiveThreads;
		Mutex mThreadSync;
	};

	/** Profiling entry containing information about a single CPU profiling block containing timing information. */
	struct B3D_EXPORT CPUProfilerBasicSamplingEntry
	{
		struct B3D_EXPORT Data
		{
			Data() = default;

			String Name; /**< Name of the profiling block. */
			u32 NumCalls = 0; /**< Number of times the block was entered. */

			u64 MemAllocs; /**< Number of memory allocations that happened within the block. */
			u64 MemFrees; /**< Number of memory deallocations that happened within the block. */

			double AvgTimeMs = 0.0; /**< Average time it took to execute the block, per call. In milliseconds. */
			double MaxTimeMs = 0.0; /**< Maximum time of a single call in the block. In milliseconds. */
			double TotalTimeMs = 0.0; /**< Total time the block took, across all calls. In milliseconds. */

			double AvgSelfTimeMs = 0.0; /**< Average time it took to execute the block, per call. Ignores time used by child blocks. In milliseconds. */
			double TotalSelfTimeMs = 0.0; /**< Total time the block took, across all calls. Ignores time used by child blocks. In milliseconds. */

			double EstimatedSelfOverheadMs = 0.0; /**< Estimated overhead of profiling methods, only for this exact block. In milliseconds. */
			double EstimatedOverheadMs = 0.0; /**< Estimated overhead of profiling methods for this block and all children. In milliseconds. */

			float PctOfParent = 1.0f; /**< Percent of parent block time this block took to execute. Ranging [0.0, 1.0]. */
		} Data;

		ProfilerVector<CPUProfilerBasicSamplingEntry> ChildEntries;
	};

	/**
	 * Profiling entry containing information about a single CPU profiling block containing CPU cycle count based
	 * information.
	 */
	struct B3D_EXPORT CPUProfilerPreciseSamplingEntry
	{
		struct B3D_EXPORT Data
		{
			Data() = default;

			String Name; /**< Name of the profiling block. */
			u32 NumCalls = 0; /**< Number of times the block was entered. */

			u64 MemAllocs; /**< Number of memory allocations that happened within the block. */
			u64 MemFrees; /**< Number of memory deallocations that happened within the block. */

			u64 AvgCycles = 0; /**< Average number of cycles it took to execute the block, per call. */
			u64 MaxCycles = 0; /**< Maximum number of cycles of a single call in the block. */
			u64 TotalCycles = 0; /**< Total number of cycles across all calls in the block. */

			u64 AvgSelfCycles = 0; /**< Average number of cycles it took to execute the block, per call. Ignores cycles used by child blocks. */
			u64 TotalSelfCycles = 0; /**< Total number of cycles across all calls in the block. Ignores time used by child blocks. */

			u64 EstimatedSelfOverhead = 0; /**< Estimated overhead of profiling methods, only for this exact block. In cycles. */
			u64 EstimatedOverhead = 0; /**< Estimated overhead of profiling methods for this block and all children. In cycles. */

			float PctOfParent = 1.0f; /**< Percent of parent block cycles used by this block. Ranging [0.0, 1.0]. */
		} Data;

		ProfilerVector<CPUProfilerPreciseSamplingEntry> ChildEntries;
	};

	/** CPU profiling report containing all profiling information for a single profiling session. */
	class B3D_EXPORT CPUProfilerReport
	{
	public:
		CPUProfilerReport() = default;

		/**
		 * Returns root entry for the basic (time based) sampling data. Root entry always contains the profiling block
		 * associated with the entire thread.
		 */
		const CPUProfilerBasicSamplingEntry& GetBasicSamplingData() const { return mBasicSamplingRootEntry; }

		/**
		 * Returns root entry for the precise (CPU cycle based) sampling data. Root entry always contains the profiling
		 * block associated with the entire thread.
		 */
		const CPUProfilerPreciseSamplingEntry& GetPreciseSamplingData() const { return mPreciseSamplingRootEntry; }

	private:
		friend class ProfilerCPU;

		CPUProfilerBasicSamplingEntry mBasicSamplingRootEntry;
		CPUProfilerPreciseSamplingEntry mPreciseSamplingRootEntry;
	};

	/** Provides global access to ProfilerCPU instance. */
	B3D_EXPORT ProfilerCPU& GetProfilerCPU();

	/** Shortcut for profiling a single function call. */
#define PROFILE_CALL(call, name)              \
	{                                         \
		b3d::GetProfilerCPU().BeginSample(name); \
		call;                                 \
		b3d::GetProfilerCPU().EndSample(name);   \
	}

	/** @} */
} // namespace b3d
