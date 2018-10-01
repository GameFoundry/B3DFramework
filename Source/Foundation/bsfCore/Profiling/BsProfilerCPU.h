//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
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
	class BS_CORE_EXPORT ProfilerCPU : public Module<ProfilerCPU>
	{
		/**	Timer class responsible for tracking elapsed time. */
		class Timer
		{
		public:
			Timer();

			/** Sets the start time for the timer. */
			void start();

			/** Stops the timer and calculates the elapsed time from start time to now. */
			void stop();

			/**	Resets the elapsed time to zero. */
			void reset();

			double time;
		private:
			double startTime;
			std::chrono::high_resolution_clock mHRClock;

			/**	Returns time elapsed since CPU was started in millseconds. */
			inline double getCurrentTime() const;
		};

		/**	Timer class responsible for tracking number of elapsed CPU cycles. */
		class TimerPrecise
		{
		public:
			TimerPrecise();

			/** Starts the counter marking the current number of executed CPU cycles since CPU was started. */
			void start();

			/** Ends the counter and calculates the number of CPU cycles between now and the start time. */
			void stop();

			/**	Resets the cycle count to zero. */
			void reset();

			UINT64 cycles;
		private:
			UINT64 startCycles;

			/** Queries the CPU for the current number of CPU cycles executed since the program was started. */
			static inline UINT64 getNumCycles();
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
			ProfileSample(double _time, UINT64 _numAllocs, UINT64 _numFrees)
				:time(_time), numAllocs(_numAllocs), numFrees(_numFrees)
			{ }

			double time;
			UINT64 numAllocs;
			UINT64 numFrees;
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
			PreciseProfileSample(UINT64 _cycles, UINT64 _numAllocs, UINT64 _numFrees)
				:cycles(_cycles), numAllocs(_numAllocs), numFrees(_numFrees)
			{ }

			UINT64 cycles;
			UINT64 numAllocs;
			UINT64 numFrees;
		};

		/**	Contains basic (time based) profiling data contained in a profiling block. */
		struct ProfileData
		{
			ProfileData(FrameAlloc* alloc);

			/** Begins a new sample and records current sample state. Previous sample must not be active. */
			void beginSample();

			/**
			 * Records current sample state and creates a new sample based on start and end state. Adds the sample to the 
			 * sample list.
			 */
			void endSample();

			/**
			 * Removes the last added sample from the sample list and makes it active again. You must call endSample() 
			 * when done as if you called beginSample().
			 */
			void resumeLastSample();

			Vector<ProfileSample, StdFrameAlloc<ProfileSample>> samples;
			Timer timer;

			UINT64 memAllocs;
			UINT64 memFrees;
		};

		/**	Contains precise (CPU cycle based) profiling data contained in a profiling block. */
		struct PreciseProfileData
		{
			PreciseProfileData(FrameAlloc* alloc);

			/** Begins a new sample and records current sample state. Previous sample must not be active. */
			void beginSample();

			/**
			 * Records current sample state and creates a new sample based on start and end state. Adds the sample to the 
			 * sample list.
			 */
			void endSample();

			/**
			 * Removes the last added sample from the sample list and makes it active again. You must call endSample() 
			 * when done as if you called beginSample.
			 */
			void resumeLastSample();

			Vector<PreciseProfileSample, StdFrameAlloc<PreciseProfileSample>> samples;
			TimerPrecise timer;

			UINT64 memAllocs;
			UINT64 memFrees;
		};

		/**
		 * Contains all sampling information about a single named profiling block. Each block has its own sampling 
		 * information and optionally child blocks.
		 */
		struct ProfiledBlock
		{
			ProfiledBlock(FrameAlloc* alloc);
			~ProfiledBlock();

			/**	Attempts to find a child block with the specified name. Returns null if not found. */
			ProfiledBlock* findChild(const char* name) const;

			char* name;
			
			ProfileData basic;
			PreciseProfileData precise;

			Vector<ProfiledBlock*, StdFrameAlloc<ProfiledBlock*>> children;
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
				:type(ActiveSamplingType::Basic), block(nullptr)
			{ }

			ActiveBlock(ActiveSamplingType _type, ProfiledBlock* _block)
				:type(_type), block(_block)
			{ }

			ActiveSamplingType type;
			ProfiledBlock* block;
		};

		/** Contains data about an active profiling thread. */
		struct ThreadInfo
		{
			ThreadInfo();

			/**
			 * Starts profiling on the thread. New primary profiling block is created with the given name.
			 */
			void begin(const char* _name);

			/**
			 * Ends profiling on the thread. You should end all samples before calling this, but if you don't they will be 
			 * terminated automatically.
			 */
			void end();

			/**
			 * 	Deletes all internal profiling data and makes the object ready for another iteration. Should be called 
			 * after end in order to delete any existing data.
			 */
			void reset();

			/**	Gets the primary profiling block used by the thread. */
			ProfiledBlock* getBlock(const char* name);
			
			/** Deletes the provided block. */
			void releaseBlock(ProfiledBlock* block);

			static BS_THREADLOCAL ThreadInfo* activeThread;
			bool isActive;

			ProfiledBlock* rootBlock;

			FrameAlloc frameAlloc;
			ActiveBlock activeBlock;
			Stack<ActiveBlock, StdFrameAlloc<ActiveBlock>>* activeBlocks;
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
		void beginThread(const char* name);

		/**	Ends sampling for the current thread. No beginSample* \ endSample* calls after this point. */
		void endThread();

		/**
		 * Begins sample measurement. Must be followed by endSample(). 
		 *
		 * @param[in]	name	Unique name for the sample you can later use to find the sampling data.
		 */
		void beginSample(const char* name);

		/**
		 * Ends sample measurement.
		 *
		 * @param[in]	name	Unique name for the sample. 
		 * 					
		 * @note	
		 * Unique name is primarily needed to more easily identify mismatched begin/end sample pairs. Otherwise the name in 
		 * beginSample() would be enough.
		 */
		void endSample(const char* name);

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
		void beginSamplePrecise(const char* name);

		/**
		 * Ends precise sample measurement.
		 *
		 * @param[in]	name	Unique name for the sample. 
		 * 					
		 * @note	
		 * Unique name is primarily needed to more easily identify mismatched begin/end sample pairs. Otherwise the name 
		 * in beginSamplePrecise() would be enough.
		 */
		void endSamplePrecise(const char* name);

		/** Clears all sampling data, and ends any unfinished sampling blocks. */
		void reset();

		/**
		 * Generates a report from all previously sampled data.
		 * 			
		 * @note	Generating a report will stop all in-progress sampling. You should make sure
		 * 			you call endSample* manually beforehand so this doesn't have to happen.
		 */
		CPUProfilerReport generateReport();

	private:
		/**
		 * Calculates overhead that the timing and sampling methods themselves introduce so we might get more accurate 
		 * measurements when creating reports.
		 */
		void estimateTimerOverhead();

	private:
		double mBasicTimerOverhead;
		UINT64 mPreciseTimerOverhead;

		double mBasicSamplingOverheadMs;
		double mPreciseSamplingOverheadMs;
		UINT64 mBasicSamplingOverheadCycles;
		UINT64 mPreciseSamplingOverheadCycles;

		ProfilerVector<ThreadInfo*> mActiveThreads;
		Mutex mThreadSync;
	};

	/** Profiling entry containing information about a single CPU profiling block containing timing information. */
	struct BS_CORE_EXPORT CPUProfilerBasicSamplingEntry
	{
		struct BS_CORE_EXPORT Data
		{
			Data();

			String name; /**< Name of the profiling block. */
			UINT32 numCalls; /**< Number of times the block was entered. */

			UINT64 memAllocs; /**< Number of memory allocations that happened within the block. */
			UINT64 memFrees; /**< Number of memory deallocations that happened within the block. */

			double avgTimeMs; /**< Average time it took to execute the block, per call. In milliseconds. */
			double maxTimeMs; /**< Maximum time of a single call in the block. In milliseconds. */
			double totalTimeMs; /**< Total time the block took, across all calls. In milliseconds. */

			double avgSelfTimeMs; /**< Average time it took to execute the block, per call. Ignores time used by child blocks. In milliseconds. */
			double totalSelfTimeMs; /**< Total time the block took, across all calls. Ignores time used by child blocks. In milliseconds. */

			double estimatedSelfOverheadMs; /**< Estimated overhead of profiling methods, only for this exact block. In milliseconds. */
			double estimatedOverheadMs; /**< Estimated overhead of profiling methods for this block and all children. In milliseconds. */

			float pctOfParent; /**< Percent of parent block time this block took to execute. Ranging [0.0, 1.0]. */
		} data;

		ProfilerVector<CPUProfilerBasicSamplingEntry> childEntries;
	};

	/**
	 * Profiling entry containing information about a single CPU profiling block containing CPU cycle count based 
	 * information.
	 */
	struct BS_CORE_EXPORT CPUProfilerPreciseSamplingEntry
	{
		struct BS_CORE_EXPORT Data
		{
			Data();

			String name; /**< Name of the profiling block. */
			UINT32 numCalls; /**< Number of times the block was entered. */

			UINT64 memAllocs; /**< Number of memory allocations that happened within the block. */
			UINT64 memFrees; /**< Number of memory deallocations that happened within the block. */

			UINT64 avgCycles; /**< Average number of cycles it took to execute the block, per call. */
			UINT64 maxCycles; /**< Maximum number of cycles of a single call in the block. */
			UINT64 totalCycles; /**< Total number of cycles across all calls in the block. */

			UINT64 avgSelfCycles; /**< Average number of cycles it took to execute the block, per call. Ignores cycles used by child blocks. */
			UINT64 totalSelfCycles; /**< Total number of cycles across all calls in the block. Ignores time used by child blocks. */

			UINT64 estimatedSelfOverhead; /**< Estimated overhead of profiling methods, only for this exact block. In cycles. */
			UINT64 estimatedOverhead; /**< Estimated overhead of profiling methods for this block and all children. In cycles. */

			float pctOfParent; /**< Percent of parent block cycles used by this block. Ranging [0.0, 1.0]. */
		} data;

		ProfilerVector<CPUProfilerPreciseSamplingEntry> childEntries;
	};

	/** CPU profiling report containing all profiling information for a single profiling session. */
	class BS_CORE_EXPORT CPUProfilerReport
	{
	public:
		CPUProfilerReport();

		/**
		 * Returns root entry for the basic (time based) sampling data. Root entry always contains the profiling block 
		 * associated with the entire thread.
		 */
		const CPUProfilerBasicSamplingEntry& getBasicSamplingData() const { return mBasicSamplingRootEntry; }

		/**
		 * Returns root entry for the precise (CPU cycle based) sampling data. Root entry always contains the profiling 
		 * block associated with the entire thread.
		 */
		const CPUProfilerPreciseSamplingEntry& getPreciseSamplingData() const { return mPreciseSamplingRootEntry; }

	private:
		friend class ProfilerCPU;

		CPUProfilerBasicSamplingEntry mBasicSamplingRootEntry;
		CPUProfilerPreciseSamplingEntry mPreciseSamplingRootEntry;
	};

	/** Provides global access to ProfilerCPU instance. */
	BS_CORE_EXPORT ProfilerCPU& gProfilerCPU();

	/** Shortcut for profiling a single function call. */
#define PROFILE_CALL(call, name)					\
	{												\
		bs::gProfilerCPU().beginSample(name);		\
		call;										\
		bs::gProfilerCPU().endSample(name);			\
	}

	/** @} */
}
